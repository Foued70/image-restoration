#include <iostream>
#include <algorithm>
#include <set>
#include <iterator>
#include <opencv2/opencv.hpp>
#include "image.hpp"
#include "neighborhood.hpp"

using namespace std;
using namespace cv;

int f(int u, int v, int p) {
	return p == 2 ? (u - v) * (u - v) : abs(u - v);
}

int Ei(int label, int pix, int u, int p) {
	return (f(label+1, pix, p) - f(label, pix, p)) * (1 - u);
}

int Eij(int up, int uq) {
	return (1 - 2 * uq) * up + uq;
}

void Image::createEdges() {
	int A = Eij(0, 0);
	int B = Eij(0, 1);
	int C = Eij(1, 0);
	int D = Eij(1, 1);

	cout << "A = " << A << endl;
	cout << "B = " << B << endl;
	cout << "C = " << C << endl;
	cout << "D = " << D << endl;

	/*
	 * Add sink edges first, so that the first push in discharge
	 * will go towards the sink.
	 */
	for (int i = 0; i < pixels; ++i) {
		t_index[i] = network.addEdge(i, sink, 0);
	}

	for (int j = 0; j < rows; ++j) {
		for (int i = 0; i < cols; ++i) {
			Neighborhood::const_iterator it;
			for (it = neigh.begin(); it != neigh.end(); ++it) {

				int x = i + it->x;
				int y = j + it->y;

				if (x >= 0 && x < cols && y >= 0 && y < cols)
					network.addDoubleEdge(
							j*cols + i,
							y*cols + x,
							it->w * B
							);
			}
		}
	}

	for (int i = 0; i < pixels; ++i) {
		s_index[i] = network.addEdge(source, i, 0);
	}
}

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
		//cout << "Label: " << label << endl;

		setupSourceSink(alpha, label, p);
#ifdef DINIC
		network.resetFlow();
		network.minCutDinic(source, sink);
#else
		network.minCutPushRelabel(source, sink);
#endif

		for (int j = 0; j < rows; ++j) {
			for (int i = 0; i < cols; ++i) {
				if (!network.cut[j*cols + i])
					out->at<uchar>(j, i) = label;
			}
		}
	}
}

