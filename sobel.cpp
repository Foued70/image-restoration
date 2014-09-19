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

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	int blur = 10;
	int c;
	int scale = 1;
	int delta = 0;

	/* Read command line parameters beta and p. */
	while ((c = getopt(argc, argv, "b:")) != -1) {
		switch (c)
		{
		case 'b':
			blur = atoi(optarg);
			break;
		case '?':
			if (optopt == 'b')
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
	Mat abs_grad_x, abs_grad_y;

	Sobel(image, grad_x, CV_16S, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);

	Sobel(image, grad_y, CV_16S, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);

	// Approximate Total Gradient
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

	imwrite(argv[optind + 1], grad);

	//int sz[] = {10,10,2,2};
	int sz[] = {2,2,10,10};
	Mat L(4, sz, CV_32F);
	L = Scalar(0);

	Mat plane;

	const Mat* arr = { &L };
	NAryMatIterator it(&arr, &plane, 1);

	cout << it.nplanes << endl;
	for(int p = 0; p < it.nplanes; ++p, ++it)
	{
		cout << it.planes[p] << endl;
		cout << "BLOOP" << endl;
	}

	Mat_<int> b(2, 2, 0);
	Mat_<Mat_<int> > hey = Mat_<Mat_<int> >::zeros(10, 10);

	//hey(0,3).create(2, 2); // = Mat_<int>(2, 2, 1);
	cout << hey(3,3) << endl;
	//hey(3,3).create(2,2);

	for (int i = 0; i < hey.rows; ++i) {
		for (int j = 0; j < hey.cols; ++j) {
			//hey(i,j).create(2, 2);
		}
	}

	cout << hey.rows << " x " << hey.cols << endl;

	typedef Matx<float,2,2> Tensor;
	Mat_<Tensor> mt = Mat_<Tensor>::zeros(10,10);
	mt(2,2) = Tensor(1,2,3,4);
	cout << mt << endl;
	cout << mt(2,2) << endl;


	//hey(0,0).create(2, 2); // = Mat_<int>(2, 2, 1);

	//cout << hey(0,0) << endl;

	return 0;
}

