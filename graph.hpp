#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <list>
#include <set>
#include "selectionrule.hpp"

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

class FlowGraph {
private:
	int N;
	int source, sink;
	std::vector<std::vector<Edge> > G;
	std::vector<int> excess;
	std::vector<int> height;
	std::vector<int> s_index;
	std::vector<int> t_index;
	std::vector<int> count;
	SelectionRule& rule;

	/* BK stuff */
	std::vector<int> active;
	std::vector<int> color; // 0 = free, 1 = source, 2 = sink
	std::vector<Edge*> parent;
	std::queue<int> bkq;
	std::vector<int> orphans;
public:
	std::vector<char> cut;

	FlowGraph(int N, int source, int sink, SelectionRule& rule) :
		N(N),
		source(source),
		sink(sink),
		G(N),
		excess(N),
		height(N),
		s_index(N),
		t_index(N),
		count(N+1),
       		rule(rule),
		active(N),
		color(N),
		parent(N),
		cut(N) {}

	int getSource() { return source; }
	int getSink() { return sink; }
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
	void augment(const std::list<Edge*>& path);
	void initBK(int source, int sink);
	int treeCap(int p, int i, int col);
	int treeCap(Edge& e, int col);
	int treeOrigin(int u);
	void adopt();
	void grow(std::list<Edge*>& path);

	int outFlow(int source);
	int inFlow(int sink);

	bool checkExcess(void);
	bool checkActive(void);
	bool checkCapacity(void);
	bool checkLabels(void);
	bool checkCount(void);
};

