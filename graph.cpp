#include <iostream>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstdlib>

#include "graph.hpp"

using namespace std;

/* Add an edge from one vertex to another. */
void FlowGraph::addEdge(int from, int to, int cap) {
	G[from].push_back(Edge(from, to, cap, G[to].size()));
	if (from == to) G[from].back().index++;
	int index = G[from].size() - 1;
	G[to].push_back(Edge(to, from, 0, index));

	if (from == source)
		s_index[to] = index;

	if (to == sink)
		t_index[from] = index;
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
void FlowGraph::changeCapacity(int from, int to, int cap) {
	int index;
	if (from == source) {
		index = s_index[to];
	} else if (to == sink) {
		index = t_index[from];
	} else {
		index = 0;
		exit(1);
	}

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

/* Reset all flow and excess. */
void FlowGraph::resetFlow() {
	for (size_t i = 0; i < G.size(); ++i) {
		for (size_t j = 0; j < G[i].size(); ++j) {
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

/* Push given flow along an edge. */
void FlowGraph::push(Edge &e, int f) {
	e.flow += f;
	G[e.to][e.index].flow -= f;
}

/* Relabel a vertex. */
void FlowGraph::relabel(int u) {
	count[height[u]]--;
	height[u] = 2*N;

	for (size_t i = 0; i < G[u].size(); ++i) {
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
	for (size_t i = 0; i < G.size(); ++i) {
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
	size_t i;
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

	for (size_t i = 0; i < G[source].size(); ++i) {
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
	for (size_t i = 0; i < cut.size(); ++i) {
		cut[i] = height[i] >= N;
	}
}

/*
 * Send as much flow as possible through paths of length
 * at most 2, and set up the status of the vertices.
 */
void FlowGraph::initBK(int source, int sink) {
	fill(active.begin(), active.end(), 0);
	fill(color.begin(), color.end(), 0);
	fill(parent.begin(), parent.end(), (Edge*)NULL);
	orphans.clear();
	std::queue<int> empty;
	std::swap(bkq, empty);
	resetFlow();

	for (int i = 0; i < G[source].size(); ++i) {

	}

	active[source] = 1;
	active[sink]   = 1;
	color[source]  = 1;
	color[sink]    = 2;

	bkq.push(source);
	bkq.push(sink);
}

int FlowGraph::treeCap(int p, int i, int col) {
	if (col == 1)
		return G[p][i].cap - G[p][i].flow;
	else if (col == 2)
		return G[G[p][i].to][G[p][i].index].cap
			- G[G[p][i].to][G[p][i].index].flow;
	else
		return 0;
}

int FlowGraph::treeCap(Edge& e, int col) {
	if (col == 1)
		return e.cap - e.flow;
	else if (col == 2)
		return G[e.to][e.index].cap - G[e.to][e.index].flow;
	else
		return 0;
}

void FlowGraph::grow(vector<Edge*>& path) {
	assert(path.empty());

	while (!bkq.empty()) {
		int p = bkq.front();
		if (!active[p]) {
			bkq.pop();
			continue;
		}

		for (int i = 0; i < G[p].size(); ++i) {
			int q = G[p][i].to;
			if (treeCap(G[p][i], color[p]) <= 0)
				continue;

			if (color[q] == 0) {
				color[q] = color[p];

				//cout << "Parent of " << q << " is now " << p << endl;
				if (color[p] == 1) {
					parent[q] = &G[p][i];
					assert(treeOrigin(p) == source);
				} else if (color[p] == 2) {
					parent[q] = &G[q][G[p][i].index];
					assert(treeOrigin(p) == sink);
				} else {
					cout << color[p] << endl;
					assert(0);
				}

				active[q] = 1;
				bkq.push(q);
			}
			else if (color[q] != color[p]) {
				int u, v;

				if (color[p] == 1) {
					u = p;
					v = q;
					path.push_back(&G[p][i]);
					//cout << "path += " << G[p][i].from;
					//cout << "-" << G[p][i].to << endl;
				}
				else if (color[p] == 2) {
					u = q;
					v = p;
					path.push_back(&G[q][G[p][i].index]);
					//cout << "path += " << G[q][G[p][i].index].from;
					//cout << "-" << G[q][G[p][i].index].to << endl;
				}
				else {
					assert(0);
				}
				//cout << "The trees meet! " << u << " -> " << v << endl;

				/* First pushback from cut to s */
				int cur = u;
				while (cur != source) {
					path.push_back(parent[cur]);
					//cout << "path += " << parent[cur]->from << "-";
					//cout << parent[cur]->to << endl;
					cur = parent[cur]->from;
				}

				/* Then reverse */
				reverse(path.begin(), path.end());
				//cout << "reverse" << endl;

				/* Then pushback from cut to t */
				cur = v;
				while (cur != sink) {
					path.push_back(parent[cur]);
					//cout << "path += " << parent[cur]->from << "-";
					//cout << parent[cur]->to << endl;
					cur = parent[cur]->to;
				}

				return;
			}
		}

		bkq.pop();
		active[p] = 0;
	}

	/* Path is empty */
}

void FlowGraph::augment(vector<Edge*>& path) {
	assert(path.size() >= 1);

	int m = 1000000000;

	for (int i = 0; i < path.size(); ++i) {
		//cout << path[i]->from << " -> ";
		m = min(m, path[i]->cap - path[i]->flow);
	}
	//cout << path[path.size()-1]->to;
	//cout << " : " << m << endl;
	//cout << "Saturation: " << m << endl;

	assert(path[0]->from == source);
	assert(path[path.size()-1]->to == sink);

	for (int i = 0; i < path.size(); ++i) {
		/* Check for saturation */
		if (path[i]->cap - path[i]->flow == m) {
			int u = path[i]->from;
			int v = path[i]->to;

			//cout << "Saturation at " << u << " -> " << v << endl;

			if (color[u] == 1 && color[v] == 1) {
				if (v != source && v != sink) {
					orphans.push_back(v);
					parent[v] = NULL;
				}
			}
			if (color[u] == 2 && color[v] == 2) {
				if (u != source && u != sink) {
					orphans.push_back(u);
					parent[u] = NULL;
				}
			}
		}
		push(*(path[i]), m);
	}
}

int FlowGraph::treeOrigin(int u) {
	int cur = u;
	while (parent[cur] != NULL) {
		if (color[cur] == 1)
			cur = parent[cur]->from;
		else if (color[cur] == 2)
			cur = parent[cur]->to;
		else
			assert(0);
	}

	return cur;
}

void FlowGraph::adopt() {
	while (orphans.size() > 0) {
		int u = orphans.back();
		//cout << "Orphan: " << u << endl;
		orphans.pop_back();

		assert(color[u] != 0);

		bool found = false;
		for (int i = 0; i < G[u].size() && !found; ++i) {
			int v = G[u][i].to;

			if (color[u] != color[v])
				continue;

			if (treeCap(G[v][G[u][i].index], color[u]) <= 0)
				continue;

			int origin = treeOrigin(v);
			if (origin != source && origin != sink)
				continue;

			//cout << "Possible parent: " << v << endl;
			/* Found a possible parent */

			//cout << "Parent of " << u << " is now " << v << endl;
			if (origin == source) {
				parent[u] = &G[v][G[u][i].index];
				found = true;
			} else if (origin == sink) {
				parent[u] = &G[u][i];
				found = true;
			} else {
				assert(0);
			}
		}

		if (!found) {
			for (int i = 0; i < G[u].size(); ++i) {
				int v = G[u][i].to;

				if (color[u] != color[v])
					continue;

				if (treeCap(G[v][G[u][i].index], color[v]) > 0) {
					active[v] = 1;
					bkq.push(v);
				}

				if (v == source || v == sink)
					continue;

				if (parent[v] && (parent[v]->to == u || parent[v]->from == u)) {
					orphans.push_back(v);
					parent[v] = NULL;
				}
			}

			color[u] = 0;

			active[u] = 0;
			/* We might still have u in the queue */
		}
	}
}

void FlowGraph::minCutBK(int source, int sink) {
	initBK(source, sink);

	while (true) {
		vector<Edge*> path;
		//cout << "Growing path" << endl;
		grow(path);

		if (path.empty()) {
			//cout << "Path was empty..." << endl;
			break;
		}

		//cout << "Augmenting along path" << endl;
		augment(path);
		//cout << "Adopting orphans" << endl;
		adopt();
	}

	for (int i = 0; i < cut.size(); ++i) {
		if (color[i] == 0) cout << "boop" << endl;
		cut[i] = color[i] == 1;
	}
	assert(checkCapacity());
	assert(checkActive());
}

bool FlowGraph::checkExcess(void) {
	for (size_t i = 0; i < excess.size(); ++i) {
		if (excess[i] < 0) {
			return false;
		}
	}

	return true;
}

bool FlowGraph::checkActive(void) {
	for (int i = 0; i < active.size(); ++i) {
		if (active[i] != 0) {
			return false;
		}
	}

	return true;
}

bool FlowGraph::checkCapacity(void) {
	for (size_t i = 0; i < G.size(); ++i) {
		for (size_t j = 0; j < G[i].size(); ++j) {
			if (G[i][j].flow > G[i][j].cap) return false;
		}
	}

	return true;
}

bool FlowGraph::checkLabels(void) {
	for (size_t i = 0; i < G.size(); ++i) {
		for (size_t j = 0; j < G[i].size(); ++j) {
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
	for (size_t i = 0; i < count.size(); ++i) {
		int c = 0;
		for (size_t j = 0; j < height.size(); ++j) {
			if ((unsigned)height[j] == i) c++;
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
	for (size_t i = 0; i < G[source].size(); ++i) {
		c += G[source][i].flow;
	}
	return c;
}

int FlowGraph::inFlow(int sink) {
	int c = 0;
	for (size_t i = 0; i < G[sink].size(); ++i) {
		c += G[sink][i].flow;
	}
	return c;
}

