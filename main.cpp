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

//bool test1(void);
//bool test2(void);

//int f(int u, int v) {
//	return (u - v) * (u - v);
//}
//
//int Ei(int label, int pix, int u) {
//	return (f(label+1, pix) - f(label, pix)) * (1 - u);
//}
//
//int Eij(int b, int up, int uq) {
//	return b * ((1 - 2 * uq) * up + uq);
//}

/*
0 0 -> 0 * b
0 1 -> 1 * b
1 0 -> 1 * b
1 1 -> 0 * b
*/

int main(int argc, char *argv[])
{
	int p = 2;
	double beta = 10;
	int index;
	int c;

	while ((c = getopt (argc, argv, "b:p:")) != -1) {
		switch (c)
		{
		case 'p':
			p = atoi(optarg);
			break;
		case 'b':
			beta = atof(optarg);
			break;
		case '?':
			if (optopt == 'p')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			if (optopt == 'b')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		default:
			exit(1);
		}
	}

	cout << "Using p = " << p << endl;
	cout << "Using beta = " << beta << endl;

	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);

	Mat image;
	image = imread(argv[optind], CV_LOAD_IMAGE_GRAYSCALE);

	if (!image.data) {
		cout << "Loading image failed" << endl;
		return -1;
	}

	int rows = image.rows;
	int cols = image.cols;
	int pixels = rows * cols;
	int in_sum = 0;
	int out_sum = 0;

	int alpha;
	int b;

	alpha = 1;
	b = beta;
	if (beta < 5) {
		b = 100 * beta;
		alpha = 100;
	}

	Mat out = image.clone();

	HighestLevelRule hrule(pixels + 2);
	FIFORule frule(pixels + 2);
	Neighborhood neigh;
	Image im(&image, &out, dynamic_cast<SelectionRule&>(frule), neigh);
	im.restore(alpha, b, p);

	//im.restoreBisect(atoi(argv[2]));
	//FlowGraph network(pixels + 2, dynamic_cast<SelectionRule&>(frule));

	imwrite(argv[optind+1], out);

	//cout << "Input image pixel average: ";
	//cout << static_cast<double>(in_sum) / pixels << endl;
	//cout << "Output image pixel average: ";
	//cout << static_cast<double>(out_sum) / pixels << endl;

	return 0;
}

