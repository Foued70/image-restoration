#pragma once

#include <opencv2/opencv.hpp>

typedef cv::Matx<double, 2, 2> Tensor;

void createAnisotropyTensor(
		cv::Mat_<Tensor>& tensors,
		cv::Mat& in,
		double sigma,
		double rho,
		double gamma,
		cv::Mat& blur,
		cv::Mat& edge,
		cv::Mat& structure,
		cv::Mat& color
		);

void createUniformAnisotropyTensor(cv::Mat_<Tensor>& tensors, cv::Mat& in, double gamma);

