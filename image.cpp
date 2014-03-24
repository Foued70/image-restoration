#include <iostream>
#include <algorithm>
#include <set>
#include <iterator>
#include <opencv2/opencv.hpp>
#include "image.hpp"

using namespace std;
using namespace cv;

int f(int u, int v) {
	return (u - v) * (u - v);
}

int Ei(int label, int pix, int u) {
	return (f(label+1, pix) - f(label, pix)) * (1 - u);
}

int Eij(int b, int up, int uq) {
	return b * ((1 - 2 * uq) * up + uq);
}

void Image::createEdges(int beta) {
	int A = Eij(beta, 0, 0);
	int B = Eij(beta, 0, 1);
	int C = Eij(beta, 1, 0);
	int D = Eij(beta, 1, 1);

	cout << "A = " << A << endl;
	cout << "B = " << B << endl;
	cout << "C = " << C << endl;
	cout << "D = " << D << endl;

	cout << "Image is " << cols << "x" << rows << endl;
	cout << "Which gives " << pixels << " pixels." << endl;

	for (int j = 0; j < rows; ++j) {
		for (int i = 0; i < cols; ++i) {
			if (i + 1 < cols)
				network.addEdge(j*cols + i, j*cols + i + 1, B+C-A-D);
			if (j + 1 < rows)
				network.addEdge(j*cols + i, (j+1)*cols + i, B+C-A-D);
		}
	}

	for (int i = 0; i < pixels; ++i) {
		t_index[i] = network.addEdge(i, sink, 0);
	}
	for (int i = 0; i < pixels; ++i) {
		s_index[i] = network.addEdge(source, i, 0);
	}
}

void Image::setupSourceSink(int beta, int label) {
	int A = Eij(beta, 0, 0);
	int B = Eij(beta, 0, 1);
	int C = Eij(beta, 1, 0);
	int D = Eij(beta, 1, 1);

	fill(s_caps.begin(), s_caps.end(), 0);
	fill(t_caps.begin(), t_caps.end(), 0);

	for (int j = 0; j < rows; ++j) {
		for (int i = 0; i < cols; ++i) {
			int e0 = Ei(label, in->at<uchar>(j, i), 0);
			int e1 = Ei(label, in->at<uchar>(j, i), 1);

			if (e0 < e1) {
				s_caps[j*cols + i] += e1 - e0;
			}
			else {
				t_caps[j*cols + i] += e0 - e1;
			}

			if (i + 1 < cols) {
				s_caps[j*cols + i] += C - A;
				t_caps[j*cols + i + 1] += C - D;
			}

			if (j + 1 < rows) {
				s_caps[j*cols + i] += C - A;
				t_caps[(j+1)*cols + i] += C - D;
			}
		}
	}

	for (int i = 0; i < s_caps.size(); ++i) {
		network.changeCapacity(source, s_index[i], s_caps[i]);
	}

	for (int i = 0; i < t_caps.size(); ++i) {
		network.changeCapacity(i, t_index[i], t_caps[i]);
	}
}

void Image::setupSourceSink(int beta, int label, set<int> nodes) {
	int A = Eij(beta, 0, 0);
	int B = Eij(beta, 0, 1);
	int C = Eij(beta, 1, 0);
	int D = Eij(beta, 1, 1);

	fill(s_caps.begin(), s_caps.end(), 0);
	fill(t_caps.begin(), t_caps.end(), 0);

	//for (int j = 0; j < rows; ++j) {
	//	for (int i = 0; i < cols; ++i) {

	for (set<int>::const_iterator it = nodes.begin();
			it != nodes.end();
			++it) {
		int j = *it / cols;
		int i = *it % cols;

		int e0 = Ei(label, in->at<uchar>(j, i), 0);
		int e1 = Ei(label, in->at<uchar>(j, i), 1);

		if (e0 < e1) {
			s_caps[j*cols + i] += e1 - e0;
		}
		else {
			t_caps[j*cols + i] += e0 - e1;
		}

		if (i + 1 < cols) {
			s_caps[j*cols + i] += C - A;
			t_caps[j*cols + i + 1] += C - D;
		}

		if (j + 1 < rows) {
			s_caps[j*cols + i] += C - A;
			t_caps[(j+1)*cols + i] += C - D;
		}
	}

	for (set<int>::const_iterator it = nodes.begin();
			it != nodes.end();
			++it) {
		network.changeCapacity(source, s_index[*it], s_caps[*it]);
		network.changeCapacity(*it, t_index[*it], t_caps[*it]);
	}
}

void Image::restore(int beta) {
	createEdges(beta);

	set<int> all;
	for (int i = 0; i < pixels; ++i) {
		all.insert(i);
	}
	for (int label = 255; label >= 0; --label) {
		cout << "Label: " << label << endl;

		//vector<char> lower = segment(beta, label, all);
		setupSourceSink(beta, label);
		network.minCutPushRelabel(source, sink);

		//cout << "lower.size() = " << lower.size() << endl;
		//for (set<int>::const_iterator it = lower.begin();
		//		it != lower.end();
		//		++it) {
		//	out->at<uchar>(*it / cols, *it % cols) = label;
		//}
		for (int j = 0; j < rows; ++j) {
			for (int i = 0; i < cols; ++i) {
				if (!network.cut[j*cols + i])
					out->at<uchar>(j, i) = label;
			}
		}
	}
}

vector<char> Image::segment(int beta, int label, vector<char>& active) {
	//setupSourceSink(beta, label, active);
	// SLOW
	setupSourceSink(beta, label);
	// SLOW Should make use of active
	network.minCutPushRelabel(source, sink);

	vector<char> lower(pixels);
	for (int i = 0; i < active.size(); ++i) {
		if (!network.cut[i] && active[i]) {
			lower[i] = 1;
		}
	}

	return lower;
}

void Image::restoreBisect(int beta) {
	createEdges(beta);

	vector<char> all(pixels);
	fill(all.begin(), all.end(), 1);

	restorePart(beta, -1, 255, all);
}

void Image::restorePart(int beta, int lo, int hi, vector<char>& active) {
	cout << "Restoring between " << lo << " and " << hi << endl;
	/*
	 * Kjører segmentering et sted mellom lo og hi. Alle piksler som
	 * er >= N vil få cut = true. Disse er da med på source-siden av
	 * kuttet. Alle de andre setter vi = label. Flere og flere
	 * havner >= N. Når du havner >= N sitter du igjen med forrige
	 * (!) label. Dvs at alle på source-siden har label > current
	 * (de kommer aldri til å komme seg ut av source-siden), og alle
	 * på sink-siden har label <= current. Labelen går vanligvis
	 * nedover.
	 *
	 * Alle >= N: har label > current
	 * Alle andre: har label <= current
	 *
	 */

	if (lo + 1 == hi) {
		cout << "Found label: " << hi << endl;
		for (int i = 0; i < active.size(); ++i) {
			if (active[i]) {
				out->at<uchar>(i / cols, i % cols) = hi;
			}
		}
		return;
	}

	/*
	 * Del bildet i to her, på label = mid?
	 * Noen er lo < p <= mid.
	 * Andre er mid < p <= hi.
	 *
	 * Burde derfor alltid være større en lo (starte lo = -1).
	 * Burde alltid være <= hi.
	 *
	 * Så hvis lo == mid + 1 har vi funnet vår label.
	 */

	/*
	 * Eksempel:
	 * lo = 1 (minste mulige er 2)
	 * hi = 4 (høyeste mulige er 4)
	 * mid = 1 + 1 = 2
	 */

	int mid = lo + (hi - lo) / 2;
	cout << lo << " ----(" << mid << ")---- " << hi << endl;

	network.setSegment(&active);
	vector<char> lower = segment(beta, mid, active);
	vector<char> higher(pixels);

	// SLOW
	int l = 0;
	int h = 0;
	for (int i = 0; i < active.size(); ++i) {
		if (active[i] && !lower[i]) {
			higher[i] = 1;
			h++;
		}
		if (active[i] && lower[i]) l++;
	}

	cout << "Flow into active: " << network.inFlow(active) << endl;

	/*
	 * For these guys we know that the label will be lower, and that
	 * the ChangeCapacity approach will work
	 */

	if (mid >= lo + 1 && l > 0)
		restorePart(beta, lo, mid, lower);

	/*
	 * Here we should reset the flow in the relevant section (as
	 * well as out of the section?), so every edge touching a node
	 * in the set. Will this affect further calculations? No, since
	 * nodes outside this set were on the other side of the cut and
	 * have therefore already been treated. This will also reset the
	 * height of the nodes, and their activeness!
	 */
	// SLOW
	network.resetFlow();
	network.resetHeights();

	if (hi >= mid + 1 && h > 0)
		restorePart(beta, mid, hi, higher);
}

