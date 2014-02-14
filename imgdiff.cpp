#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	if (argc < 3) {
		cout << "Wrong usage" << endl;
		return -1;
	}

	Mat image1;
	image1 = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	Mat image2;
	image2 = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

	assert(image1.rows = image2.cols);
	assert(image1.rows = image2.rows);

	for (int j = 0; j < image1.rows; ++j) {
		for (int i = 0; i < image1.cols; ++i) {
			int p1 = image1.at<uchar>(j, i);
			int p2 = image2.at<uchar>(j, i);
			if (p1 != p2) {
				cout << "(" << j << ", " << i << "): ";
				cout << p1 << " != " << p2 << endl;
			}
		}
	}
}

