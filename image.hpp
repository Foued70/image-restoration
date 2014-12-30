#pragma once

#include <vector>
#include <set>
#include <opencv2/opencv.hpp>
#include "graph.hpp"
#include "selectionrule.hpp"
#include "neighborhood.hpp"
#include "anisotropy.hpp"

//void createEdges();
void createEdgesAnisotropic(
		FlowGraph& network,
		Neighborhood& neigh,
		int beta,
		const cv::Mat_<Tensor>& tensors
		);
void setupSourceSink(FlowGraph& network, cv::Mat& in, int alpha, int label, int p);

//void restore(int alpha, int p);

void restoreAnisotropicTV(
		cv::Mat& in,
		cv::Mat& out,
		cv::Mat_<Tensor>& tensors,
		Neighborhood& neigh,
		int alpha, int beta, int p
		);

