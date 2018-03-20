#pragma once
#ifndef CAMERACALIBRATION_H
#define CAMERACALIBRATION_H

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

class CameraCalibration
{
private:
	const float calibrationSquareDimension = 0.03875f; //meters
	const cv::Size chessboardDimensions = cv::Size(6, 9);
	std::vector<cv::Mat> _gatheredImages;
	void createKnownBoardPosition(cv::Size boardSize, float squareEdgeLength, std::vector<cv::Point3f>& corners);
	void getChessboardCorners(std::vector<cv::Mat> images, std::vector<std::vector<cv::Point2f>>& allFoundCorners, bool showResults = false);
	void cameraCalibration(std::vector<cv::Mat> calibrationImages, cv::Size boardSize, float squareLength, cv::Mat& cameraMatrix, cv::Mat& distanceCoefficients);
	bool savedCameraCalibration(std::string name, cv::Mat cameraMatrix, cv::Mat distanceCoefficients);
	bool loadCameraCalibration(std::string name, cv::Mat cameraMatrix, cv::Mat distanceCoefficients);
public:
	CameraCalibration(std::vector<cv::Mat> gatheredImages) { _gatheredImages = gatheredImages; };
	void RunCameraCalibration();

};
#endif