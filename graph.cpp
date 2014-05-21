#include <iostream>
#include <queue>
#include <stack>
#include <vector>
#include <cassert>

#include "graph.hpp"

using namespace std;

int FlowGraph::addEdge(int from, int to, int cap) {
	G[from].push_back(Edge(from, to, cap, G[to].size()));
	if (from == to) G[from].back().index++;
	int index = G[from].size() - 1;
	G[to].push_back(Edge(to, from, 0, index));

	return index;
}

void FlowGraph::addDoubleEdge(int from, int to, int cap) {
	G[from].push_back(Edge(from, to, cap, G[to].size()));
	G[to].push_back(Edge(to, from, cap, G[from].size() - 1));
}

void FlowGraph::changeCapacity(int from, int index, int cap) {
	int to = G[from][index].to;
	int diff = G[from][index].flow - cap;

	G[from][index].cap = cap;

	if (diff > 0) {
		excess[from] += diff;
		excess[to] -= diff;
		G[from][index].flow = cap;
		G[to][G[from][index].index].flow = -cap;
		rule.add(from, height[from], excess[from]);
	}
}

void FlowGraph::resetFlow() {
	for (int i = 0; i < G.size(); ++i) {
		for (int j = 0; j < G[i].size(); ++j) {
			G[i][j].flow = 0;
		}
	}
	fill(excess.begin(), excess.end(), 0);
}

void FlowGraph::resetHeights() {
	fill(height.begin(), height.end(), 0);
	fill(count.begin(), count.end(), 0);
}

// FIXME: Rewrite this and fix Diniz
void FlowGraph::reset(vector<char> nodes) {
	for (int i = 0; i < nodes.size(); ++i) {
		if (nodes[i]) {
			for (int j = 0; j < G[i].size(); ++j) {
				G[i][j].flow = 0;
				G[G[i][j].to][G[i][j].index].flow = 0;
				excess[G[i][j].to] = 0;
			}
			excess[i] = 0;
			height[i] = 0;
			rule.deactivate(i);
		}
	}
}

void FlowGraph::push(Edge &e) {
	int flow = min(e.cap - e.flow, excess[e.from]);
	excess[e.from] -= flow;
	excess[e.to]   += flow;
	e.flow += flow;
	G[e.to][e.index].flow -= flow;

	rule.add(e.to, height[e.to], excess[e.to]);
}

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

void FlowGraph::gap(int h) {
	int c = 0;
	for (int i = 0; i < G.size(); ++i) {
		if (height[i] < h) continue;
		if (height[i] >= N) continue;

		/* FIXME: Who should be responsible for doing this? */
		rule.deactivate(i);

		count[height[i]]--;
		height[i] = N;
	}

	rule.gap(h);
}

void FlowGraph::discharge(int u) {
	int i;
	for (i = 0; i < G[u].size() && excess[u] > 0; ++i) {
		if (G[u][i].cap > G[u][i].flow
				&& height[u] == height[G[u][i].to] + 1) {
			push(G[u][i]);
		}
	}

	if (excess[u] > 0) {
		if (count[height[u]] == 1)
			gap(height[u]);
		else
			relabel(u);
	}
}

double FlowGraph::globalRelabel(int source, int sink) {
	queue<int> nq;
	queue<int> hq;
	vector<char> visited(N);
	nq.push(sink);
	hq.push(0);

	int c = 0;
	int s = 0;
	visited[sink] = true;
	visited[source] = true;
	while (!nq.empty()) {
		int v = nq.front();
		int h = hq.front();
		nq.pop();
		hq.pop();
		h++;
		for (int i = 0; i < G[v].size(); ++i) {
			int to    = v;
			int from  = G[v][i].to;
			int index = G[v][i].index;

			if (visited[from]) continue;
			if (rule.isActive(from)) continue;
			if (height[from] > h) continue;

			if (G[from][index].cap > G[from][index].flow) {
				count[height[from]]--;
				height[from] = h;
				count[h]++;
				c++;
				s += h;
				visited[from] = true;
				rule.add(from, height[from], excess[from]);
				nq.push(from);
				hq.push(h);
			}
		}
	}
	cout << "Relabeled: " << c << endl;
	cout << "Avg: " << static_cast<double>(s) / c << endl;

	return static_cast<double>(s) / c;
}

void FlowGraph::minCutPushRelabel(int source, int sink) {
	height[source] = N;

	rule.activate(source);
	rule.activate(sink);

	for (int i = 0; i < G[source].size(); ++i) {
		//if (cut[G[source][i].to]) continue;
		excess[source] = G[source][i].cap;
		push(G[source][i]);
	}
	excess[source] = 0;

	int c = 0;
	while (!rule.empty()) {
		c++;
		int u = rule.next();
		discharge(u);
	}
	cout << "Discharged: " << c << endl;

	for (int i = 0; i < cut.size(); ++i) {
		cut[i] = height[i] >= N;
	}
}

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

void FlowGraph::minCutDinic(int source, int sink) {
	maxFlowDinic(source, sink);

	fill(cut.begin(), cut.end(), false);
	DFS(source, sink);
}

int FlowGraph::blockingFlow(vector<int> &level, int u,
		int source, int sink, int limit) {
	if (limit <= 0) return 0;

	if (u == sink) return limit;

	int throughput = 0;
	for (int i = 0; i < G[u].size(); ++i) {
		int res = G[u][i].cap - G[u][i].flow;
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

int FlowGraph::maxFlowDinic(int source, int sink) {
	while (true) {
		vector<int> level(N);

		queue<int> nq;

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

int FlowGraph::activeNodes(void) {
	int c = 0;
	for (int i = 0; i < excess.size(); ++i) {
		if (excess[i] > 0) c++;
	}
	return c;
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

int FlowGraph::outCap(int source) {
	int c = 0;
	for (int i = 0; i < G[source].size(); ++i) {
		c += G[source][i].cap;
	}
	return c;
}

int FlowGraph::inCap(int sink) {
	int c = 0;
	for (int i = 0; i < G[sink].size(); ++i) {
		c += G[G[sink][i].to][G[sink][i].index].cap;
	}
	return c;
}

