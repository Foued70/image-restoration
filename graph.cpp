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
	G[from].e.push_back(Edge(from, to, cap, G[to].e.size()));
	if (from == to) G[from].e.back().index++;
	int index = G[from].e.size() - 1;
	G[to].e.push_back(Edge(to, from, 0, index));

	if (from == source)
		G[to].si = index;

	if (to == sink)
		G[from].ti = index;
}

/*
 * Add an edge and at the same time an antiparallel edge
 * with the same capacity.
 */
void FlowGraph::addDoubleEdge(int from, int to, int cap) {
	G[from].e.push_back(Edge(from, to, cap, G[to].e.size()));
	G[to].e.push_back(Edge(to, from, cap, G[from].e.size() - 1));
}

#ifdef PUSH_RELABEL

/*
 * Change the capacity of an edge. Need the from-vertex and
 * the index of the edge in its edge list (returned from addEdge.
 */
void FlowGraph::changeCapacity(int from, int to, int cap) {
	int index;
	if (from == source) {
		index = G[to].si;
	} else if (to == sink) {
		index = G[from].ti;
	} else {
		exit(1);
	}

	int diff = G[from].e[index].flow - cap;

	G[from].e[index].cap = cap;

	/* Check if we need to reduce the flow. */
	if (diff > 0) {
		G[from].excess += diff;
		G[to].excess -= diff;
		G[from].e[index].flow = cap;
		G[to].e[G[from].e[index].index].flow = -cap;
		rule.add(from, G[from].height, G[from].excess);
	}
}

#else

/*
 * Change the capacity of an edge. Need the from-vertex and
 * the index of the edge in its edge list (returned from addEdge)
 */
void FlowGraph::changeCapacity(int from, int to, int cap) {
	int index;
	if (from == source) {
		index = G[to].si;
	} else if (to == sink) {
		index = G[from].ti;
	} else {
		exit(1);
	}

	/* Nodes in the S set can not send any more flow anyways. */
	if (from == source && G[to].c == SOURCE)
		return;

	G[from].e[index].cap = cap;

	if (from != source)
		return;

	/* Push flow along two-edged path. */
	int si = G[to].si;
	int ti = G[to].ti;

	Edge *sv, *vt;
	sv = &G[source].e[si];
	vt = &G[to].e[ti];

	int rs = sv->cap - sv->flow;
	int rt = vt->cap - vt->flow;

	if (rs > 0 && rt > 0) {
		int m = min(rs, rt);
		push(*sv, m);
		push(*vt, m);

		if (m == rs && G[to].p == sv) {
			G[to].p = NULL;
			orphans.push(to);
		}

		if (m == rt && G[to].p == vt) {
			G[to].p = NULL;
			orphans.push(to);
		}
	}

	/* Activate vertices if the new edge was not saturated. */
	if (G[from].e[si].flow != G[from].e[si].cap) {
		if (!G[from].active) {
			bkq.push(from);
			G[from].active = true;
		}
		if (!G[to].active) {
			bkq.push(to);
			G[to].active = true;
		}
	}
}

#endif

/* Reset all flow and excess. */
void FlowGraph::resetFlow() {
	for (size_t i = 0; i < G.size(); ++i) {
		for (size_t j = 0; j < G[i].e.size(); ++j) {
			G[i].e[j].flow = 0;
		}
		G[i].excess = 0;
	}
}

/* Reset all distance labels. */
void FlowGraph::resetHeights() {
	for (size_t i = 0; i < G.size(); ++i) {
		G[i].height = 0;
	}
	fill(count.begin(), count.end(), 0);
}

/* Push along an edge. */
void FlowGraph::push(Edge &e) {
	int flow = min(e.cap - e.flow, G[e.from].excess);
	G[e.from].excess -= flow;
	G[e.to].excess   += flow;
	e.flow += flow;
	G[e.to].e[e.index].flow -= flow;

	rule.add(e.to, G[e.to].height, G[e.to].excess);
}

/* Push given flow along an edge. */
void FlowGraph::push(Edge &e, int f) {
	e.flow += f;
	G[e.to].e[e.index].flow -= f;
}

/* Relabel a vertex. */
void FlowGraph::relabel(int u) {
	count[G[u].height]--;
	G[u].height = 2*N;

	for (size_t i = 0; i < G[u].e.size(); ++i) {
		if (G[u].e[i].cap > G[u].e[i].flow) {
			G[u].height = min(G[u].height, G[G[u].e[i].to].height + 1);
		}
	}

	if (G[u].height >= N) {
		G[u].height = N;
	}
	else {
		count[G[u].height]++;
		rule.add(u, G[u].height, G[u].excess);
	}
}

/* Relabel all vertices over the gap h to label N. */
void FlowGraph::gap(int h) {
	for (size_t i = 0; i < G.size(); ++i) {
		if (G[i].height < h) continue;
		if (G[i].height >= N) continue;

		rule.deactivate(i);

		count[G[i].height]--;
		G[i].height = N;
	}

	rule.gap(h);
}

/* Discharge a vertex. */
void FlowGraph::discharge(int u) {
	size_t i;
	for (i = 0; i < G[u].e.size() && G[u].excess > 0; ++i) {
		if (G[u].e[i].cap > G[u].e[i].flow
				&& G[u].height == G[G[u].e[i].to].height + 1) {
			push(G[u].e[i]);
		}
	}

	if (G[u].excess > 0) {
		/* Check if a gap will appear. */
		if (count[G[u].height] == 1)
			gap(G[u].height);
		else
			relabel(u);
	}
}

/* Run the push-relabel algorithm to find the min-cut. */
void FlowGraph::minCutPushRelabel(int source, int sink) {
	G[source].height = N;

	rule.activate(source);
	rule.activate(sink);

	for (size_t i = 0; i < G[source].e.size(); ++i) {
		G[source].excess = G[source].e[i].cap;
		push(G[source].e[i]);
	}
	G[source].excess = 0;

	int c = 0;
	/* Loop over active nodes using selection rule. */
	while (!rule.empty()) {
		c++;
		int u = rule.next();
		discharge(u);
	}

	/* Output the cut based on vertex heights. */
	for (size_t i = 0; i < cut.size(); ++i) {
		cut[i] = G[i].height >= N;
	}
}

/* The capacity of the edge, in the direction given by the tree. */
int FlowGraph::treeCap(const Edge& e, Color col) const {
	if (col == SOURCE)
		return e.cap - e.flow;
	else if (col == SINK)
		return G[e.to].e[e.index].cap - G[e.to].e[e.index].flow;
	else
		return 0;
}

/* Try to grow the trees S and T from the active vertices. */
Edge *FlowGraph::grow() {
	while (!bkq.empty()) {
		int p = bkq.front();
		if (!G[p].active) {
			bkq.pop();
			continue;
		}

		size_t i = 0;

		/*
		 * If we're growing from the same vertex as before,
		 * continue with the same index, if not restart at 0.
		 */
		if (lastGrowVertex == p)
			i = lastIndex;

		lastGrowVertex = p;

		for (; i < G[p].e.size(); ++i) {
			lastIndex = i;
			Edge *e = &G[p].e[i];
			int q = e->to;

			if (G[p].c == G[q].c)
				continue;

			if (treeCap(*e, G[p].c) <= 0)
				continue;

			if (G[q].c == FREE) {
				/* Found a free vertex, add it. */
				G[q].c = G[p].c;

				int len;
				if (G[p].c == SOURCE) {
					G[q].p = e;
					assert(treeOrigin(p, len) == source);
				} else if (G[p].c == SINK) {
					G[q].p = &G[q].e[e->index];
					assert(treeOrigin(p, len) == sink);
				} else {
					cout << G[p].c << endl;
					exit(1);
				}
				(void)len;

				G[q].active = 1;
				bkq.push(q);
			}
			else if (G[q].c != G[p].c) {
				/* The trees meet! */
				if (G[p].c == SOURCE) {
					return e;
				}
				else if (G[p].c == SINK) {
					return &G[q].e[e->index];
				}
				else {
					exit(1);
				}

				return NULL;
			}
		}

		bkq.pop();
		G[p].active = 0;
	}

	/* Path is empty */
	return NULL;
}

/* Augment along path given by the edge e, and implicitly by the trees. */
int FlowGraph::augment(Edge* e) {
	int m = e->cap - e->flow;

	/* Find maximum flow we can send. */
	Edge *cur = e;
	while (cur != NULL) {
		m = min(m, cur->cap - cur->flow);
		cur = G[cur->from].p;
	}

	cur = e;
	while (cur != NULL) {
		m = min(m, cur->cap - cur->flow);
		cur = G[cur->to].p;
	}

	cur = e;
	bool back = true;
	int len = 0;
	/* Loop through path and update flow. */
	while (cur != NULL) {
		/* If saturated, we must orphanize. */
		if (cur->cap - cur->flow == m) {
			int u = cur->from;
			int v = cur->to;

			if (G[u].c == SOURCE && G[v].c == SOURCE) {
				if (v != source && v != sink) {
					orphans.push(v);
					G[v].p = NULL;
				}
			}
			if (G[u].c == SINK && G[v].c == SINK) {
				if (u != source && u != sink) {
					orphans.push(u);
					G[u].p = NULL;
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
			cur = G[cur->from].p;
			if (cur == NULL) {
				back = false;
				cur = G[e->to].p;
			}
		} else {
			cur = G[cur->to].p;
		}
	}
	return len;
}

/* Find the origin of vertex u. */
int FlowGraph::treeOrigin(int u, int &len) const {
	int cur = u;
	len = 0;

	if (G[cur].c == SOURCE) {
		while (G[cur].p != NULL) {
			cur = G[cur].p->from;
			len++;
		}
	} else if (G[cur].c == SINK) {
		while (G[cur].p != NULL) {
			cur = G[cur].p->to;
			len++;
		}
	} else {
		exit(1);
	}

	return cur;
}

/* Adopt orphans. */
void FlowGraph::adopt() {
	while (orphans.size() > 0) {
		int u = orphans.front();
		orphans.pop();

		assert(G[u].c != FREE);

		int minlen = 1000000000;
		int minidx = -1;
		/* Aim to find parent close to the root of the tree. */
		for (size_t i = 0; i < G[u].e.size(); ++i) {
			int v = G[u].e[i].to;

			if (G[u].c != G[v].c)
				continue;

			if (treeCap(G[v].e[G[u].e[i].index], G[u].c) <= 0)
				continue;

			int len;
			int origin = treeOrigin(v, len);
			if (origin != source && origin != sink)
				continue;

			if (len < minlen) {
				minlen = len;
				minidx = i;
			}
			if (minlen <= 2) break;
			/* Found a possible parent */
		}

		bool found = false;
		if (minidx != -1) {
			int i = minidx;
			int v = G[u].e[i].to;
			int len;
			int origin = treeOrigin(v, len);

			if (origin == source) {
				G[u].p = &G[v].e[G[u].e[i].index];
				found = true;
			} else if (origin == sink) {
				G[u].p = &G[u].e[i];
				found = true;
			} else {
				exit(1);
			}
		}

		/* If not found, free vertex, and orphanize possible children. */
		if (!found) {
			for (size_t i = 0; i < G[u].e.size(); ++i) {
				int v = G[u].e[i].to;

				if (G[u].c != G[v].c)
					continue;

				if (treeCap(G[v].e[G[u].e[i].index], G[v].c) > 0) {
					G[v].active = true;
					bkq.push(v);
				}

				if (v == source || v == sink)
					continue;

				if (G[v].p
						&& (G[v].p->to == u
						||  G[v].p->from == u)) {
					orphans.push(v);
					G[v].p = NULL;
				}
			}

			G[u].c = FREE;

			G[u].active = false;
			/* We might still have u in the queue */
		}
	}
}

void FlowGraph::minCutBK(int source, int sink) {
	lastGrowVertex = -1;
	adopt();

	int numpaths  = 0;
	double totlen = 0;
	while (true) {
		Edge *e;
		e = grow();

		if (e == NULL) {
			/* Empty path. */
			break;
		}

		totlen += augment(e);
		numpaths++;
		adopt();
	}

	cout << "Avg length: " << double(totlen) / double(numpaths) << endl;

	int size1 = 0, size2 = 0;
	for (size_t i = 0; i < cut.size(); ++i) {
		if (G[i].c == SOURCE) size1++;
		else if (G[i].c == SINK) size2++;
		cut[i] = G[i].c == SOURCE;
	}
	assert(checkCapacity());
	assert(checkActive());
	cout << "Inbetweeners: " << cut.size() - size1 - size2 << endl;
}

bool FlowGraph::checkExcess(void) {
	for (size_t i = 0; i < G.size(); ++i) {
		if (G[i].excess < 0) {
			return false;
		}
	}

	return true;
}

bool FlowGraph::checkTree(void) {
	bool ret = true;
	for (size_t i = 0; i < G.size(); ++i) {
		if (G[i].c == FREE) continue;
		int len;
		if ((treeOrigin(i, len) != getSource() && G[i].c == SOURCE)
				|| (treeOrigin(i, len) != getSink() && G[i].c == SINK)) {
			cout << "Origin of " << i << " is " << treeOrigin(i, len) << endl;
			ret = false;
		}
		if (G[i].p == NULL) continue;
		if (G[i].p->cap - G[i].p->flow <= 0) {
			cout << "ERR: " << G[i].p->from << " -> " << G[i].p->to;
			cout << " has cap: " << G[i].p->cap << endl;
			ret = false;
		}
	}
	return ret;
}

bool FlowGraph::checkActive(void) {
	bool ret = true;
	for (size_t i = 0; i < G.size(); ++i) {
		if (G[i].active) {
			cout << i << " is active." << endl;
			ret = false;
		}
	}

	return ret;
}

int FlowGraph::numActive(void) {
	int num = 0;
	for (size_t i = 0; i < G.size(); ++i) {
		if (G[i].active) {
			num++;
		}
	}

	return num;
}

bool FlowGraph::checkCapacity(void) {
	for (size_t i = 0; i < G.size(); ++i) {
		for (size_t j = 0; j < G[i].e.size(); ++j) {
			if (G[i].e[j].flow > G[i].e[j].cap) {
				cout << i << " " << j << endl;
				return false;
			}
		}
	}

	return true;
}

bool FlowGraph::checkLabels(void) {
	for (size_t i = 0; i < G.size(); ++i) {
		for (size_t j = 0; j < G[i].e.size(); ++j) {
			if (G[i].e[j].flow < G[i].e[j].cap) {
				if (G[i].height > G[G[i].e[j].to].height + 1) {
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
		for (size_t j = 0; j < G.size(); ++j) {
			if ((unsigned)G[j].height == i) c++;
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
	for (size_t i = 0; i < G[source].e.size(); ++i) {
		c += G[source].e[i].flow;
	}
	return c;
}

int FlowGraph::inFlow(int sink) {
	int c = 0;
	for (size_t i = 0; i < G[sink].e.size(); ++i) {
		c += G[sink].e[i].flow;
	}
	return c;
}

