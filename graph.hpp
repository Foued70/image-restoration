#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <list>
#include <set>
#include "selectionrule.hpp"

enum Color { FREE = 0, SOURCE, SINK };

class Edge {
private:

public:
	int from, to;
	int cap;
	int flow;
	int index;

	Edge(int f, int t, int c, int i) :
		from(f), to(t), cap(c), flow(0), index(i) {}
};

class Vertex {
private:

public:
	std::vector<Edge> e;
	Color c;
	bool active;
	Edge *p;
	int height;
	int excess;
	int si;
	int ti;

	Vertex(Color c, bool a) :
		c(c), active(a), p(NULL), height(0), excess(0), si(0), ti(0) {}

	Vertex() :
		c(FREE), active(false), p(NULL), height(0), excess(0), si(0), ti(0) {}
};

class FlowGraph {
private:
	int N;
	int source, sink;
	std::vector<Vertex > G;
	std::vector<int> count;
	SelectionRule& rule;

	/* BK stuff */
	std::queue<int> bkq;
	std::queue<int> orphans;

	int lastGrowVertex;
	size_t lastIndex;
public:
	std::vector<char> cut;

	FlowGraph(int N, int source, int sink, SelectionRule& rule) :
		N(N),
		source(source),
		sink(sink),
		G(N),
		count(N+1),
		rule(rule),
		lastGrowVertex(-1),
		cut(N) {

#ifdef BOYKOV_KOLMOGOROV
		bkq.push(source);
		bkq.push(sink);
		G[source].c = SOURCE;
		G[sink].c   = SINK;
		G[source].active = true;
		G[sink].active   = true;
#endif
	}

	int getSource() const { return source; }
	int getSink() const { return sink; }
	void addEdge(int from, int to, int cap);
	void addDoubleEdge(int from, int to, int cap);
	void changeCapacity(int from, int index, int cap);
	void resetFlow();
	void resetHeights();

	void push(Edge &e);
	void push(Edge &e, int f);
	void relabel(int u);
	void gap(int h);
	void discharge(int u);
	void minCutPushRelabel(int source, int sink);

	void minCutBK(int source, int sink);
	int augment(Edge *e);
	int treeCap(const Edge& e, Color col) const;
	int treeOrigin(int u, int &len) const;
	void adopt();
	Edge *grow();

	int outFlow(int source);
	int inFlow(int sink);

	int numActive(void);
	bool checkExcess(void);
	bool checkTree(void);
	bool checkActive(void);
	bool checkCapacity(void);
	bool checkLabels(void);
	bool checkCount(void);
};

