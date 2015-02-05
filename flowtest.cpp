#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "graph.hpp"
#include "selectionrule.hpp"
#include "image.hpp"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	int nodes = 2 + 2;
	FIFORule frule(nodes);
	SelectionRule& rule = frule;

	int source = 0;
	int sink   = 3;

	FlowGraph network(nodes, source, sink, rule);
	network.addEdge(0, 1, 10);
	network.addEdge(0, 2, 10);
	network.addEdge(1, 2, 1);
	network.addEdge(1, 3, 9);
	network.addEdge(2, 3, 1);

	network.minCutBK(source, sink);

	for (int i = 0; i < nodes; ++i) {
		cout << i << ": " << int(network.cut[i]) << endl;
	}
}
