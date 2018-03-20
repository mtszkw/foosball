#pragma once
#include "cameraCalibration.h"

using namespace cv;
using namespace std;

void CameraCalibration::createKnownBoardPosition(cv::Size boardSize, float squareEdgeLength, std::vector<cv::Point3f>& corners)
{
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));

		}
	}
}

//extracting chessboard corners
void CameraCalibration::getChessboardCorners(std::vector<cv::Mat> images, std::vector<std::vector<cv::Point2f>>& allFoundCorners, bool showResults)
{
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		vector<Point2f> pointBuf;
		bool found = findChessboardCorners(*iter, Size(6, 9), pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		if (found)
		{
			allFoundCorners.push_back(pointBuf);
		}

		if (showResults)
		{
			drawChessboardCorners(*iter, Size(6, 9), pointBuf, found);
			imshow("Looking for corners", *iter);
			waitKey(0);
		}
	}
}

void CameraCalibration::cameraCalibration(std::vector<cv::Mat> calibrationImages, cv::Size boardSize, float squareLength, cv::Mat& cameraMatrix, cv::Mat& distanceCoefficients)
{
	vector<vector<Point2f>> checkerboardImageSpacePoints;
	getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

	vector<vector<Point3f>> worldSpaceCornerPoints(1);

	createKnownBoardPosition(boardSize, squareLength, worldSpaceCornerPoints[0]);
	worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

	vector<Mat> rVectors, tVectors;
	distanceCoefficients = Mat::zeros(8, 1, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
}

bool CameraCalibration::savedCameraCalibration(std::string name, cv::Mat cameraMatrix, cv::Mat distanceCoefficients)
{
	ofstream outStream(name);
	if (outStream)
	{
		uint16_t rows = cameraMatrix.rows;
		uint16_t columns = cameraMatrix.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = cameraMatrix.at<double>(r, c);
				outStream << value << endl;
			}
		}

		rows = distanceCoefficients.rows;
		columns = distanceCoefficients.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = cameraMatrix.at<double>(r, c);
				outStream << value << endl;
			}
		}
		outStream.close();
		return true;
	}
	return false;
}

bool CameraCalibration::loadCameraCalibration(std::string name, cv::Mat cameraMatrix, cv::Mat distanceCoefficients)
{
	ifstream inStream(name);
	if (inStream)
	{
		uint16_t rows;
		uint16_t columns;

		inStream >> rows;
		inStream >> columns;

		cameraMatrix = Mat(Size(columns, rows), CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				cameraMatrix.at<double>(r, c) = read;
				cout << cameraMatrix.at<double>(r, c) << "\n";
			}
		}
		//distance coefficients
		inStream >> rows;
		inStream >> columns;

		distanceCoefficients = Mat::zeros(rows, columns, CV_64F);
		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				distanceCoefficients.at<double>(r, c) = read;
				cout << distanceCoefficients.at<double>(r, c) << "\n";
			}
		}
		inStream.close();
		return true;
	}
	return false;
}

void CameraCalibration::RunCameraCalibration()
{
	Mat frame;
	Mat drawToFrame;
	vector<Mat> savedImages;
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);

	Mat distanceCoefficients;

	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	VideoCapture vid(0);

	if (!vid.isOpened())
	{
		return;
	}

	int fps = 40;

	namedWindow("WebCam", CV_WINDOW_AUTOSIZE);

	while (true)
	{
		if (!vid.read(frame))
			break;

		vector<Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, chessboardDimensions, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(drawToFrame);
		drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);
		if (found)
			imshow("Webcam", drawToFrame);
		else
			imshow("Webcam", frame);

		char character = waitKey(1000 / fps);

		switch (character)
		{
		case ' ':
			//saving image
			if (found)
			{
				Mat temp;
				frame.copyTo(temp);
				savedImages.push_back(temp);
				cout << savedImages.size() << endl;
			}
			break;
		case 13:
			//start calibration
			if (savedImages.size() > 15)
			{
				cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix, distanceCoefficients);
				savedCameraCalibration("camCalib", cameraMatrix, distanceCoefficients);
			}
			break;
		case 27:
			//exit
			return;
			break;
		}
		return;
	}
}

