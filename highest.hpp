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
	vector<char> active;
	vector<queue<int> > hset;
	queue<int> q;
	int highest;

public:
	vector<char> cut;

	PushRelabel(int N) :
		N(N),
		G(N),
		excess(N),
		height(N),
		active(N),
		cut(N),
		count(N+1),
       		hset(N+1),
       		highest(0) {}

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
	}

	void SetValue(int i, int v) {
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

			int start = highest;
			highest = -1;
			for (int i = start; i >= 0; --i) {
				if (hset[i].size() > 0) {
					highest = i;
					break;
				}
			}
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
		hset[height[i]].push(i);
		if (highest < height[i]) highest = height[i];
	}

	void PrintHSet(void) {
		for (int i = 0; i <= N; ++i) {
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
			count[N]++;
		}

		//for (int i = h; i < N; ++i) {
		//	if (hset[i].size() > 0) {
		//		hset[i] = queue<int>();
		//	}
		//}

		highest = -1;
		for (int i = h-1; i >= 0; --i) {
			if (hset[i].size() > 0) {
				highest = i;
				break;
			}
		}
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
		else {
			int start = highest;
			highest = -1;
			for (int i = start; i >= 0; --i) {
				if (hset[i].size() > 0) {
					highest = i;
					break;
				}
			}
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

	double GlobalRelabel(int source, int sink) {
		queue<int> nq;
		queue<int> hq;
		vector<bool> visited(N);
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
				if (active[from]) continue;
				if (height[from] > h) continue;

				if (G[from][index].cap > G[from][index].flow) {
					count[height[from]]--;
					height[from] = h;
					count[h]++;
					c++;
					s += h;
					visited[from] = true;
					Enqueue(from);
					nq.push(from);
					hq.push(h);
				}
			}
		}
		cout << "Relabeled: " << c << endl;
		cout << "Avg: " << static_cast<double>(s) / c << endl;

		return static_cast<double>(s) / c;
	}

	int ActiveNodes(void) {
		int c = 0;
		for (int i = 0; i < excess.size(); ++i) {
			if (excess[i] > 0) c++;
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

	void MinCutPushRelabel(int source, int sink) {

		height[source] = N;

		active[source] = active[sink] = true;

		for (int i = 0; i < G[source].size(); ++i) {
			if (cut[G[source][i].to]) continue;
			excess[source] = G[source][i].cap;
			Push(G[source][i]);
		}
		excess[source] = 0;

		while (highest >= 0) {
			int u = hset[highest].front();
			hset[highest].pop();

			active[u] = false;
			Discharge(u);
		}

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

