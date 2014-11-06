#pragma once

#include <iostream>
#include <queue>
#include <vector>
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
	std::vector<int> count;
	std::vector<int> s_index;
	std::vector<int> t_index;
	SelectionRule& rule;

public:
	std::vector<char> cut;

	FlowGraph(int N, int source, int sink, SelectionRule& rule) :
		N(N),
		G(N),
		excess(N),
		height(N),
		cut(N),
		s_index(N),
		t_index(N),
		source(source),
		sink(sink),
		count(N+1),
       		rule(rule) {}

	int getSource() { return source; }
	int getSink() { return sink; }
	void addEdge(int from, int to, int cap);
	void addDoubleEdge(int from, int to, int cap);
	void changeCapacity(int from, int index, int cap);
	void resetFlow();
	void resetHeights();

	void push(Edge &e);
	void relabel(int u);
	void gap(int h);
	void discharge(int u);
	void minCutPushRelabel(int source, int sink);

	int outFlow(int source);
	int inFlow(int sink);

	bool checkExcess(void);
	bool checkCapacity(void);
	bool checkLabels(void);
	bool checkCount(void);
};

