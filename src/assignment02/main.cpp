#include "../framework/kinect.hpp"
#include "../assignment01/touchRecognizer.hpp"
#include "classifier.hpp"
#include "digitVisualizer.hpp"
#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
#include "data.hpp"

int main()
{
    // initialize framework
    Kinect::initialize();
    Kinect::listDevices();

    // set up device
    Kinect kinect;
    if(!kinect.ready())
    {
        std::cout << "Could not set up Kinect. Exiting." << std::endl;
        return -1;
    }

    // prepare touch pad
    TouchRecognizer touchRecognizer;

    // load data set
    const auto data = DataSet::read("pendigits.tra");

    // prepare variables
    cv::Mat depthFrame;
    cv::Mat colorFrame;
    cv::Mat outputFrame;
    auto running = true;
    std::vector<cv::Point2f> path;
	const std::chrono::milliseconds interval(10);

    // calibrate
    kinect.getDepthFrame(depthFrame);
    touchRecognizer.calibrate(depthFrame);

    // prepare classifier and viewer
    Classifier classifier(data, 10);
    DigitVisualizer digitVisualizer;

    // prepare windows - this isn't necessary, but it allows to assign useful positions
    cv::namedWindow("color");
    cv::namedWindow("depth");
    cv::namedWindow("output");

    // move windows
    kinect.getColorFrame(colorFrame);
    const auto xOffset = colorFrame.cols;
    const auto yOffset = colorFrame.rows;
    cv::moveWindow("color", 0 * xOffset, 1 * yOffset);
    cv::moveWindow("depth", 1 * xOffset, 0 * yOffset);
    cv::moveWindow("output", 0 * xOffset, 0 * yOffset);

	auto current = std::chrono::system_clock::now();
	bool classified = 0;
    while(running)
    {
        // update frames
        kinect.getDepthFrame(depthFrame);
        kinect.getColorFrame(colorFrame);

		// run touch recognizer
		auto positions = touchRecognizer.recognize(depthFrame);

		// if foot is on the ground
        if(!positions.empty())
		{
			// get touch positions in regular intervals
			auto lastDrawn = std::chrono::system_clock::now();
			if (lastDrawn > current + interval) {
				current = lastDrawn;
				for (const auto& position : positions) {
					path.push_back(position.center);
				}
			}
			classified = 0;
		}
		// foot is in the air
        else
		{
			if (!path.empty() && !classified) {
				// done drawing -> classify
				classifier.classify(path);
				digitVisualizer.setSimplifiedPath(classifier.getSimplifiedPath());
				digitVisualizer.setResult(classifier.getResult());
				//digitVisualizer.setResult(0);
				classified = 1;
				path.clear();
			}
		}

        // visualize results
        digitVisualizer.draw(colorFrame, path, outputFrame);

        // show frames
        auto depthFrameUnscaled = depthFrame.clone();
        depthFrame = 20 * depthFrame;
        cv::imshow("depth", depthFrame);
        cv::imshow("color", colorFrame);
        cv::imshow("output", outputFrame);

        // check for keypresses
        const auto key = cv::waitKey(10);
        switch (key)
        {
		case 0x1b: // esc
            running = false;
            break;
		case 0x0d: // enter
			if (!path.empty()) path.clear();
			kinect.getDepthFrame(depthFrame);
			touchRecognizer.calibrate(depthFrame);
			digitVisualizer.setResult(-1);
			classified = 0;
            break;
		case 0x30: // number keys - you can use this to take a look at the data set
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39: 
            DataVisualizer::showNumber(data, key - 0x30);
            break;
        default:
            break;
        }
    }
}
