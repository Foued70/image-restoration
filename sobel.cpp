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

void createAnisotropyTensor(
		Mat_<Tensor>& tensors,
		Mat& in,
		double sigma,
		double rho,
		double gamma,
		char *fstructure,
		char *fedge
		) {

	int scale = 1;
	int delta = 0;

	Mat grad, blurred;

	int rows = in.rows;
	int cols = in.cols;
	int pixels = rows * cols;

	GaussianBlur(in, blurred, Size(0,0), sigma, 0, BORDER_REFLECT);

	Mat grad_x, grad_y;
	Mat kernel;

	Point anchor = Point(-1, -1);

       	kernel = Mat::zeros(1, 3, CV_16S);

	kernel.at<short>(0, 0) = -1;
	kernel.at<short>(0, 1) = 0;
	kernel.at<short>(0, 2) = 1;

	filter2D(blurred, grad_x, CV_64F, kernel, anchor, 0, BORDER_REFLECT);
	transpose(kernel, kernel);
	filter2D(blurred, grad_y, CV_64F, kernel, anchor, 0, BORDER_REFLECT);

	Mat x_sq, y_sq, xy;
	Mat x_sqr, y_sqr, xyr;

	x_sq = grad_x.mul(grad_x);
	y_sq = grad_y.mul(grad_y);
	xy   = grad_x.mul(grad_y);

	GaussianBlur(x_sq, x_sqr, Size(0,0), rho, 0, BORDER_REFLECT);
	GaussianBlur(y_sq, y_sqr, Size(0,0), rho, 0, BORDER_REFLECT);
	GaussianBlur(xy  , xyr  , Size(0,0), rho, 0, BORDER_REFLECT);

	Mat evec, eval;
	Mat edgedetect(in.size(), CV_64F);

	Mat h = Mat::zeros(in.size(), CV_64F);
	Mat s = Mat::zeros(in.size(), CV_64F);
	Mat v = Mat::zeros(in.size(), CV_64F);
	Mat hsv;
	vector<Mat> channels;

	for (int i = 0; i < in.rows; ++i) {
		for (int j = 0; j < in.cols; ++j) {
			Tensor b = Tensor(
					x_sqr.at<double>(i,j),
					xyr.at<double>(i,j),
					xyr.at<double>(i,j),
					y_sqr.at<double>(i,j)
					);

			Tensor c = Tensor(
					x_sq.at<double>(i,j),
					xy.at<double>(i,j),
					xy.at<double>(i,j),
					y_sq.at<double>(i,j)
					);

			/* Returns the eigenvectors as row vectors! */
			eigen(b, eval, evec);

			double s1 = eval.at<double>(0);
			double s2 = eval.at<double>(1);

			if (s2 > s1)
				fprintf(stderr, "OOPS: Wrong ordering of eigenvalues\n");

			double l1 = 1.0;
			double l2 = 1.0 / (1.0 + (s1 - s2) * (s1 - s2) / (gamma*gamma));
			//double C  = 1000;
			//double l1 = gamma + (1.0 - gamma) * exp(-C / ((s1-s2)*(s1-s2)));
			//double l2 = gamma;

			Mat eval2 = eval.clone();
			eval2.at<double>(0) = l1;
			eval2.at<double>(1) = l2;
			tensors(i, j) = Tensor(Mat(evec.t() * Mat::diag(eval2) * evec));
			if (i == j)
				cout << eval2 << endl;

			h.at<double>(i, j) = fmod(atan2(evec.at<double>(1), evec.at<double>(0)) * 180.0 / M_PI + 180.0, 180.0);
			s.at<double>(i, j) = 0;
			v.at<double>(i, j) = 1.0/(1.0 + l2);

			/* Returns the eigenvectors as row vectors! */
			eigen(c, eval, evec);

			s1 = eval.at<double>(0);
			s2 = eval.at<double>(1);

			edgedetect.at<double>(i, j) = s1;
		}
	}
	normalize(edgedetect, edgedetect, 0, 255, NORM_MINMAX, CV_8U);

	cout << "Writing edges to " << fedge << endl;
	imwrite(fedge, edgedetect);

	Mat structure;
	GaussianBlur(in, structure, Size(0,0), sigma, 0, BORDER_REFLECT);
	for (int i = 0; i < in.rows; i += 10) {
		for (int j = 0; j < in.cols; j += 10) {
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

	cout << "Writing structure to " << fstructure << endl;
	imwrite(fstructure, structure);

	Mat ho, so, vo;
	normalize(h, ho, 0, 180, NORM_MINMAX, CV_8U);
	normalize(s, so, 255, 255, NORM_MINMAX, CV_8U);
	normalize(v, vo, 2, 255, NORM_MINMAX, CV_8U);

	channels.push_back(ho);
	channels.push_back(so);
	channels.push_back(vo);
	merge(channels, hsv);

	//cout << hsv << endl;
	Mat colortensor;
	cvtColor(hsv, colortensor, CV_HSV2BGR);
	//cout << boop << endl;

	imwrite("tensor.png", colortensor);
}

void createUniformAnisotropyTensor(Mat_<Tensor>& tensors, Mat& in, double gamma) {
	int rows = in.rows;
	int cols = in.cols;

	Mat evec, eval;
	for (int i = 0; i < in.rows; ++i) {
		for (int j = 0; j < in.cols; ++j) {
			Tensor b = Tensor(1, 1, 1, 1);

			/* Returns the eigenvectors as row vectors! */
			eigen(b, eval, evec);

			double s1 = eval.at<double>(0);
			double s2 = eval.at<double>(1);

			double l1 = 1.0 / gamma;
			double l2 = 1.0;

			Mat eval2 = eval.clone();
			eval2.at<double>(0) = l1;
			eval2.at<double>(1) = l2;
			tensors(i, j) = Tensor(Mat::diag(eval2));
		}
	}
}

int main(int argc, char *argv[])
{
	int p = 2;
	double beta = 10;

	int neighbors = 8;

	double sigma = 10.0;
	double rho   = 10.0;
	double gamma = 10000.0;

	int c;

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
			gamma = atof(optarg);
			break;
		case 'r':
			rho = atof(optarg);
			break;
		case 's':
			sigma = atof(optarg);
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
	Mat image;
	image = imread(argv[optind], CV_LOAD_IMAGE_GRAYSCALE);

	if (!image.data) {
		cout << "Loading image failed" << endl;
		return -1;
	}

	cout << "Using gamma = " << gamma << endl;
	cout << "Using rho = " << rho << endl;
	cout << "Using sigma = " << sigma << endl;

	Mat_<Tensor> tensors = Mat_<Tensor>::zeros(image.rows, image.cols);
	createAnisotropyTensor(tensors, image, sigma, rho, gamma,
			argv[optind + 1], argv[optind + 2]);
	//createUniformAnisotropyTensor(tensors, image, gamma);

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
	cout << "Creating size " << neighbors << " neighborhood." << endl;
	Neighborhood neigh;
	if (neighbors >= 4) {
		neigh.add( 1, 0, b * 1.0);
		neigh.add( 0, 1, b * 1.0);
		neigh.add(-1, 0, b * 1.0);
		neigh.add( 0,-1, b * 1.0);
	}

	if (neighbors >= 8) {
		neigh.add( 1, 1, b * 1.0/sqrt(2.0));
		neigh.add(-1, 1, b * 1.0/sqrt(2.0));
		neigh.add( 1,-1, b * 1.0/sqrt(2.0));
		neigh.add(-1,-1, b * 1.0/sqrt(2.0));
	}

	if (neighbors >= 16) {
		neigh.add8(1, 2, 1.0);
	}

	if (neighbors >= 32) {
		neigh.add8(3, 1, 1.0);
		neigh.add8(3, 2, 1.0);
	}

	if (neighbors >= 48) {
		neigh.add8(1, 4, 1.0);
		neigh.add8(3, 4, 1.0);
	}

	if (neighbors >= 72) {
		neigh.add8(1, 5, 1.0);
		neigh.add8(2, 5, 1.0);
		neigh.add8(3, 5, 1.0);
	}

	neigh.setupAngles();
	for (Neighborhood::iterator it = neigh.begin(); it != neigh.end(); ++it) {
		cout << it->x << ", " << it->y << ": " << it->dt * 180 / M_PI << endl;
	}

	Mat out = image.clone();

	restoreAnisotropicTV(image, out, tensors, neigh, a, b, p);

	cout << "Writing output to " << argv[optind + 3] << endl;
	imwrite(argv[optind + 3], out);

	return 0;
}

