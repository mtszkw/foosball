#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

namespace detection
{
	class FoundBallsState
	{
		private:
			double ticks;
    		bool foundball;
    		int notFoundCount;
	       	cv::Point center;

		public:			
			cv::KalmanFilter kalmanFilter;
			cv::Mat state;  
    		cv::Mat meas;
			
			vector<vector<cv::Point> > contours;
        	vector<vector<cv::Point> > balls;
        	vector<cv::Rect> ballsBox;

			FoundBallsState(double ticks, bool foundball, int notFoundCount);

			double getTicks() { return ticks; };
			void setTicks(double newTicks) { ticks = newTicks; };

			bool getFoundball() {return foundball; };
			void setFoundball(bool newFoundball) {foundball = newFoundball; };

			int getNotFoundCount() {return notFoundCount; };
			void setNotFoundCount(int newNotFoundCount) {notFoundCount = newNotFoundCount; };

	       	cv::Point getCenter() {return center; };
			void setCenter(cv::Point x);

			void clearVectors()
			{
				contours.clear();
				balls.clear();
				ballsBox.clear();
			}

		void contoursFiltering(cv::Mat& rangeRes);
		void detectedBalls(cv::Mat& res, double dT);
		void detectedBallsResult(cv::Mat& res);
		void updateFilter();
		void showCenterPosition(cv::Mat& res, int x, int y);
		void showStatistics(cv::Mat& res, int founded, int all, int x, int y);
	};

    cv::Mat transformToHSV(cv::Mat& image);
    void circlesDetection(cv::Mat& hueImage, cv::Mat& image);
}
