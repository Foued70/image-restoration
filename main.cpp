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
#include "anisotropy.hpp"

using namespace std;
using namespace cv;

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
	Mat blur, edge, structure, color;
	createAnisotropyTensor(tensors, image, sigma, rho, gamma,
			blur, edge, structure, color);
	imwrite(argv[optind + 1], blur);
	imwrite(argv[optind + 2], edge);
	imwrite(argv[optind + 3], structure);
	imwrite(argv[optind + 4], color);

	/*
	 * Network only handles integer edges, so we increase the scale a bit.
	 */
	int a;
	int b;
	a = 100;
	b = beta;

	/*
	 * Specify the neighbors of a pixel.
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

	cout << "Neighborhood: " << endl;
	neigh.setupAngles();
	for (Neighborhood::iterator it = neigh.begin(); it != neigh.end(); ++it) {
		cout << it->x << ", " << it->y << ": " << it->dt * 180 / M_PI << endl;
	}

	Mat out = image.clone();
	restoreAnisotropicTV(image, out, tensors, neigh, a, b, p);

	cout << "Writing output to " << argv[optind + 3] << endl;
	imwrite(argv[optind + 5], out);

	return 0;
}

