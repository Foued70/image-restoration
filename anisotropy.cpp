#include <opencv2/opencv.hpp>
#include <cstdlib>
#include <cstdio>
#include "anisotropy.hpp"

using namespace cv;
using namespace std;

void createAnisotropyTensor(
		Mat_<Tensor>& tensors,
		Mat& in,
		double sigma,
		double rho,
		double gamma,
		cv::Mat& blur,
		cv::Mat& edge,
		cv::Mat& structure,
		cv::Mat& color
		) {

	int scale = 1;
	int delta = 0;

	Mat grad;

	int rows = in.rows;
	int cols = in.cols;
	int pixels = rows * cols;

	GaussianBlur(in, blur, Size(0,0), sigma, 0, BORDER_REFLECT);

	Mat grad_x, grad_y;
	Mat kernel;

	Point anchor = Point(-1, -1);

       	kernel = Mat::zeros(1, 3, CV_16S);

	kernel.at<short>(0, 0) = -1;
	kernel.at<short>(0, 1) = 0;
	kernel.at<short>(0, 2) = 1;

	filter2D(blur, grad_x, CV_64F, kernel, anchor, 0, BORDER_REFLECT);
	transpose(kernel, kernel);
	filter2D(blur, grad_y, CV_64F, kernel, anchor, 0, BORDER_REFLECT);

	Mat x_sq, y_sq, xy;
	Mat x_sqr, y_sqr, xyr;

	x_sq = grad_x.mul(grad_x);
	y_sq = grad_y.mul(grad_y);
	xy   = grad_x.mul(grad_y);

	GaussianBlur(x_sq, x_sqr, Size(0,0), rho, 0, BORDER_REFLECT);
	GaussianBlur(y_sq, y_sqr, Size(0,0), rho, 0, BORDER_REFLECT);
	GaussianBlur(xy  , xyr  , Size(0,0), rho, 0, BORDER_REFLECT);

	Mat evec, eval;

	Mat h = Mat::zeros(in.size(), CV_64F);
	Mat s = Mat::zeros(in.size(), CV_64F);
	Mat v = Mat::zeros(in.size(), CV_64F);
	Mat hsv;
	vector<Mat> channels;

	edge.create(in.size(), CV_64F);

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

			h.at<double>(i, j) = fmod(atan2(evec.at<double>(1), evec.at<double>(0)) * 180.0 / M_PI + 180.0, 180.0);
			s.at<double>(i, j) = 0;
			v.at<double>(i, j) = 1.0/(1.0 + l2);

			/* Returns the eigenvectors as row vectors! */
			eigen(c, eval, evec);

			s1 = eval.at<double>(0);
			s2 = eval.at<double>(1);

			edge.at<double>(i, j) = s1;
		}
	}
	//normalize(edge, edge, 0, 255, NORM_MINMAX, CV_8U);

	structure.create(in.size(), CV_64F);
	GaussianBlur(in, structure, Size(0,0), sigma, 0, BORDER_REFLECT);
	cvtColor(structure, structure, CV_GRAY2BGR);
	for (int i = 0; i < in.rows; i += 15) {
		for (int j = 0; j < in.cols; j += 15) {
			Tensor b = tensors(i, j);

			eigen(b, eval, evec);

			double s1 = eval.at<double>(0);
			double s2 = eval.at<double>(1);

			Point2f p1(evec.row(0));
			Point2f p2(evec.row(1));
			line(structure, Point(j, i), Point2f(j, i) + 10 * s2 * p1,
					CV_RGB(255,0,0));
			line(structure, Point(j, i), Point2f(j, i) + 10 * s1 * p2,
					CV_RGB(0,0,0));
			//ellipse(structure, Point(j, i), Size(10*s1, 10*s2),
			//		atan2(p2.y, p2.x) * 180 / M_PI,
			//		0, 360,
			//		CV_RGB(0, 200, 200));
		}
	}

	Mat ho, so, vo;
	normalize(h, ho, 0, 180, NORM_MINMAX, CV_8U);
	normalize(s, so, 255, 255, NORM_MINMAX, CV_8U);
	normalize(v, vo, 0, 255, NORM_MINMAX, CV_8U);

	channels.push_back(ho);
	channels.push_back(so);
	channels.push_back(vo);
	merge(channels, hsv);

	cvtColor(hsv, color, CV_HSV2BGR);
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

