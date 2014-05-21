#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	if (argc < 6) {
		cout << "Wrong usage" << endl;
		return -1;
	}

	Mat image1;
	image1 = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	Mat image2;
	image2 = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

	int center = atoi(argv[4]);
	double scale = atof(argv[5]);

	assert(image1.rows = image2.cols);
	assert(image1.rows = image2.rows);

	Mat out = image2.clone();

	int maxi = 0;
	int mini = 255;
	bool range = false;

	for (int j = 0; j < image1.rows; ++j) {
		for (int i = 0; i < image1.cols; ++i) {
			int p1 = image1.at<uchar>(j, i);
			int p2 = image2.at<uchar>(j, i);

			int o = center + round(double(p1 - p2) * scale);

			if (o > maxi) maxi = o;
			if (o < mini) mini = o;

			if (o < 0) {
				range = true;
				o = 0;
			}
			if (o > 255) {
				range = true;
				o = 255;
			}

			out.at<uchar>(j, i) = o;
		}
	}

	if (range) cout << "Wrong range!" << endl;

	cout << "Minimum: " << mini << endl;
	cout << "Maximum: " << maxi << endl;

	imwrite(argv[3], out);
}

