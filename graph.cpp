#include <iostream>
#include <queue>
#include <stack>
#include <vector>
#include <cassert>

#include "graph.hpp"

using namespace std;

/* Add an edge from one vertex to another. */
int FlowGraph::addEdge(int from, int to, int cap) {
	G[from].push_back(Edge(from, to, cap, G[to].size()));
	if (from == to) G[from].back().index++;
	int index = G[from].size() - 1;
	G[to].push_back(Edge(to, from, 0, index));

	return index;
}

/*
 * Add an edge and at the same time an antiparallel edge
 * with the same capacity.
 */
void FlowGraph::addDoubleEdge(int from, int to, int cap) {
	G[from].push_back(Edge(from, to, cap, G[to].size()));
	G[to].push_back(Edge(to, from, cap, G[from].size() - 1));
}

/*
 * Change the capacity of an edge. Need the from-vertex and
 * the index of the edge in its edge list (returned from addEdge.
 */
void FlowGraph::changeCapacity(int from, int index, int cap) {
	int to = G[from][index].to;
	int diff = G[from][index].flow - cap;

	G[from][index].cap = cap;

	if (diff > 0) {
		excess[from] += diff;
		excess[to] -= diff;
		G[from][index].flow = cap;
		G[to][G[from][index].index].flow = -cap;
#ifndef DINIC
		rule.add(from, height[from], excess[from]);
#endif
	}
}

/* Reset all flow and excess. */
void FlowGraph::resetFlow() {
	for (int i = 0; i < G.size(); ++i) {
		for (int j = 0; j < G[i].size(); ++j) {
			G[i][j].flow = 0;
		}
	}
	fill(excess.begin(), excess.end(), 0);
}

/* Reset all distance labels. */
void FlowGraph::resetHeights() {
	fill(height.begin(), height.end(), 0);
	fill(count.begin(), count.end(), 0);
}

/* Push along an edge. */
void FlowGraph::push(Edge &e) {
	int flow = min(e.cap - e.flow, excess[e.from]);
	excess[e.from] -= flow;
	excess[e.to]   += flow;
	e.flow += flow;
	G[e.to][e.index].flow -= flow;

	rule.add(e.to, height[e.to], excess[e.to]);
}

/* Relabel a vertex. */
void FlowGraph::relabel(int u) {
	count[height[u]]--;
	height[u] = 2*N;

	for (int i = 0; i < G[u].size(); ++i) {
		if (G[u][i].cap > G[u][i].flow) {
			height[u] = min(height[u], height[G[u][i].to] + 1);
		}
	}

	if (height[u] >= N) {
		height[u] = N;
	}
	else {
		count[height[u]]++;
		rule.add(u, height[u], excess[u]);
	}
}

/* Relabel all vertices over the gap h to label N. */
void FlowGraph::gap(int h) {
	int c = 0;
	for (int i = 0; i < G.size(); ++i) {
		if (height[i] < h) continue;
		if (height[i] >= N) continue;

		rule.deactivate(i);

		count[height[i]]--;
		height[i] = N;
	}

	rule.gap(h);
}

/* Discharge a vertex. */
void FlowGraph::discharge(int u) {
	int i;
	for (i = 0; i < G[u].size() && excess[u] > 0; ++i) {
		if (G[u][i].cap > G[u][i].flow
				&& height[u] == height[G[u][i].to] + 1) {
			push(G[u][i]);
		}
	}

	if (excess[u] > 0) {
		/* Check if a gap will appear. */
		if (count[height[u]] == 1)
			gap(height[u]);
		else
			relabel(u);
	}
}

/* Run the push-relabel algorithm to find the min-cut. */
void FlowGraph::minCutPushRelabel(int source, int sink) {
	height[source] = N;

	rule.activate(source);
	rule.activate(sink);

	for (int i = 0; i < G[source].size(); ++i) {
		excess[source] = G[source][i].cap;
		push(G[source][i]);
	}
	excess[source] = 0;

	int c = 0;
	/* Loop over active nodes using selection rule. */
	while (!rule.empty()) {
		c++;
		int u = rule.next();
		discharge(u);
	}

	/* Output the cut based on vertex heights. */
	for (int i = 0; i < cut.size(); ++i) {
		cut[i] = height[i] >= N;
	}
}

/*
 * Depth-first-search used to find a cut at the
 * end of Diniz.
 */
void FlowGraph::DFS(int source, int sink) {
	stack<int> s;
	vector<bool> visited(N);
	s.push(source);

	cut[source] = true;
	while (!s.empty()) {
		int u = s.top();
		s.pop();

		for (int i = 0; i < G[u].size(); ++i) {
			if (cut[G[u][i].to]) continue;

			if (G[u][i].cap > G[u][i].flow) {
				assert(G[u][i].to != sink);
				cut[G[u][i].to] = true;
				s.push(G[u][i].to);
			}
		}
	}
}

/* Diniz min-cut algorithm. */
void FlowGraph::minCutDinic(int source, int sink) {
	maxFlowDinic(source, sink);

	fill(cut.begin(), cut.end(), false);
	DFS(source, sink);
}

/*
 * Recursive blocking flow algorithm, from the current
 * vertex u, from which we can at most send limit flow.
 */
int FlowGraph::blockingFlow(vector<int> &level, int u,
		int source, int sink, int limit) {
	if (limit <= 0) return 0;

	if (u == sink) return limit;

	int throughput = 0;
	for (int i = 0; i < G[u].size(); ++i) {

		int res = G[u][i].cap - G[u][i].flow;

		/* Recurse on vertices one level closer to sink. */
		if (level[G[u][i].to] == level[u] + 1 && res > 0) {
			int aug = blockingFlow(
					level,
					G[u][i].to,
					source,
					sink,
					min(limit - throughput, res)
					);

			throughput += aug;
			G[u][i].flow += aug;
			G[G[u][i].to][G[u][i].index].flow -= aug;
		}
	}

	if (throughput == 0) {
		level[u] = 0;
	}

	return throughput;
}

/* Diniz max-flow algorithm. */
int FlowGraph::maxFlowDinic(int source, int sink) {

	vector<int> level(N);

	while (true) {
		fill(level.begin(), level.end(), 0);

		queue<int> nq;

		/*
		 * Fill the level vector using a basic breadth-first
		 * search.
		 */
		nq.push(source);
		level[source] = 1;
		while (!nq.empty()) {
			int u = nq.front();
			nq.pop();
			for (int i = 0; i < G[u].size(); ++i) {
				if (level[G[u][i].to] > 0) continue;

				if (G[u][i].cap > G[u][i].flow) {
					level[G[u][i].to] = level[u] + 1;
					nq.push(G[u][i].to);
				}
			}
		}

		if (level[sink] == 0) break;

		blockingFlow(level, source, source, sink, 1000000000);
	}

	return outFlow(source);
}

bool FlowGraph::checkExcess(void) {
	for (int i = 0; i < excess.size(); ++i) {
		if (excess[i] < 0) {
			return false;
		}
	}

	return true;
}

bool FlowGraph::checkCapacity(void) {
	for (int i = 0; i < G.size(); ++i) {
		for (int j = 0; j < G[i].size(); ++j) {
			if (G[i][j].flow > G[i][j].cap) return false;
		}
	}

	return true;
}

bool FlowGraph::checkLabels(void) {
	for (int i = 0; i < G.size(); ++i) {
		for (int j = 0; j < G[i].size(); ++j) {
			if (G[i][j].flow < G[i][j].cap) {
				if (height[i] > height[G[i][j].to] + 1) {
					return false;
				}
			}
		}
	}

	return true;
}

bool FlowGraph::checkCount(void) {
	for (int i = 0; i < count.size(); ++i) {
		int c = 0;
		for (int j = 0; j < height.size(); ++j) {
			if (height[j] == i) c++;
		}
		if (c != count[i]) {
			cout << "c = " << c << ", count[" << i << "] = ";
			cout << count[i] << endl;
			return false;
		}
	}

	return true;
}

int FlowGraph::outFlow(int source) {
	int c = 0;
	for (int i = 0; i < G[source].size(); ++i) {
		c += G[source][i].flow;
	}
	return c;
}

int FlowGraph::inFlow(int sink) {
	int c = 0;
	for (int i = 0; i < G[sink].size(); ++i) {
		c += G[sink][i].flow;
	}
	return c;
}

