#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

namespace detection
{
	enum Mode {
	BALL = 0,
	BLUE_PLAYERS,
	RED_PLAYERS
	};

    cv::Mat transformToHSV(cv::Mat image, Mode mode);

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
			
			std::vector<std::vector<cv::Point> > contours;
        	std::vector<std::vector<cv::Point> > balls;
        	std::vector<cv::Rect> ballsBox;

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

	class PlayersFinder
	{		
		public:
        	std::vector<std::vector<cv::Point> > players;
        	std::vector<cv::Rect> playersBox;

			PlayersFinder() {};

			void clearVectors()
			{
				players.clear();
				playersBox.clear();
			}

			void contoursFiltering(cv::Mat& rangeRes);
			void detectedPlayersResult(cv::Mat& res, Mode mode);
	};
}