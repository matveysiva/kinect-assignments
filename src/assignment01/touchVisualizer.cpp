#include "../assignment01/touchVisualizer.hpp"

void TouchVisualizer::draw(cv::Mat & frame, const cv::RotatedRect & position, cv::Mat & output)
{
	cv::ellipse(output, position, cv::Scalar(0,0,255), 1, 8); //Red Ellipse
}

void TouchVisualizer::drawDots(cv::Mat & frame, const std::vector<cv::Point> & dots, cv::Mat & output)
{
	for (auto dot : dots) {
		cv::circle(output, dot, 8, cv::Scalar(255, 255, 255), -1); //White Circle
	}
}