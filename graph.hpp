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
	std::vector<std::vector<Edge> > G;
	std::vector<int> excess;
	std::vector<int> height;
	std::vector<int> count;
	SelectionRule& rule;

public:
	std::vector<char> cut;

	FlowGraph(int N, SelectionRule& rule) :
		N(N),
		G(N),
		excess(N),
		height(N),
		cut(N),
		count(N+1),
       		rule(rule) {}

	int addEdge(int from, int to, int cap);
	void changeCapacity(int from, int index, int cap);
	void setValue(int i, int v);
	void resetFlow();
	void resetHeights();
	void reset(std::set<int>);

	void push(Edge &e);
	void relabel(int u);
	void gap(int h);
	void discharge(int u);
	double globalRelabel(int source, int sink);
	void minCutPushRelabel(int source, int sink);

	void minCutDinic(int source, int sink);
	int maxFlowDinic(int source, int sink);
	int blockingFlow(std::vector<int> &level, int u,
			int source, int sink, int limit);
	void DFS(int source, int sink);

	void printGraph(void);
	void printHeights(void);

	int activeNodes(void);
	int outFlow(int source);
	int inFlow(int sink);
	int outCap(int source);
	int inCap(int sink);
	int totalHeight(void);

	bool checkExcess(void);
	bool checkCapacity(void);
	bool checkLabels(void);
	bool checkCount(void);
};

