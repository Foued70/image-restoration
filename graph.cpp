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
 * the index of the edge in its edge list (returned from addEdge)
 */
void FlowGraph::changeCapacity(int from, int to, int cap) {
	int index;
	if (from == source) {
		index = s_index[to];
	} else if (to == sink) {
		index = t_index[from];
	} else {
		exit(1);
	}

	G[from][index].cap = cap;

	if (to != sink)
		return;

	int si = s_index[from];
	int ti = t_index[from];

	int rs = G[source][si].cap - G[source][si].flow;
	int rt = cap - G[from][ti].flow;

	if (rs > 0 && rt > 0) {
		int m = min(rs, rt);
		push(G[source][si], m);
		push(G[from][ti], m);

		if (m == rs && parent[to] == &G[from][G[source][si].index]) {
			parent[to] = NULL;
			orphans.push(to);
		} else if (m == rt && parent[to] == &G[from][ti]) {
			parent[to] = NULL;
			orphans.push(to);
		}
	}

	if (G[from][ti].flow != G[from][ti].cap) {
		if (!active[from]) {
			bkq.push(from);
			active[from] = 1;
		}
		if (!active[to]) {
			bkq.push(to);
			active[to] = 1;
		}
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

int FlowGraph::treeCap(const Edge& e, Color col) const {
	if (col == SOURCE)
		return e.cap - e.flow;
	else if (col == SINK)
		return G[e.to][e.index].cap - G[e.to][e.index].flow;
	else
		return 0;
}

Edge *FlowGraph::grow() {
	static size_t i = 0;

	while (!bkq.empty()) {
		int p = bkq.front();
		if (!active[p]) {
			bkq.pop();
			continue;
		}
		//cout << "Growing from (" << G[p].size() << "): " << p << endl;

		if (lastGrowVertex != p)
			i = 0;

		lastGrowVertex = p;

		for (; i < G[p].size(); ++i) {
			Edge *e = &G[p][i];
			int q = e->to;

			if (color[p] == color[q])
				continue;

			if (treeCap(*e, color[p]) <= 0)
				continue;

			if (color[q] == FREE) {
				color[q] = color[p];

				//cout << "Parent of " << q << " is now " << p << endl;
				if (color[p] == SOURCE) {
					parent[q] = e;
					assert(treeOrigin(p) == source);
				} else if (color[p] == SINK) {
					parent[q] = &G[q][e->index];
					assert(treeOrigin(p) == sink);
				} else {
					cout << color[p] << endl;
					exit(1);
				}

				active[q] = 1;
				bkq.push(q);
			}
			else if (color[q] != color[p]) {

				//cout << "The trees meet! " << p << " -> " << q << endl;
				if (color[p] == SOURCE) {
					return e;
				}
				else if (color[p] == SINK) {
					return &G[q][e->index];
				}
				else {
					exit(1);
				}

				return NULL;
			}
		}

		bkq.pop();
		active[p] = 0;
	}

	/* Path is empty */
	return NULL;
}

int FlowGraph::augment(Edge* e) {
	int m = e->cap - e->flow;

	Edge *cur = e;
	while (cur != NULL) {
		m = min(m, cur->cap - cur->flow);
		cur = parent[cur->from];
	}

	cur = e;
	while (cur != NULL) {
		m = min(m, cur->cap - cur->flow);
		cur = parent[cur->to];
	}
	//cout << path[path.size()-1]->to;
	//cout << " : " << m << endl;
	//cout << "Saturation: " << m << endl;

	//assert(path[0]->from == source);
	//assert(path[path.size()-1]->to == sink);

	cur = e;
	bool back = true;
	int len = 0;
	while (cur != NULL) {
		/* Check for saturation */
		if (cur->cap - cur->flow == m) {
			int u = cur->from;
			int v = cur->to;

			//cout << "Saturation at " << u << " -> " << v << endl;

			if (color[u] == SOURCE && color[v] == SOURCE) {
				if (v != source && v != sink) {
					orphans.push(v);
					parent[v] = NULL;
				}
			}
			if (color[u] == SINK && color[v] == SINK) {
				if (u != source && u != sink) {
					orphans.push(u);
					parent[u] = NULL;
				}
			}
		}
		len++;
		push(*cur, m);

		/*
		 * If we reach the source, we must start again
		 * in e, and go towards the sink.
		 */
		if (back) {
			cur = parent[cur->from];
			if (cur == NULL) {
				back = false;
				cur = parent[e->to];
			}
		} else {
			cur = parent[cur->to];
		}
	}
	return len;
}

int FlowGraph::treeOrigin(int u) const {
	int cur = u;

	if (color[cur] == SOURCE) {
		while (parent[cur] != NULL) {
			cur = parent[cur]->from;
		}
	} else if (color[cur] == SINK) {
		while (parent[cur] != NULL) {
			cur = parent[cur]->to;
		}
	} else {
		exit(1);
	}

	return cur;
}

void FlowGraph::adopt() {
	while (orphans.size() > 0) {
		int u = orphans.front();
		//cout << "Orphan: " << u << endl;
		orphans.pop();

		assert(color[u] != FREE);

		bool found = false;
		for (size_t i = 0; i < G[u].size() && !found; ++i) {
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
			for (size_t i = 0; i < G[u].size(); ++i) {
				int v = G[u][i].to;

				if (color[u] != color[v])
					continue;

				if (treeCap(G[v][G[u][i].index], color[v]) > 0) {
					active[v] = 1;
					bkq.push(v);
				}

				if (v == source || v == sink)
					continue;

				if (parent[v]
						&& (parent[v]->to == u
						||  parent[v]->from == u)) {
					orphans.push(v);
					parent[v] = NULL;
				}
			}

			color[u] = FREE;

			active[u] = 0;
			/* We might still have u in the queue */
		}
	}
}

void FlowGraph::minCutBK(int source, int sink) {
	lastGrowVertex = -1;
	adopt();

	//cout << "Num active: " << numActive() << endl;
	int numpaths = 0;
	int totlen   = 0;
	while (true) {
		Edge *e;
		//cout << "Growing path" << endl;
		//printQ(bkq);
		e = grow();

		if (e == NULL) {
			//cout << "Path was empty..." << endl;
			break;
		}

		//cout << "Augmenting along path" << endl;
		totlen += augment(e);
		numpaths++;
		//cout << "Adopting orphans" << endl;
		adopt();
	}

	//cout << "Avg length: " << double(totlen) / double(numpaths) << endl;

	int size1 = 0, size2 = 0;
	for (size_t i = 0; i < cut.size(); ++i) {
		if (color[i] == SOURCE) size1++;
		else if (color[i] == SINK) size2++;
		cut[i] = color[i] == SINK;
	}
	assert(checkCapacity());
	assert(checkActive());
	//cout << "Num active: " << numActive() << endl;
	cout << "Inbetweeners: " << cut.size() - size1 - size2 << endl;
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
	bool ret = true;
	for (size_t i = 0; i < active.size(); ++i) {
		if (active[i] != 0) {
			cout << i << " is active." << endl;
			ret = false;
		}
	}

	return ret;
}

int FlowGraph::numActive(void) {
	int num = 0;
	for (size_t i = 0; i < active.size(); ++i) {
		if (active[i] != 0) {
			num++;
		}
	}

	return num;
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

