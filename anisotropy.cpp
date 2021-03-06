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

	Mat grad;

	/* Apply blur by sigma. */
	GaussianBlur(in, blur, Size(0,0), sigma, 0, BORDER_REFLECT);

	Mat grad_x, grad_y;
	Mat kernel;

	Point anchor = Point(-1, -1);

       	kernel = Mat::zeros(1, 3, CV_16S);

	kernel.at<short>(0, 0) = -1;
	kernel.at<short>(0, 1) = 0;
	kernel.at<short>(0, 2) = 1;

	/* Calculate gradient in x and y. */
	filter2D(blur, grad_x, CV_64F, kernel, anchor, 0, BORDER_REFLECT);
	transpose(kernel, kernel);
	filter2D(blur, grad_y, CV_64F, kernel, anchor, 0, BORDER_REFLECT);

	Mat x_sq, y_sq, xy;
	Mat x_sqr, y_sqr, xyr;

	/* Element of outer product. */
	x_sq = grad_x.mul(grad_x);
	y_sq = grad_y.mul(grad_y);
	xy   = grad_x.mul(grad_y);

	/* Integration scale (rho) smoothing. */
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
			/* Structure tensor. */
			Tensor b = Tensor(
					x_sqr.at<double>(i,j),
					xyr.at<double>(i,j),
					xyr.at<double>(i,j),
					y_sqr.at<double>(i,j)
					);

			/* Structure tensor without rho smoothing, for the edge detector. */
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

			Mat eval2 = eval.clone();
			eval2.at<double>(0) = l1;
			eval2.at<double>(1) = l2;
			tensors(i, j) = Tensor(Mat(evec.t() * Mat::diag(eval2) * evec));

			h.at<double>(i, j) = fmod(atan2(evec.at<double>(1), evec.at<double>(0))
					* 180.0 / M_PI + 180.0, 180.0);
			s.at<double>(i, j) = 0;
			v.at<double>(i, j) = 1.0/(1.0 + l2);

			/* Returns the eigenvectors as row vectors! */
			eigen(c, eval, evec);

			s1 = eval.at<double>(0);
			s2 = eval.at<double>(1);

			edge.at<double>(i, j) = s1;
		}
	}
	normalize(edge, edge, 0, 255, NORM_MINMAX, CV_8U);

	/* Create tensor visualization, double the size. */
	structure.create(in.size(), CV_64F);
	GaussianBlur(in, structure, Size(0,0), sigma, 0, BORDER_REFLECT);
	cvtColor(structure, structure, CV_GRAY2BGR);
	resize(structure, structure, Size(0, 0), 2, 2);
	for (int i = 0; i < in.rows; i += 15) {
		for (int j = 0; j < in.cols; j += 15) {
			Tensor b = tensors(i, j);

			eigen(b, eval, evec);

			double s1 = eval.at<double>(0);
			double s2 = eval.at<double>(1);

			Point2f p1(evec.row(0));
			Point2f p2(evec.row(1));
			line(structure, 2*Point(j, i), 2*Point2f(j, i) + 20 * s2 * p1,
					CV_RGB(255,0,0), 1.2, CV_AA);
			line(structure, 2*Point(j, i), 2*Point2f(j, i) + 20 * s1 * p2,
					CV_RGB(0,0,0), 1.2, CV_AA);
		}
	}

	Mat ho, so, vo;
	h.convertTo(ho, CV_8U);
	normalize(s, so, 255, 255, NORM_MINMAX, CV_8U);
	normalize(v, vo, 0, 255, NORM_MINMAX, CV_8U);

	channels.push_back(ho);
	channels.push_back(so);
	channels.push_back(vo);
	merge(channels, hsv);

	cvtColor(hsv, color, CV_HSV2BGR);
}

/* Create a uniform tensor, for testing. */
void createUniformAnisotropyTensor(Mat_<Tensor>& tensors, Mat& in, double gamma) {
	Mat evec, eval;
	for (int i = 0; i < in.rows; ++i) {
		for (int j = 0; j < in.cols; ++j) {
			Tensor b = Tensor(1, 1, 1, 1);

			/* Returns the eigenvectors as row vectors! */
			eigen(b, eval, evec);

			double l1 = 1.0 / gamma;
			double l2 = 1.0;

			Mat eval2 = eval.clone();
			eval2.at<double>(0) = l1;
			eval2.at<double>(1) = l2;
			tensors(i, j) = Tensor(Mat::diag(eval2));
		}
	}
}

