#pragma once

#include <vector>
#include <set>
#include <opencv2/opencv.hpp>
#include "graph.hpp"
#include "selectionrule.hpp"
#include "neighborhood.hpp"

class Image {
private:
	cv::Mat *in, *out;
	int rows, cols;
	int pixels;
	int source, sink;

	std::vector<int> s_index;
	std::vector<int> t_index;

	std::vector<int> s_caps;
	std::vector<int> t_caps;

	FlowGraph network;

	/* These would preferrably be pointers, not references */
	Neighborhood& neigh;
	SelectionRule& rule;

	void createEdges();
	void setupSourceSink(int alpha, int label, int p);

public:
	Image(cv::Mat *in, cv::Mat *out, SelectionRule& rule, Neighborhood& neigh) :
		network(in->rows * in->cols + 2, rule),
		in(in),
		out(out),
		rule(rule),
		neigh(neigh),
		rows(in->rows),
		cols(in->cols),
		pixels(rows * cols),
		source(pixels),
		s_index(pixels),
		t_index(pixels),
		s_caps(pixels),
		t_caps(pixels),
		sink(pixels + 1) {}

	void restore(int alpha, int p);
};

