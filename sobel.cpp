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
	int scale = 1;
	int delta = 0;

	int p = 2;
	double beta = 10;
	char rarg = 'f';
	int neighbors = 8;
	int index;
	int c;
	double g = 10000.0;
	double sigma = 10.0;
	double rho = 10.0;

	/* Read command line parameters beta and p. */
	while ((c = getopt(argc, argv, "b:p:r:s:g:n:fh")) != -1) {
		switch (c)
		{
		case 'p':
			p = atoi(optarg);
			break;
		case 'b':
			beta = atof(optarg);
			break;
		case 'g':
			g = atof(optarg);
			break;
		case 'r':
			rho = atof(optarg);
			break;
		case 's':
			sigma = atof(optarg);
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
			if (optopt == 'p' || optopt == 'b' || optopt == 'g'
					|| optopt == 'n' || optopt == 'r'
					|| optopt == 's') {
				fprintf(stderr, "Option -%c requires an argument.\n",
						optopt);
			}
			else if (isprint(optopt)) {
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			}
			else {
				fprintf(stderr, "Unknown option character `\\x%x'.\n",
						optopt);
			}
			return 1;
		default:
			exit(1);
		}
	}

	/*
	 * Non-option arguments are now in argv from index optind
	 * to index argc-1
	 */
	Mat image, grad, blurred;
	image = imread(argv[optind], CV_LOAD_IMAGE_GRAYSCALE);

	if (!image.data) {
		cout << "Loading image failed" << endl;
		return -1;
	}

	cout << "Using gamma = " << g << endl;
	cout << "Using rho = " << rho << endl;
	cout << "Using sigma = " << sigma << endl;

	int rows = image.rows;
	int cols = image.cols;
	int pixels = rows * cols;

	GaussianBlur(image, blurred, Size(0,0), sigma, 0, BORDER_DEFAULT);

	Mat grad_x, grad_y;

	Sobel(blurred, grad_x, CV_16S, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	Sobel(blurred, grad_y, CV_16S, 0, 1, 3, scale, delta, BORDER_DEFAULT);

	Mat x_sq, y_sq, xy;

	x_sq = grad_x.mul(grad_x);
	y_sq = grad_y.mul(grad_y);
	xy   = grad_x.mul(grad_y);

	GaussianBlur(x_sq, x_sq, Size(0,0), rho, 0, BORDER_DEFAULT);
	GaussianBlur(y_sq, y_sq, Size(0,0), rho, 0, BORDER_DEFAULT);
	GaussianBlur(xy  , xy  , Size(0,0), rho, 0, BORDER_DEFAULT);

	Mat_<Tensor> tensors = Mat_<Tensor>::zeros(image.rows, image.cols);

	Mat evec, eval;
	Mat structure = blurred.clone();

	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			Tensor b = Tensor(
					x_sq.at<short>(i,j),
					xy.at<short>(i,j),
					xy.at<short>(i,j),
					y_sq.at<short>(i,j)
					);

			/* Returns the eigenvectors as row vectors! */
			eigen(b, eval, evec);

			double s1 = eval.at<double>(0);
			double s2 = eval.at<double>(1);

			if (s2 > s1)
				fprintf(stderr, "OOPS: Wrong ordering of eigenvalues\n");

			double l1 = 1.0;
			double l2 = 1.0 / (1.0 + (s1 - s2) * (s1 - s2) / (g*g));

			Mat eval2 = eval.clone();
			eval2.at<double>(0) = l1;
			eval2.at<double>(1) = l2;
			tensors(i, j) = Tensor(Mat(evec.t() * Mat::diag(eval2) * evec));
		}
	}

	for (int i = 0; i < image.rows; i += 10) {
		for (int j = 0; j < image.cols; j += 10) {
			Tensor b = tensors(i, j);

			eigen(b, eval, evec);

			double s1 = eval.at<double>(0);
			double s2 = eval.at<double>(1);

			Point2f p1(evec.row(0));
			Point2f p2(evec.row(1));
			line(structure, Point(j, i), Point2f(j, i) + 9 * s2 * p1, 255);
			line(structure, Point(j, i), Point2f(j, i) + 9 * s1 * p2, 0);
		}
	}
	cout << "Writing structure to " << argv[optind + 1] << endl;
	imwrite(argv[optind + 1], structure);

	/*
	 * Network only handles integer edges, so for floating
	 * point beta parameters, we cheat a little bit.
	 */
	int a;
	int b;
	a = 100;
	b = beta;
	//if (beta < 10) {
	//	b = 100 * beta;
	//	a = 100;
	//}

	/*
	 * Specify one quarter of the neighbors of a pixel. The rest
	 * are added symmetrically on the other sides.
	 */
	Neighborhood neigh;
	neigh.add( 1, 0, b * 1.0);
	neigh.add( 0, 1, b * 1.0);
	neigh.add(-1, 0, b * 1.0);
	neigh.add( 0,-1, b * 1.0);

	neigh.add( 1, 1, b * 1.0/sqrt(2.0));
	neigh.add(-1, 1, b * 1.0/sqrt(2.0));
	neigh.add( 1,-1, b * 1.0/sqrt(2.0));
	neigh.add(-1,-1, b * 1.0/sqrt(2.0));

	neigh.add( 1, 2, b * 1.0/sqrt(2.0));
	neigh.add( 2, 1, b * 1.0/sqrt(2.0));
	neigh.add( 2,-1, b * 1.0/sqrt(2.0));
	neigh.add( 1,-2, b * 1.0/sqrt(2.0));
	neigh.add(-1, 2, b * 1.0/sqrt(2.0));
	neigh.add(-2, 1, b * 1.0/sqrt(2.0));
	neigh.add(-2,-1, b * 1.0/sqrt(2.0));
	neigh.add(-1,-2, b * 1.0/sqrt(2.0));

	neigh.add(-3, 1, b * 1.0/sqrt(2.0));
	neigh.add(-3, 2, b * 1.0/sqrt(2.0));
	neigh.add(-2, 3, b * 1.0/sqrt(2.0));
	neigh.add(-1, 3, b * 1.0/sqrt(2.0));
	neigh.add(-3,-1, b * 1.0/sqrt(2.0));
	neigh.add(-3,-2, b * 1.0/sqrt(2.0));
	neigh.add(-2,-3, b * 1.0/sqrt(2.0));
	neigh.add(-1,-3, b * 1.0/sqrt(2.0));
	neigh.add( 3, 1, b * 1.0/sqrt(2.0));
	neigh.add( 3, 2, b * 1.0/sqrt(2.0));
	neigh.add( 2, 3, b * 1.0/sqrt(2.0));
	neigh.add( 1, 3, b * 1.0/sqrt(2.0));
	neigh.add( 3,-1, b * 1.0/sqrt(2.0));
	neigh.add( 3,-2, b * 1.0/sqrt(2.0));
	neigh.add( 2,-3, b * 1.0/sqrt(2.0));
	neigh.add( 1,-3, b * 1.0/sqrt(2.0));

	neigh.setupAngles();
	for (Neighborhood::iterator it = neigh.begin(); it != neigh.end(); ++it) {
		cout << it->x << ", " << it->y << ": " << it->dt * 180 / M_PI << endl;
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
	im.restoreAnisotropicTV(a, b, p, tensors);

	cout << "Writing output to " << argv[optind + 2] << endl;
	imwrite(argv[optind + 2], out);

	return 0;
}

