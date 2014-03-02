#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <stack>
#include <cassert>
#include <algorithm>

using namespace std;

class Edge {
private:

public:
	int from, to;
	int cap;
	int flow;
	int index;

	Edge(int from, int to, int cap, int index) :
		from(from), to(to), cap(cap), flow(0), index(index) {}
};

class PushRelabel {
private:
	int N;
	vector<vector<Edge> > G;
	vector<int> excess;
	vector<int> height;
	vector<int> count;
	/* vector<bool> is slow because of bit operations */
	vector<char> active;
	vector<int> value;
	vector<queue<int> > hset;
	priority_queue<int> hq;
	queue<int> q;
	vector<char> inhq;
	int highest;
	//bool first;

public:
	vector<bool> cut;

	PushRelabel(int N) :
		N(N),
		G(N),
		excess(N),
		height(N),
		active(N),
		cut(N),
		count(N+1),
       		hset(256),
       		highest(0),
		inhq(256),
       		value(N) {}
       		//first(true) {}

	int AddEdge(int from, int to, int cap) {
		G[from].push_back(Edge(from, to, cap, G[to].size()));
		if (from == to) G[from].back().index++;
		int index = G[from].size() - 1;
		G[to].push_back(Edge(to, from, 0, index));

		return index;
	}

	void ChangeCapacity(int from, int index, int cap) {
		int to = G[from][index].to;
		int diff = G[from][index].flow - cap;

		G[from][index].cap = cap;

		if (diff > 0) {
			excess[from] += diff;
			excess[to] -= diff;
			G[from][index].flow = cap;
			G[to][G[from][index].index].flow = -cap;
			Enqueue(from);
		}

		//assert(excess[from] >= 0);
		//assert(excess[to]   >= 0);

		//Enqueue(to);
		//Enqueue(from);
	}

	void SetValue(int i, int v) {
		value[i] = v;
	}

	void ResetFlow() {
		for (int i = 0; i < G.size(); ++i) {
			for (int j = 0; j < G[i].size(); ++j) {
				G[i][j].flow = 0;
			}
			excess[i] = 0;
		}
	}
	
	void Push(Edge &e) {
		int flow = min(e.cap - e.flow, excess[e.from]);
		excess[e.from] -= flow;
		excess[e.to]   += flow;
		e.flow += flow;
		G[e.to][e.index].flow -= flow;
		Enqueue(e.to);
	}

	void Relabel(int u) {
		count[height[u]]--;
		height[u] = 2*N;

		for (int i = 0; i < G[u].size(); ++i) {
			if (G[u][i].cap > G[u][i].flow) {
				height[u] = min(height[u], height[G[u][i].to] + 1);
			}
		}

		if (height[u] >= N) {
			height[u] = N;
			count[N]++;
		}
		else {
			count[height[u]]++;
			Enqueue(u);
		}
	}

	//void Enqueue(int i) {
	//	if (active[i] || excess[i] == 0) return;
	//	if (height[i] >= N) return;
	//	active[i] = true;
	//	q.push(i);
	//}

	void Enqueue(int i) {
		if (active[i] || excess[i] == 0) return;
		if (height[i] >= N) return;
		active[i] = true;
		hset[value[i]].push(i);
		//if (highest < value[i]) highest = value[i];
		PushQ(value[i]);
	}

	void PrintHSet(void) {
		for (int i = 0; i <= 255; ++i) {
			const char *b = "()";
			if (i == highest) b = "[]";
			if (hset[i].size() > 0) {
				cout << b[0] << i << ", ";
				cout << hset[i].size() << b[1] << ", ";
			}
		}
		cout << endl;

		for (int i = N; i >= 0; --i) {
			if (count[i] > 0) {
				cout << "Highest node is at " << i << endl;
				break;
			}
		}
	}

	void Gap(int h) {
		int c = 0;
		for (int i = 0; i < G.size(); ++i) {
			if (height[i] < h) continue;
			if (height[i] == N) continue;
			count[height[i]]--;
			height[i] = N;
			active[i] = false;
			count[N]++;
		}

		// WTF MAN
		//for (int i = h; i < 256; ++i) {
		//	while (!hset[i].empty()) {
		//		int u = hset[i].front();
		//		hset[i].pop();
		//		active[u] = false; // FIXME
		//	}
		//}

		//highest = -1;
		//for (int i = h; i >= 0; --i) {
		//	if (hset[i].size() > 0) {
		//		highest = i;
		//		break;
		//	}
		//}
	}

	void Discharge(int u) {
		int i;
		for (i = 0; i < G[u].size() && excess[u] > 0; ++i) {
			if (G[u][i].cap > G[u][i].flow
					&& height[u] == height[G[u][i].to] + 1) {
				Push(G[u][i]);
			}
		}
		
		if (excess[u] > 0) {
			if (count[height[u]] == 1)
				Gap(height[u]);
			else
				Relabel(u);
		}
	}

	void PrintGraph(void) {
		for (int i = 0; i < G.size(); ++i) {
			cout << i << ": ";
			for (int j = 0; j < G[i].size(); ++j) {
				cout << G[i][j].to << "(" << G[i][j].cap;
				cout << ", " << G[i][j].flow;
				cout << "), ";
			}
			cout << endl;
		}
	}

	void PrintHeights(void) {
		for (int i = 0; i < height.size(); ++i) {
			cout << height[i] << " ";
		}
		cout << endl;
	}

	void DFS(int source, int sink) {
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

	int ActiveNodes(void) {
		int c = 0;
		for (int i = 0; i < active.size(); ++i) {
			if (active[i]) c++;
		}
		return c;
	}
	
	int OutFlow(int source) {
		int c = 0;
		for (int i = 0; i < G[source].size(); ++i) {
			c += G[source][i].flow;
		}
		return c;
	}

	int InFlow(int sink) {
		int c = 0;
		for (int i = 0; i < G[sink].size(); ++i) {
			c += G[sink][i].flow;
		}
		return c;
	}

	int OutCap(int source) {
		int c = 0;
		for (int i = 0; i < G[source].size(); ++i) {
			c += G[source][i].cap;
		}
		return c;
	}

	int InCap(int sink) {
		int c = 0;
		for (int i = 0; i < G[sink].size(); ++i) {
			c += G[G[sink][i].to][G[sink][i].index].cap;
		}
		return c;
	}

	int TotalHeight(void) {
		int c = 0;
		for (int i = 0; i < height.size(); ++i) {
			c += height[i];
		}
		return c;
	}

	int PopHighest(void) {
		int u = hq.top();
		hq.pop();
		inhq[u] = 0;
		return u;
	}

	int PushQ(int v) {
		if (!inhq[v]) {
			hq.push(v);
			inhq[v] = 1;
		}
	}

	void MinCutPushRelabel(int source, int sink) {
		//fill(count.begin(), count.end(), 0);
		//fill(height.begin(), height.end(), 0);
		//fill(active.begin(), active.end(), false);

		//ResetFlow();

		height[source] = N;

		//if (first) {
		//	fill(height.begin(), height.end(), 1);
		//	height[source] = N;
		//	height[sink]   = 0;
		//	count[1] = N - 2;
		//	count[0] = 1;
		//	count[N] = 1;
		//	first = false;
		//}

		active[source] = active[sink] = true;
		//for (int i = 0; i < N; ++i) {
		//	count[height[i]]++;
		//	//if (excess[i] > 0) Enqueue(i);
		//}

		for (int i = 0; i < G[source].size(); ++i) {
			excess[source] = G[source][i].cap;
			Push(G[source][i]);
		}
		excess[source] = 0;

		//cout << "Before: Active nodes: " << ActiveNodes() << endl;
		//cout << "Before: HSetSum: " << HSetSum() << endl;
		//cout << "Before: Outflow: " << OutFlow(source) << endl;
		//cout << "Before: Inflow: " << InFlow(sink) << endl;
		//double avg = GlobalRelabel(source, sink);

		//for (int i = 0; i < N; ++i) {
		//	if (excess[i] > 0) Enqueue(i);
		//}

		//cout << "Checking excess." << endl;
		//assert(CheckExcess());
		//cout << "Checking capacities vs. flow." << endl;
		//assert(CheckCapacity());
		//cout << "Checking labels." << endl;
		//assert(CheckLabels());
		//cout << "Checking count." << endl;
		//assert(CheckCount());
		//cout << "Everything A OK." << endl;

		//PrintGraph();

		//int p = 1;
		//while (!q.empty()) {

		//highest = -1;
		//for (int i = 255; i >= 0; --i) {
		//	if (hset[i].size() > 0) {
		//		highest = i;
		//		break;
		//	}
		//}

		highest = 1;

		while (highest >= 0 && !hq.empty()) {
			highest = PopHighest();
			//cout << "Popped " << highest;
			//cout << endl << "Len = " << hq.size();
			if (hset[highest].size() == 0) continue;
			//cout << endl;

			//PrintHSet();
			int u = hset[highest].front();
			hset[highest].pop();

			if (hset[highest].size() > 0) {
				PushQ(highest);
			}

			//if (active[u]) {
				active[u] = false;

				//if (height[u] < N) {
					Discharge(u);
				//}
			//}

			//int h = 255;
			//highest = -1;
			//for (int i = h; i >= 0; --i) {
			//	if (hset[i].size() > 0) {
			//		highest = i;
			//		break;
			//	}
			//}
		}

		//CheckActive();
		//cout << "After: Active nodes: " << ActiveNodes() << endl;
		//cout << "After: HSetSum: " << HSetSum() << endl;
		//cout << "After: Outflow: " << OutFlow(source) << endl;
		//cout << "After: Inflow: " << InFlow(sink) << endl;

		for (int i = 0; i < cut.size(); ++i) {
			cut[i] = height[i] >= N;
		}
	}

	bool CheckExcess(void) {
		for (int i = 0; i < excess.size(); ++i) {
			if (excess[i] < 0) {
				return false;
			}
		}

		return true;
	}

	bool CheckCapacity(void) {
		for (int i = 0; i < G.size(); ++i) {
			for (int j = 0; j < G[i].size(); ++j) {
				if (G[i][j].flow > G[i][j].cap) return false;
			}
		}

		return true;
	}

	bool CheckLabels(void) {
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

	int HSetSum(void) {
		int h;

		h = 0;
		for (int i = 0; i < 256; ++i) {
			h += hset[i].size();
		}

		return h;
	}

	bool CheckActive(void) {
		int a;
		int n;
		int h;
		int e;
		int en;

		a = n = e = en = 0;
		for (int i = 0; i < G.size(); ++i) {
			if (active[i]) a++;
			if (height[i] >= N) n++;
			if (excess[i] > 0) e++;
			if (excess[i] > 0 && height[i] < N) en++;
		}

		h = 0;
		for (int i = 0; i < 256; ++i) {
			h += hset[i].size();
		}

		cout << "=============" << endl;
		cout << "Active: " << a << endl;
		cout << "H = N:  " << n << endl;
		cout << "Really active: " << a - n << endl;
		cout << "Excess: " << e << endl;
		cout << "Excess and < N: " << en << endl;
		cout << "hset-sum: " << h << endl;
		cout << "highest: " << highest << endl;
		cout << "at highest: " << hset[highest].size() << endl;
		cout << "=============" << endl;

		return true;
	}

	bool CheckCount(void) {
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

	void MinCutDinic(int source, int sink) {
		cout << "Flow = " << MaxFlowDinic(source, sink) << endl;
		cout << "Starting DFS." << endl;
		fill(cut.begin(), cut.end(), false);
		DFS(source, sink);
		cout << "DFS finished." << endl;
	}

	int BlockingFlow(vector<int> &level, int u, int source, int sink, int limit) {
		if (limit <= 0) return 0;

		if (u == sink) return limit;

		int throughput = 0;
		for (int i = 0; i < G[u].size(); ++i) {
			int res = G[u][i].cap - G[u][i].flow;
			if (level[G[u][i].to] == level[u] + 1 && res > 0) {
				int aug = BlockingFlow(
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

	int MaxFlowDinic(int source, int sink) {
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

			BlockingFlow(level, source, source, sink, 1000000000);
		}
		return OutFlow(source);
	}
};

