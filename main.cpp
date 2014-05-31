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
	int p = 2;
	double beta = 10;
	char rarg = 'f';
	int neighbors = 4;
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
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			if (optopt == 'b')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			if (optopt == 'n')
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

	if (neighbors != 4 && neighbors != 8) {
		cout << "Only 4 or 8 neighbors supported, using 4." << endl;
		neighbors = 4;
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

	int rows = image.rows;
	int cols = image.cols;
	int pixels = rows * cols;

#ifdef DINIC
	cout << "Using Dinic's maximum flow algorithm." << endl;
#else
	cout << "Using the Push-Relabel maximum flow algorithm." << endl;
#endif

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

	Mat out = image.clone();

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

	Image im(&image, &out, *rule, neigh);
	im.restore(a, p);

	imwrite(argv[optind + 1], out);

	return 0;
}

