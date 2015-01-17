#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "neighborhood.hpp"

#define PADDING 20

using namespace std;
using namespace cv;

double per(Neighborhood &neigh, double radius) {
	double p = 0.0;

	int center = radius + PADDING;
	for (int i = 0; i < 2 * center; ++i) {
		for (int j = 0; j < 2 * center; ++j) {
			Neighborhood::iterator it;
			for (it = neigh.begin(); it != neigh.end(); ++it) {
				Mat ee = (Mat_<double>(2, 1) << it->x, it->y);

				int x = j + it->x;
				int y = i + it->y;

				if (it->x < 0)
					continue;
				if (it->x == 0 && it->y > 0)
					continue;

				int rx = x - center;
				int ry = y - center;
				double rs = sqrt(rx * rx + ry * ry);
				rx = j - center;
				ry = i - center;
				double rt = sqrt(rx * rx + ry * ry);

				if (rs < radius && rt < radius)
					continue;
				if (rs >= radius && rt >= radius)
					continue;
				
				p += norm(ee)
					* norm(ee)
					* it->dt
					/ (2.0 * pow(ee.dot(ee), 3.0 / 2.0));
			}
		}
	}

	return p;
}

double per(Neighborhood &neigh, Mat &image) {
	double p = 0.0;

	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			Neighborhood::iterator it;
			for (it = neigh.begin(); it != neigh.end(); ++it) {
				Mat ee = (Mat_<double>(2, 1) << it->x, it->y);

				int x = j + it->x;
				int y = i + it->y;

				if (it->x < 0)
					continue;
				if (it->x == 0 && it->y > 0)
					continue;
				
				if (x < 0 || x >= image.cols || y < 0 || y >= image.rows)
					continue;

				if (image.at<uchar>(i, j) == image.at<uchar>(y, x))
					continue;

				p += norm(ee)
					* norm(ee)
					* it->dt
					/ (2.0 * pow(ee.dot(ee), 3.0 / 2.0));
			}
		}
	}

	return p;
}

int main(int argc, char *argv[])
{
	Mat image;
	image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	double radius = atof(argv[2]);

	Neighborhood neigh;
	/* Size 4 */
	neigh.add( 1, 0, 1.0);
	neigh.add( 0, 1, 1.0);
	neigh.add(-1, 0, 1.0);
	neigh.add( 0,-1, 1.0);
	neigh.setupAngles();

	cout << "Size 4: " << per(neigh, radius) << endl;
	cout << "Size 4: " << per(neigh, image) << endl;
	cout << "Size 4: " << per(neigh, image) / M_PI << endl;

	/* Size 8 */
	neigh.add( 1, 1, 1.0);
	neigh.add(-1, 1, 1.0);
	neigh.add( 1,-1, 1.0);
	neigh.add(-1,-1, 1.0);
	neigh.setupAngles();

	cout << "Size 8: " << per(neigh, radius) << endl;
	cout << "Size 8: " << per(neigh, image) << endl;
	cout << "Size 8: " << per(neigh, image) / M_PI << endl;

	/* Size 16 */
	neigh.add8(1, 2, 1.0);
	neigh.setupAngles();

	cout << "Size 16: " << per(neigh, radius) << endl;
	cout << "Size 16: " << per(neigh, image) << endl;
	cout << "Size 16: " << per(neigh, image) / M_PI << endl;

	/* Size 32 */
	neigh.add8(3, 1, 1.0);
	neigh.add8(3, 2, 1.0);
	neigh.setupAngles();

	cout << "Size 32: " << per(neigh, radius) << endl;
	cout << "Size 32: " << per(neigh, image) << endl;
	cout << "Size 32: " << per(neigh, image) / M_PI << endl;

	/* Size 48 */
	neigh.add8(1, 4, 1.0);
	neigh.add8(3, 4, 1.0);
	neigh.setupAngles();

	cout << "Size 48: " << per(neigh, radius) << endl;
	cout << "Size 48: " << per(neigh, image) << endl;
	cout << "Size 48: " << per(neigh, image) / M_PI << endl;

	/* Size 72 */
	neigh.add8(1, 5, 1.0);
	neigh.add8(2, 5, 1.0);
	neigh.add8(3, 5, 1.0);
	neigh.setupAngles();

	cout << "Size 72: " << per(neigh, radius) << endl;
	cout << "Size 72: " << per(neigh, image) << endl;
	cout << "Size 72: " << per(neigh, image) / M_PI << endl;

	return 0;
}

