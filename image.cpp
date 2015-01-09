#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <iterator>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "image.hpp"
#include "neighborhood.hpp"
#include "selectionrule.hpp"
#include "anisotropy.hpp"

using namespace std;
using namespace cv;

int f(int u, int v, int p) {
	return p == 2 ? (u - v) * (u - v) : abs(u - v);
}

/* Fidelity energy term. */
int Ei(int label, int pix, int u, int p) {
	return (f(label+1, pix, p) - f(label, pix, p)) * u;
}

//void createEdges(FlowGraph& network, Neighborhood& neigh) {
//	/*
//	 * Add sink edges first, so that the first push in discharge
//	 * will go towards the sink. The capacities are set up in
//	 * setupSourceSink.
//	 */
//	for (int i = 0; i < pixels; ++i) {
//		network.addEdge(i, sink, 0);
//	}
//
//	/*
//	 * Create internal edges, which do not depend on the current
//	 * level.
//	 */
//	for (int j = 0; j < rows; ++j) {
//		for (int i = 0; i < cols; ++i) {
//			Neighborhood::iterator it;
//			for (it = neigh.begin(); it != neigh.end(); ++it) {
//
//				int x = i + it->x;
//				int y = j + it->y;
//
//				if (x >= 0 && x < cols && y >= 0 && y < cols)
//					network.addDoubleEdge(
//							j*cols + i,
//							y*cols + x,
//							it->w
//							);
//			}
//		}
//	}
//
//	/*
//	 * Add edges from the source. Capacities are set up in
//	 * setupSourceSink.
//	 */
//	for (int i = 0; i < pixels; ++i) {
//		network.addEdge(source, i, 0);
//	}
//}

void createEdgesAnisotropic(
		FlowGraph& network,
		Neighborhood& neigh,
		int beta,
		const Mat_<Tensor>& tensors
		) {

	int rows = tensors.rows;
	int cols = tensors.cols;
	int pixels = rows * cols;

	/*
	 * Add sink edges first, so that the first push in discharge
	 * will go towards the sink. The capacities are set up in
	 * setupSourceSink.
	 */
	for (int i = 0; i < pixels; ++i) {
		network.addEdge(i, network.getSink(), 0);
	}

	/*
	 * Create internal edges, which do not depend on the current
	 * level.
	 */
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			Neighborhood::iterator it;

			for (it = neigh.begin(); it != neigh.end(); ++it) {

				int x = j + it->x;
				int y = i + it->y;

				if (it->x < 0)
					continue;
				if (it->x == 0 && it->y > 0)
					continue;
				
				if (x >= 0 && x < cols && y >= 0 && y < rows) {

					Mat ee = (Mat_<double>(2, 1) << it->x, it->y);
					Mat M1 = Mat(tensors(i, j));
					Mat M2 = Mat(tensors(y, x));

					Mat M3 = (M1 + M2) / 2.0;

					double w = beta
						* norm(ee) * norm(ee)
						* determinant(M3)
						* it->dt
						/ pow(ee.dot(M3 * ee), 3.0 / 2.0);

					network.addDoubleEdge(
							i*cols + j,
							y*cols + x,
							w
							);
				}
			}
		}
	}

	/*
	 * Add edges from the source. Capacities are set up in
	 * setupSourceSink.
	 */
	for (int i = 0; i < pixels; ++i) {
		network.addEdge(network.getSource(), i, 0);
	}
}

/*
 * Change the capacities of the edges connecting the source
 * and the sink to the rest of the network, as these edges
 * are dependent on the current level.
 */
void setupSourceSink(FlowGraph& network, Mat& in, int alpha, int label, int p) {

	std::vector<int> s_caps(in.rows * in.cols);
	std::vector<int> t_caps(in.rows * in.cols);

	//fill(s_caps.begin(), s_caps.end(), 0);
	//fill(t_caps.begin(), t_caps.end(), 0);

	for (int j = 0; j < in.rows; ++j) {
		for (int i = 0; i < in.cols; ++i) {
			int e1 = Ei(label, in.at<uchar>(j, i), 1, p);
			int e1init = Ei(255, in.at<uchar>(j, i), 1, p);

			//if (0 < e1) {
			//	s_caps[j*in.cols + i] += e1 - 0;
			//}
			//else {
			//	t_caps[j*in.cols + i] += 0 - e1;
			//}
			s_caps[j*in.cols + i] += max(e1init, 0);
			t_caps[j*in.cols + i] += max(-e1init, 0) - (e1 - e1init);
		}
	}

	for (size_t i = 0; i < s_caps.size(); ++i) {
		network.changeCapacity(network.getSource(), i, alpha * s_caps[i]);
	}

	for (size_t i = 0; i < t_caps.size(); ++i) {
		network.changeCapacity(i, network.getSink(), alpha * t_caps[i]);
	}
}

void setupSource(FlowGraph& network, Mat& in, int alpha, int label, int p) {
	std::vector<int> s_caps(in.rows * in.cols);

	for (int j = 0; j < in.rows; ++j) {
		for (int i = 0; i < in.cols; ++i) {
			int e1 = Ei(label, in.at<uchar>(j, i), 1, p);
			int e1init = Ei(255, in.at<uchar>(j, i), 1, p);

			s_caps[j*in.cols + i] += max(e1init, 0);
		}
	}

	for (size_t i = 0; i < s_caps.size(); ++i) {
		network.changeCapacity(network.getSource(), i, alpha * s_caps[i]);
	}
}

void setupSink(FlowGraph& network, Mat& in, int alpha, int label, int p) {
	std::vector<int> t_caps(in.rows * in.cols);

	for (int j = 0; j < in.rows; ++j) {
		for (int i = 0; i < in.cols; ++i) {
			int e1 = Ei(label, in.at<uchar>(j, i), 1, p);
			int e1init = Ei(255, in.at<uchar>(j, i), 1, p);

			t_caps[j*in.cols + i] += max(-e1init, 0) - (e1 - e1init);
		}
	}

	for (size_t i = 0; i < t_caps.size(); ++i) {
		if (t_caps[i] != 0)
			network.changeCapacity(i, network.getSink(), alpha * t_caps[i]);
	}
}

//void restore(int alpha, int p) {
//
//	createEdges();
//
//	for (int label = 255; label >= 0; --label) {
//		cout << "Label: " << label << endl;
//
//		setupSourceSink(alpha, label, p);
//		network.minCutPushRelabel(source, sink);
//
//		/* Use the cut to update the output image. */
//		for (int j = 0; j < rows; ++j) {
//			for (int i = 0; i < cols; ++i) {
//				if (!network.cut[j*cols + i])
//					out->at<uchar>(j, i) = label;
//			}
//		}
//	}
//}

void restoreAnisotropicTV(
		Mat& in,
		Mat& out,
		Mat_<Tensor>& tensors,
		Neighborhood& neigh,
		int alpha, int beta, int p
		) {

	int rows = in.rows;
	int cols = in.cols;
	int pixels = rows * cols;
	int source = pixels;
	int sink   = pixels + 1;

	FIFORule frule(pixels + 2);
	SelectionRule& rule = frule;
	FlowGraph network(rows * cols + 2, source, sink, rule);

	createEdgesAnisotropic(network, neigh, beta, tensors);

	setupSource(network, in, alpha, 255, p);
	for (int label = 255; label >= 0; --label) {
		cout << "Label: " << label << endl;

		//setupSourceSink(network, in, alpha, label, p);
		setupSink(network, in, alpha, label, p);
		//network.minCutPushRelabel(source, sink);
		network.minCutBK(source, sink);

		/* Use the cut to update the output image. */
		for (int j = 0; j < rows; ++j) {
			for (int i = 0; i < cols; ++i) {
				if (!network.cut[j*cols + i])
					out.at<uchar>(j, i) = label;
			}
		}
	}
}

