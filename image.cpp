#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <iterator>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "image.hpp"
#include "neighborhood.hpp"
#include "sobel.hpp"

using namespace std;
using namespace cv;

int f(int u, int v, int p) {
	return p == 2 ? (u - v) * (u - v) : abs(u - v);
}

/* Fidelity energy term. */
int Ei(int label, int pix, int u, int p) {
	return (f(label+1, pix, p) - f(label, pix, p)) * (1 - u);
}

void Image::createEdges() {
	/*
	 * Add sink edges first, so that the first push in discharge
	 * will go towards the sink. The capacities are set up in
	 * setupSourceSink.
	 */
	for (int i = 0; i < pixels; ++i) {
		t_index[i] = network.addEdge(i, sink, 0);
	}

	/*
	 * Create internal edges, which do not depend on the current
	 * level.
	 */
	for (int j = 0; j < rows; ++j) {
		for (int i = 0; i < cols; ++i) {
			Neighborhood::iterator it;
			for (it = neigh.begin(); it != neigh.end(); ++it) {

				int x = i + it->x;
				int y = j + it->y;

				if (x >= 0 && x < cols && y >= 0 && y < cols)
					network.addDoubleEdge(
							j*cols + i,
							y*cols + x,
							it->w
							);
			}
		}
	}

	/*
	 * Add edges from the source. Capacities are set up in
	 * setupSourceSink.
	 */
	for (int i = 0; i < pixels; ++i) {
		s_index[i] = network.addEdge(source, i, 0);
	}
}

void Image::createEdgesAnisotropic(int beta, Mat_<Tensor>& tensors) {
	/*
	 * Add sink edges first, so that the first push in discharge
	 * will go towards the sink. The capacities are set up in
	 * setupSourceSink.
	 */
	for (int i = 0; i < pixels; ++i) {
		t_index[i] = network.addEdge(i, sink, 0);
	}

	ofstream wfile;
	wfile.open("weights", ios::out | ios::trunc);

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
				
				Mat ee = (Mat_<double>(2, 1) << it->x, it->y);
				Mat M  = Mat(tensors(i,j));

				double w = beta
					* norm(ee) * norm(ee)
					* determinant(M)
					* it->dt
					/ pow(ee.dot(M * ee), 3.0 / 2.0);

				wfile << w << endl;
				if (x >= 0 && x < cols && y >= 0 && y < rows)
					network.addDoubleEdge(
							i*cols + j,
							y*cols + x,
							w
							);
			}
		}
	}
	wfile.close();

	/*
	 * Add edges from the source. Capacities are set up in
	 * setupSourceSink.
	 */
	for (int i = 0; i < pixels; ++i) {
		s_index[i] = network.addEdge(source, i, 0);
	}
}

/*
 * Change the capacities of the edges connecting the source
 * and the sink to the rest of the network, as these edges
 * are dependent on the current level.
 */
void Image::setupSourceSink(int alpha, int label, int p) {

	fill(s_caps.begin(), s_caps.end(), 0);
	fill(t_caps.begin(), t_caps.end(), 0);

	for (int j = 0; j < rows; ++j) {
		for (int i = 0; i < cols; ++i) {
			int e0 = Ei(label, in->at<uchar>(j, i), 0, p);
			int e1 = Ei(label, in->at<uchar>(j, i), 1, p);

			if (e0 < e1) {
				s_caps[j*cols + i] += e1 - e0;
			}
			else {
				t_caps[j*cols + i] += e0 - e1;
			}
		}
	}

	for (int i = 0; i < s_caps.size(); ++i) {
		network.changeCapacity(source, s_index[i], alpha * s_caps[i]);
	}

	for (int i = 0; i < t_caps.size(); ++i) {
		network.changeCapacity(i, t_index[i], alpha * t_caps[i]);
	}
}

void Image::restore(int alpha, int p) {

	createEdges();

	for (int label = 255; label >= 0; --label) {
		cout << "Label: " << label << endl;

		setupSourceSink(alpha, label, p);
		network.minCutPushRelabel(source, sink);

		/* Use the cut to update the output image. */
		for (int j = 0; j < rows; ++j) {
			for (int i = 0; i < cols; ++i) {
				if (!network.cut[j*cols + i])
					out->at<uchar>(j, i) = label;
			}
		}
	}
}

void Image::restoreAnisotropicTV(int alpha, int beta, int p, Mat_<Tensor>& tensors) {

	createEdgesAnisotropic(beta, tensors);

	for (int label = 255; label >= 0; --label) {
		cout << "Label: " << label << endl;

		setupSourceSink(alpha, label, p);
		network.minCutPushRelabel(source, sink);

		/* Use the cut to update the output image. */
		for (int j = 0; j < rows; ++j) {
			for (int i = 0; i < cols; ++i) {
				if (!network.cut[j*cols + i])
					out->at<uchar>(j, i) = label;
			}
		}
	}
}

