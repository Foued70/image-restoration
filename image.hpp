#pragma once

#include <vector>
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
	Neighborhood& neigh;
	SelectionRule& rule;

	void createEdges(int beta);
	void setupSourceSink(int beta, int label);

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

	void segment(int beta, int label);
	void restore(int beta);
};

