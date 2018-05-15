#ifndef DETECTION_HPP
#define DETECTION_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <vector>

namespace detection
{
	class FoundBallsState
	{
		private:
			double ticks;
    		bool foundball;
    		int notFoundCount;
		
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

    cv::Mat transformToHSV(cv::Mat& image);
    void circlesDetection(cv::Mat& hueImage, cv::Mat& image);

}

#endif //DETECTION_HPP
