#include <iostream>
#include "image.hpp"

using namespace std;

void Image::setupInternal(int beta) {
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

}
void Image::setupSourceSink(int beta, int label) {
	for (int i = 0; i < pixels; ++i) {
		t_index[i] = network.addEdge(i, sink, 0);
	}
	for (int i = 0; i < pixels; ++i) {
		s_index[i] = network.addEdge(source, i, 0);
	}
}


for (int i = 0; i < s_caps.size(); ++i) {
	if (!network.cut[i]) {
		network.changeCapacity(source, s_index[i], s_caps[i]);
	}
}

for (int i = 0; i < t_caps.size(); ++i) {
	if (!network.cut[i]) {
		network.changeCapacity(i, t_index[i], t_caps[i]);
	}
}


void Image::segment(int beta, int label);
void Image::restore(int beta) {
	for (int label = 255; label >= 0; --label) {
		cout << "Label: " << label << endl;
		vector<int> s_caps(pixels);
		vector<int> t_caps(pixels);

		//cout << "Calculating source and sink edge capacities." << endl;
		for (int j = 0; j < rows; ++j) {
			for (int i = 0; i < cols; ++i) {
				int e0 = Ei(label, image.at<uchar>(j, i), 0);
				int e1 = Ei(label, image.at<uchar>(j, i), 1);

				if (e0 < e1) {
					s_caps[j*cols + i] += e1 - e0;
				}
				else {
					t_caps[j*cols + i] += e0 - e1;
				}

				if (i + 1 < cols) {
					//if (C - A > 0) {
						s_caps[j*cols + i] += C - A;
					//}
					//else {
					//	t_caps[j*cols + i] += A - C;
					//}

					//if (D - C > 0) {
					//	s_caps[j*cols + i + 1] += D - C;
					//}
					//else {
						t_caps[j*cols + i + 1] += C - D;
					//}
				}

				if (j + 1 < rows) {
					//if (C - A > 0) {
						s_caps[j*cols + i] += C - A;
					//}
					//else {
					//	t_caps[j*cols + i] += A - C;
					//}

					//if (D - C > 0) {
					//	s_caps[(j+1)*cols + i] += D - C;
					//}
					//else {
						t_caps[(j+1)*cols + i] += C - D;
					//}
				}
			}
		}

		for (int i = 0; i < s_caps.size(); ++i) {
			if (!network.cut[i]) {
				network.changeCapacity(source, s_index[i], s_caps[i]);
			}
		}

		for (int i = 0; i < t_caps.size(); ++i) {
			if (!network.cut[i]) {
				network.changeCapacity(i, t_index[i], t_caps[i]);
			}
		}

		network.minCutPushRelabel(source, sink);

		for (int j = 0; j < rows; ++j) {
			for (int i = 0; i < cols; ++i) {
				if (!network.cut[j*cols + i])
					out.at<uchar>(j, i) = label;
			}
		}
	}

	cout << endl;
}
