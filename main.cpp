#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "graph.hpp"

using namespace std;
using namespace cv;

bool test1(void);
bool test2(void);

int f(int u, int v) {
	return abs(u - v) * abs(u - v);
}

int Ei(int label, int pix, int u) {
	return (f(label+1, pix) - f(label, pix)) * (1 - u);
}

int Eij(int up, int uq) {
	return 50 * ((1 - 2 * uq) * up + uq);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		cout << "Wrong usage" << endl;
		return -1;
	}

	Mat image;
	image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	if (!image.data) {
		cout << "Loading image failed" << endl;
		return -1;
	}

	test1();
	cout << endl;
	test2();
	cout << endl;
	int rows = image.rows;
	int cols = image.cols;
	int pixels = rows * cols;

	Mat out = image.clone();
	for (int j = 0; j < rows; ++j) {
		for (int i = 0; i < cols; ++i) {
				out.at<uchar>(j, i) = 255;
		}
	}

	for (int label = 255; label > 0; --label) {
		cout << "LABEL: " << label << endl;
		int A = Eij(0, 0);
		int B = Eij(0, 1);
		int C = Eij(1, 0);
		int D = Eij(1, 1);

		PushRelabel network(pixels + 2);

		int source = pixels;
		int sink   = pixels + 1;

		cout << "Image is " << cols << "x" << rows << endl;
		cout << "Which gives " << pixels << " pixels." << endl;
		vector<int> s_caps(pixels);
		vector<int> t_caps(pixels);

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
					network.AddEdge(j*cols + i, j*cols + i + 1, B+C-A-D);
					if (C - A > 0) {
						s_caps[j*cols + i] += C - A;
					}
					else {
						t_caps[j*cols + i] += A - C;
					}

					if (C - D > 0) {
						s_caps[j*cols + i] += C - D;
					}
					else {
						t_caps[j*cols + i] += D - C;
					}
				}

				if (j + 1 < rows) {
					network.AddEdge(j*cols + i, (j+1)*cols + i, B+C-A-D);
					if (C - A > 0) {
						s_caps[j*cols + i] += C - A;
					}
					else {
						t_caps[j*cols + i] += A - C;
					}

					if (C - D > 0) {
						s_caps[j*cols + i] += C - D;
					}
					else {
						t_caps[j*cols + i] += D - C;
					}
				}
			}
		}

		for (int i = 0; i < s_caps.size(); ++i) {
			network.AddEdge(source, i, s_caps[i]);
		}

		for (int i = 0; i < t_caps.size(); ++i) {
			network.AddEdge(i, sink, t_caps[i]);
		}

		cout << "Source out capacity: " << network.OutCap(source) << endl;
		cout << "Sink in capacity: " << network.InCap(sink) << endl;
		cout << "Starting min cut algorithm" << endl;
		network.MinCutPushRelabel(source, sink);
		cout << "Active nodes: " << network.ActiveNodes() << endl;

		for (int j = 0; j < rows; ++j) {
			for (int i = 0; i < cols; ++i) {
				if (!network.cut[j*cols + i])
					out.at<uchar>(j, i) = label;
			}
		}
	}

	//namedWindow("Display image", CV_WINDOW_AUTOSIZE);
	//imshow("Display image", out);
	//waitKey(0);
	imwrite(argv[2], out);

	return 0;
}

bool test1(void)
{
	PushRelabel network(4);
	network.AddEdge(0, 1, 10);
	network.AddEdge(0, 2, 10);
	network.AddEdge(1, 2, 1);
	network.AddEdge(1, 3, 9);
	network.AddEdge(2, 3, 9);

	int flow = network.MaxFlowDinic(0, 3);
	
	cout << "test1: flow: " << flow << endl;

	return flow == 18;
}

bool test2(void)
{
	PushRelabel network(4);
	network.AddEdge(0, 1, 10);
	network.AddEdge(0, 2, 10);
	network.AddEdge(1, 2, 1);
	network.AddEdge(1, 3, 9);
	network.AddEdge(2, 3, 9);

	cout << "Running push-relabel" << endl;
	network.MinCutPushRelabel(0, 3);
	
	for (int i = 0; i < network.cut.size(); ++i) {
		cout << network.cut[i];
	}

	return true;
}
