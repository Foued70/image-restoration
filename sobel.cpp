#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "graph.hpp"
#include "selectionrule.hpp"
#include "neighborhood.hpp"
#include "image.hpp"
#include "sobel.hpp"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	int blur = 10;
	int scale = 1;
	int delta = 0;

	int p = 2;
	double beta = 10;
	char rarg = 'f';
	int neighbors = 8;
	int index;
	int c;

	/* Read command line parameters beta and p. */
	while ((c = getopt(argc, argv, "b:p:n:fh")) != -1) {
		switch (c)
		{
		case 'p':
			p = atoi(optarg);
			break;
		case 'b':
			beta = atof(optarg);
			break;
		case 'f':
			rarg = 'f';
			break;
		case 'h':
			rarg = 'h';
			break;
		case 'n':
			neighbors = atoi(optarg);
			break;
		case '?':
			if (optopt == 'p')
				fprintf(stderr, "Option -%c requires an argument.\n",
						optopt);
			if (optopt == 'b')
				fprintf(stderr, "Option -%c requires an argument.\n",
						optopt);
			if (optopt == 'n')
				fprintf(stderr, "Option -%c requires an argument.\n",
						optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n",
						optopt);
			return 1;
		default:
			exit(1);
		}
	}

	cout << "Using blur = " << blur << endl;

	/*
	 * Non-option arguments are now in argv from index optind
	 * to index argc-1
	 */
	Mat image, grad;
	image = imread(argv[optind], CV_LOAD_IMAGE_GRAYSCALE);

	if (!image.data) {
		cout << "Loading image failed" << endl;
		return -1;
	}

	int rows = image.rows;
	int cols = image.cols;
	int pixels = rows * cols;

	GaussianBlur(image, image, Size(3,3), 0, 0, BORDER_DEFAULT);

	Mat grad_x, grad_y;

	Sobel(image, grad_x, CV_16S, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	Sobel(image, grad_y, CV_16S, 0, 1, 3, scale, delta, BORDER_DEFAULT);

	Mat x_sq, y_sq, xy;

	x_sq = grad_x.mul(grad_x);
	y_sq = grad_y.mul(grad_y);
	xy   = grad_x.mul(grad_y);

	GaussianBlur(x_sq, x_sq, Size(5,5), 0, 0, BORDER_DEFAULT);
	GaussianBlur(y_sq, y_sq, Size(5,5), 0, 0, BORDER_DEFAULT);
	GaussianBlur(xy  , xy  , Size(5,5), 0, 0, BORDER_DEFAULT);

	double g = 1000.0;

	Mat_<Tensor> tensors = Mat_<Tensor>::zeros(image.rows, image.cols);

	Mat evec, eval;

	for (int i = 0; i < image.rows; i += 10) {
		for (int j = 0; j < image.cols; j += 10) {
			tensors(i, j) = Tensor(
					x_sq.at<short>(i,j),
					xy.at<short>(i,j),
					xy.at<short>(i,j),
					y_sq.at<short>(i,j)
					);

			eigen(tensors(i, j), eval, evec);

			double s1 = eval.at<double>(0,0);
			double s2 = eval.at<double>(0,1);
			double l1 = 1.0 / (1.0 + (s1 - s2) * (s1 - s2) / (g*g));
			double l2 = 1.0;

			Point2f p1(evec.at<double>(0,0), evec.at<double>(0,1));
			Point2f p2(evec.at<double>(1,0), evec.at<double>(1,1));
			line(image, Point(j, i), Point2f(j, i) + 9 * l1 * p1, 255);
			line(image, Point(j, i), Point2f(j, i) + 9 * l2 * p2, 0);
		}
	}
	imwrite(argv[optind + 1], image);

	cout << tensors(10,10) << endl;

	/*
	 * Network only handles integer edges, so for floating
	 * point beta parameters, we cheat a little bit.
	 */
	int a;
	int b;
	a = 1;
	b = beta;
	if (beta < 10) {
		b = 100 * beta;
		a = 100;
	}

	/*
	 * Specify one quarter of the neighbors of a pixel. The rest
	 * are added symmetrically on the other sides.
	 */
	Neighborhood neigh;
	if (neighbors >= 2) {
		neigh.add( 1, 0, b * 1.0);
		neigh.add( 0, 1, b * 1.0);
	}
	if (neighbors >= 4) {
		neigh.add( 1, 1, b * 1.0/sqrt(2.0));
		neigh.add(-1, 1, b * 1.0/sqrt(2.0));
	}

	HighestLevelRule hrule(pixels + 2);
	FIFORule frule(pixels + 2);

	SelectionRule* rule;
	if (rarg == 'h') {
		cout << "Using highest level selection rule." << endl;
		rule = &hrule;
	} else {
		cout << "Using FIFO selection rule." << endl;
		rule = &frule;
	}

	Mat out = image.clone();

	Image im(&image, &out, *rule, neigh);
	//im.restore(a, p);
	im.restoreAnisotropicTV(a, p, tensors);

	//imwrite(argv[optind + 1], out);

	return 0;
}

