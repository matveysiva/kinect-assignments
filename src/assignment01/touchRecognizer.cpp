#include "touchRecognizer.hpp"
#include <iostream>
#include <algorithm>
using namespace std;
#include <vector>
#include <opencv2/opencv.hpp>

cv::Mat m_background;

void TouchRecognizer::calibrate(const cv::Mat& background)
{
	cout << "calibrate" << endl;
	m_background = background.clone();
}

std::vector<cv::RotatedRect> TouchRecognizer::recognize(const cv::Mat& depthFrame) const
{
    std::vector<cv::RotatedRect> positions;
	
	cv::absdiff(m_background, depthFrame, depthFrame);
	return positions;
	cv::Size ksize = cv::Size(9,9);

	cv::threshold(depthFrame, depthFrame, 50, 1000, CV_THRESH_TOZERO_INV);
	cv::blur(depthFrame, depthFrame, ksize);
	cv::threshold(depthFrame, depthFrame, 10, 1000, CV_THRESH_BINARY);

	// Convert to correct type
	cv::Mat depth2;
	depthFrame.convertTo(depth2, CV_8UC1, 1.0/255.0);

	// Find contours in the processed depth image
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(depth2, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

	// Convert back
	depth2.convertTo(depthFrame, CV_16UC1, 255);

	// Loop through the contours and generate bounding boxes
	for (const auto& contour : contours) {
		if (contour.size() > 150) {  // Filter small contours (adjust this threshold as needed)
			cv::RotatedRect box = cv::minAreaRect(contour);
			positions.push_back(box);
		}
	}
	
    return positions;
}
