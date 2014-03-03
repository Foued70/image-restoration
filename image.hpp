#pragma once

class Image {
private:
	Mat *in, *out;
	int rows, cols;
	int pixels;
	int source, sink;

	std::vector<int> s_index(pixels);
	std::vector<int> t_index(pixels);

	FlowGraph network;
	Neighborhood& neigh;
	SelectionRule& rule;
	void setupInternal(int beta);
	void setupSourceSink(int beta, int label);

public:
	Image(Mat *in, Mat *out, SelectionRule& rule, Neighborhood& neigh) :
		network(in->rows * in->cols + 2, rule),
		in(in),
		out(out),
		rule(rule),
		neigh(neigh),
		rows(in->rows),
		cols(in->cols),
		pixels(rows * cols),
		source(pixels),
		sink(pixels + 1) {

		
		for (int i = 0; i < pixels; ++i) {
			t_index[i] = network.addEdge(i, sink, 0);
		}
		for (int i = 0; i < pixels; ++i) {
			s_index[i] = network.addEdge(source, i, 0);
		}

	}

	void segment(int beta, int label);
	void restore(int beta);
};

