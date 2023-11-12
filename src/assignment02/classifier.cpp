#include "classifier.hpp"
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
using namespace std;

Classifier::Classifier(std::vector<Digit> dataSet, const int k)
: c_dataSet(std::move(dataSet))
, c_simplifiedSize(c_dataSet.front().points().size())
, c_k(k)
{
}

void Classifier::classify(const std::vector<cv::Point2f>& path)
{
	// equidistant sampling
	simplify(path);

	// normalize bounding box
	cv::Rect box = cv::boundingRect(m_simplifiedPath);
	cout << box.width << " " << box.y << endl;

	float scaleX = 100.0f / box.width;
	float scaleY = 100.0f / box.height;

	cout << "scaleX" << scaleX << "scaleY" << scaleY << endl;

	std::vector<cv::Point2f> normalizedPath = m_simplifiedPath;

	for (auto &point : normalizedPath) {
		point.x = (point.x - box.x) * scaleX;
		point.y = ((point.y - box.y) * scaleY - 100) * (-1);
	}
	
	cv::Rect box_after_scale = cv::boundingRect(normalizedPath);
	cout << box_after_scale.width << " " << box_after_scale.y << endl;

    // match using knn
	std::vector<std::pair<int, double>> distances;
	for (const Digit& digit : c_dataSet) {
		// Calculate distance between the user's features and the training dataset features

		double dist = 0;
		for (int i = 0; i < 8; i++) {
			dist += cv::norm(normalizedPath[i] - static_cast<cv::Point2f>(digit.points()[i]));
		}
		//double distance = cv::norm(m_simplifiedPath, digit2f, cv::NORM_L2);
		// distances.push_back(std::make_pair(digit.label(), distance));
		distances.push_back(std::make_pair(digit.label(), dist));
	}

	// Sort by distance in ascending order
	std::sort(distances.begin(), distances.end(), [](const auto& a, const auto& b) {
		return a.second < b.second;
	});

	// Select the top K neighbors
	std::map<int, int> labelCounts;

	for (int i = 0; i < c_k; i++) {
		labelCounts[distances[i].first]++;
	}
	for (int i = 0; i < 10; i++) {
		cout << "label:" << i << ", count:" << labelCounts[i] << endl;
	}
	cout << endl;

	// Find the most common label
	int maxCount = 0;
	for (const auto& pair : labelCounts) {
		if (pair.second > maxCount) {
			maxCount = pair.second;
			m_result = pair.first; // Set the result to the most common label
		}
	}
}

int Classifier::getResult() const
{
    return m_result;
}

std::vector<cv::Point2f> Classifier::getSimplifiedPath() const
{
    return m_simplifiedPath;
}

void Classifier::simplify(std::vector<cv::Point2f> path)
{
	if (path.size() < 2) return;
    // sample path with equal spacing and store in m_simplifiedPath
    // you should use c_simplifiedSize as number of points in simplified path
	std::vector<float> acc;           // n + 1 accumulated lengths
	std::vector<cv::Point2f> res;  // array of n + 1 result points

	int numSegments = 7;
	float len = 0;                // overall length
	
	cv::Point2f p = path[0];
	
	// find segment and overall lengths
	for (size_t i = 0; i < path.size(); i++) {
		cv::Point2f q = path[i];
		len += hypot(q.x - p.x, q.y - p.y);
		acc.push_back(float(len));
		p = q;
	}

	// TODO: What is this?
	acc.push_back(2 * len);          // sentinel
	size_t curr = 0;
	size_t next = 1;

	// create equidistant result points
	for (size_t i = 0; i < numSegments; i++) {
		float runningLength = len * i / numSegments;        // running length of point i

		// advance to current segment
		while (runningLength > acc[next]) {
			curr++;
			next++;
		}

		// interpolate in segment
		cv::Point2f p = path[curr];
		cv::Point2f q = path[next];

		// interpolation factor
		float t = (runningLength - acc[curr]) / (acc[next] - acc[curr]);
		res.push_back(cv::Point2f(
			p.x * (1 - t) + q.x * t,
			p.y * (1 - t) + q.y * t)
		);
	}

	// push end point (leave out when joining consecutive segments.)
	res.push_back(path[path.size() - 1]);

	m_simplifiedPath = res;
}
