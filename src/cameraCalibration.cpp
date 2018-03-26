#include "cameraCalibration.h"
#include <catch.hpp>
#include <ctime>

using namespace calibration;

void CameraCalibration::createKnownBoardPosition(cv::Size boardSize, float squareEdgeLength,
                                                 std::vector<cv::Point3f>& corners)
{
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			corners.push_back(cv::Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));
		}
	}
}

//extracting chessboard corners
void CameraCalibration::getChessboardCorners(std::vector<cv::Mat> images,
                                             std::vector<std::vector<cv::Point2f>>& allFoundCorners,
                                             bool showResults)
{
	for (std::vector<cv::Mat>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		std::vector<cv::Point2f> pointBuf;
		bool found = findChessboardCorners(*iter, _chessboardDimensions,
			pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		if (found)
		{
			allFoundCorners.push_back(pointBuf);
		}

		if (showResults)
		{
			drawChessboardCorners(*iter, cv::Size(6, 9), pointBuf, found);
			imshow("Looking for corners", *iter);
			cv::waitKey(0);
		}
	}
}

void CameraCalibration::cameraCalibration(std::vector<cv::Mat> calibrationImages,
                                          cv::Size boardSize,
                                          float squareLength, cv::Mat& cameraMatrix,
                                          cv::Mat& distanceCoefficients)
{
	std::vector<std::vector<cv::Point2f>> checkerboardImageSpacePoints;
	getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

	std::vector<std::vector<cv::Point3f>> worldSpaceCornerPoints(1);

	createKnownBoardPosition(boardSize, squareLength, worldSpaceCornerPoints[0]);
	worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

	std::vector<cv::Mat> rVectors, tVectors;
	distanceCoefficients = cv::Mat::zeros(8, 1, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix,
	                distanceCoefficients,
	                rVectors, tVectors);
}

bool CameraCalibration::saveCameraCalibration(std::string name)
{
	std::ofstream outStream(name);
	if (outStream)
	{
		uint16_t rows = _cameraMatrix.rows;
		uint16_t columns = _cameraMatrix.cols;

		outStream << rows << std::endl;
		outStream << columns << std::endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = _cameraMatrix.at<double>(r, c);
				outStream << value << std::endl;
			}
		}

		rows = _distanceCoefficients.rows;
		columns = _distanceCoefficients.cols;

		outStream << rows << std::endl;
		outStream << columns << std::endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = _distanceCoefficients.at<double>(r, c);
				outStream << value << std::endl;
			}
		}
		outStream.close();
		return true;
	}
	return false;
}

bool CameraCalibration::loadCameraCalibration(std::string name)
{
	std::ifstream inStream(name);
	if (inStream)
	{
		uint16_t rows;
		uint16_t columns;

		inStream >> rows;
		inStream >> columns;

		_cameraMatrix = cv::Mat(cv::Size(columns, rows), CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				_cameraMatrix.at<double>(r, c) = read;
				std::cout << _cameraMatrix.at<double>(r, c) << "\n";
			}
		}
		//distance coefficients
		inStream >> rows;
		inStream >> columns;

		_distanceCoefficients = cv::Mat::zeros(rows, columns, CV_64F);
		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				_distanceCoefficients.at<double>(r, c) = read;
				std::cout << _distanceCoefficients.at<double>(r, c) << "\n";
			}
		}
		inStream.close();
		return true;
	}
	return false;
}

void CameraCalibration::runCameraCalibration()
{
	std::vector<cv::Mat> savedImages;
	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	for (std::vector<cv::Mat>::iterator iter = _gatheredImages.begin(); 
		iter != _gatheredImages.end(); iter++)
	{
		cv::Mat frame = *iter;

		std::vector<cv::Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, _chessboardDimensions, foundPoints,
		                              CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		if (found)
		{
			cv::Mat temp;
			frame.copyTo(temp);
			savedImages.push_back(temp);
		}
	}
	if(!savedImages.empty())
	{
		cameraCalibration(savedImages, _chessboardDimensions, _calibrationSquareDimension, _cameraMatrix,
			_distanceCoefficients);
		time_t now = time(0);
		tm ltm;
		localtime_s(&ltm, &now);
		saveCameraCalibration("camcalib_" + std::to_string(1900 + ltm.tm_year) + "_"
			+ std::to_string(1 + ltm.tm_mon) + "_" + std::to_string(ltm.tm_mday) + "_"
			+ std::to_string(ltm.tm_hour) + "_" + std::to_string(ltm.tm_min + 1) + "_" 
			+ std::to_string(ltm.tm_sec + 1));
	}
}

CameraCalibration::CameraCalibration(std::vector<cv::Mat> gatheredImages, 
	cv::Size chessboardDimensions, float chessboardSquareDimension)
{
	_gatheredImages = gatheredImages;
	_calibrationSquareDimension = chessboardSquareDimension;
	_chessboardDimensions = chessboardDimensions;
	runCameraCalibration();
};

CameraCalibration::CameraCalibration(std::string filePath)
{
	loadCameraCalibration(filePath);
};
