#pragma once
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

namespace calibration
{
	class CameraCalibration
	{
	private:
		float _calibrationSquareDimension; // in meters
		cv::Size _chessboardDimensions;
		std::vector<cv::Mat> _gatheredImages;
		cv::Mat _cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
		cv::Mat _distanceCoefficients;
		static void createKnownBoardPosition(cv::Size boardSize, 
			float squareEdgeLength, std::vector<cv::Point3f>& corners);
		void getChessboardCorners(std::vector<cv::Mat> images, 
			std::vector<std::vector<cv::Point2f>>& allFoundCorners, 
			bool showResults = false);
		void cameraCalibration(std::vector<cv::Mat> calibrationImages, 
			cv::Size boardSize, float squareLength, cv::Mat& cameraMatrix, 
			cv::Mat& distanceCoefficients);
		bool saveCameraCalibration(std::string name);
		bool loadCameraCalibration(std::string name);
		void runCameraCalibration();
	public:
		CameraCalibration(std::vector<cv::Mat> gatheredImages, cv::Size chessboardDimensions,
			float chessboardSquareDimension);
		CameraCalibration(std::string filePath);
		cv::Mat getCameraMatrix() { return  _cameraMatrix; }; // first calibration vector
		cv::Mat getDistanceCoefficients() { return  _distanceCoefficients; }; 
		// second callibration vector
	};
}
