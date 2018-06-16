#pragma
#include <opencv2/opencv.hpp>
#include "detection/score.hpp"
namespace gui
{
	void consoleKeyDoc();
	void showOriginalFrame(bool originalEnabled, cv::Mat& frame);
	void handlePressedKeys(int key, bool& originalEnabled, bool& trackingEnabled, bool& blueDetectionEnabled, bool& redDetectionEnabled, bool& pause, bool& debugMode);
	void printKeyDoc(cv::Mat& frame, int x, int y);
	void printScoreBoard(detection::ScoreCounter& scoreCounter, cv::Mat &res, int x, int y);
	void showCenterPosition(cv::Mat& res, cv::Point center, int x, int y);
	void showStatistics(cv::Mat& res, int founded, int all, int x, int y);
}