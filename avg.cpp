#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	if (argc < 2) {
		cout << "Wrong usage" << endl;
		return -1;
	}

	Mat image;
	image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	int sum = 0;

	for (int j = 0; j < image.rows; ++j) {
		for (int i = 0; i < image.cols; ++i) {
			sum += image.at<uchar>(j, i);
		}
	}

	cout << static_cast<double>(sum) / (image.rows * image.cols) << endl;
}

