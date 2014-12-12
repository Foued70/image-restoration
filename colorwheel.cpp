#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
	Size size(512, 512);

	Mat h = Mat::ones(size, CV_64F);
	Mat s = Mat::ones(size, CV_64F);
	Mat v = Mat::ones(size, CV_64F);
	Mat hsv;
	vector<Mat> channels;

	Point center(size.height / 2, size.width / 2);
	for (int i = 0; i < size.height; ++i) {
		for (int j = 0; j < size.width; ++j) {
			int x = j - center.x;
			int y = i - center.y;

			if (sqrt(x*x + y*y) > size.width / 2.0) {
				h.at<double>(i, j) = 0;
				s.at<double>(i, j) = 0;
				v.at<double>(i, j) = size.width / sqrt(4.0);
				continue;
			}

			h.at<double>(i, j) = fmod(atan2(y, x) * 180.0 / M_PI + 180.0, 180.0);
			s.at<double>(i, j) = 255;
			v.at<double>(i, j) = sqrt(y*y + x*x);
		}
	}

	Mat ho, so, vo;
	normalize(h, ho, 0, 180, NORM_MINMAX, CV_8U);
	normalize(s, so, 0, 255, NORM_MINMAX, CV_8U);
	normalize(v, vo, 0, 255, NORM_MINMAX, CV_8U);

	channels.push_back(ho);
	channels.push_back(so);
	channels.push_back(vo);
	merge(channels, hsv);

	Mat colortensor;
	cvtColor(hsv, colortensor, CV_HSV2BGR);

	imwrite("wheel.png", colortensor);

	return 0;
}

