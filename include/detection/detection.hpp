#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

namespace detection
{
	enum Mode {
    	BALL = 0,
    	BLUE_PLAYERS,
    	RED_PLAYERS
	};

	cv::Scalar getColorForMode(detection::Mode mode, int colorIndex);
	cv::Mat getMaskForMode(Mode mode, cv::Size size);
    cv::Mat transformToHSV(cv::Mat image, Mode mode);
	cv::Mat tracking(cv::Mat image1, cv::Mat image2);

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

		double getTicks() { return ticks; }
		void setTicks(double newTicks) { ticks = newTicks; }

		cv::Point getCenter() const { return center; }

		bool getFoundball() {return foundball; }
		void setFoundball(bool newFoundball) {foundball = newFoundball; }

		int getNotFoundCount() {return notFoundCount; }
		void setNotFoundCount(int newNotFoundCount) {notFoundCount = newNotFoundCount; }

	    cv::Point getCenter() {return center; }
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
	};

	class PlayersFinder
	{		
	public:
        vector<vector<cv::Point> > players;
        vector<cv::Rect> playersBox;

		PlayersFinder() {}

		void clearVectors()
		{
			players.clear();
			playersBox.clear();
		}

		void contoursFiltering(cv::Mat& rangeRes);
		void detectedPlayersResult(cv::Mat& res, Mode mode);
	};
	
	void detectPlayers(bool detectionEnabled, bool debugMode, Mode mode,
        PlayersFinder& playersFinder, cv::Mat& frame, cv::Mat& restul);

	void trackBall(bool trackingEnabled, bool debugMode, FoundBallsState& foundBallsState,
        double deltaTicks, int& founded, int& counter, cv::Mat& frame, cv::Mat& nextFrame, cv::Mat& restul);
} // namespace detection
