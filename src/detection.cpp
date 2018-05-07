#include "detection.hpp"

detection::FoundBallsState::FoundBallsState(double ticks, bool foundball, int notFoundCount) 
				: ticks(ticks), foundball(foundball), notFoundCount(notFoundCount)
{
	int stateSize = 6;
	int measSize = 4;
	int contrSize = 0;
	    
	unsigned int type = CV_32F;

	state = cv::Mat(stateSize, 1, type);  
	meas = cv::Mat(measSize, 1, type);  

	cv::KalmanFilter kf(stateSize, measSize, contrSize, type);  

	cv::setIdentity(kf.transitionMatrix);

	kf.measurementMatrix = cv::Mat::zeros(measSize, stateSize, type);
	kf.measurementMatrix.at<float>(0) = 1.0f;
	kf.measurementMatrix.at<float>(7) = 1.0f;
	kf.measurementMatrix.at<float>(16) = 1.0f;
	kf.measurementMatrix.at<float>(23) = 1.0f;

	kf.processNoiseCov.at<float>(0) = 1e-2;
	kf.processNoiseCov.at<float>(7) = 1e-2;
	kf.processNoiseCov.at<float>(14) = 5.0f;
	kf.processNoiseCov.at<float>(21) = 5.0f;
	kf.processNoiseCov.at<float>(28) = 1e-2;
	kf.processNoiseCov.at<float>(35) = 1e-2;

	cv::setIdentity(kf.measurementNoiseCov, cv::Scalar(1e-1));
	detection::FoundBallsState::kalmanFilter = kf;
}

void detection::FoundBallsState::contoursFiltering(cv::Mat& rangeRes)
{
    cv::findContours(rangeRes, contours, CV_RETR_EXTERNAL,
       	             CV_CHAIN_APPROX_NONE);
   	for (size_t i = 0; i < contours.size(); i++)
   	{
       	cv::Rect bBox;
       	bBox = cv::boundingRect(contours[i]);

        float ratio = (float) bBox.width / (float) bBox.height;
   	    if (ratio > 1.0f)
       	    ratio = 1.0f / ratio;

        if(ratio > 0.75 && bBox.area() >= 100)
        {
            balls.push_back(contours[i]);
            ballsBox.push_back(bBox);            
        }           
	}
}

void detection::FoundBallsState::detectedBalls(cv::Mat& res, double dT)
{
    kalmanFilter.transitionMatrix.at<float>(2) = dT;
    kalmanFilter.transitionMatrix.at<float>(9) = dT;
            
    state = kalmanFilter.predict();
            
    cv::Rect predRect;
    predRect.width = state.at<float>(4);
    predRect.height = state.at<float>(5);
    predRect.x = state.at<float>(0) - predRect.width / 2;
    predRect.y = state.at<float>(1) - predRect.height / 2;

    cv::Point center;
    center.x = state.at<float>(0);
    center.y = state.at<float>(1);
    cv::circle(res, center, 2, CV_RGB(255,0,0), -1);
    cv::rectangle(res, predRect, CV_RGB(255,0,0), 2);
}

void detection::FoundBallsState::detectedBallsResult(cv::Mat& res)
{
	for (size_t i = 0; i < balls.size(); i++)
   	{
       	cv::drawContours(res, balls, i, CV_RGB(20,150,20), 1);
       	cv::rectangle(res, ballsBox[i], CV_RGB(0,255,0), 2);

       	cv::Point center;
       	center.x = ballsBox[i].x + ballsBox[i].width / 2;
       	center.y = ballsBox[i].y + ballsBox[i].height / 2;
       	cv::circle(res, center, 2, CV_RGB(20,150,20), -1);
   	}
}


void detection::FoundBallsState::updateFilter() 
{
    if (balls.size() == 0)
    {
    	setNotFoundCount(getNotFoundCount() + 1);
    	if( getNotFoundCount() >= 100 )
    	{
       		setFoundball(false);
    	}
    }
    else
    {
    	setNotFoundCount(0);

    	meas.at<float>(0) = ballsBox[0].x + ballsBox[0].width / 2;
        meas.at<float>(1) = ballsBox[0].y + ballsBox[0].height / 2;
        meas.at<float>(2) = (float)ballsBox[0].width;
        meas.at<float>(3) = (float)ballsBox[0].height;

        if (!getFoundball())
        {
			kalmanFilter.errorCovPre.at<float>(0) = 1; 
			kalmanFilter.errorCovPre.at<float>(7) = 1; 
			kalmanFilter.errorCovPre.at<float>(14) = 1;
			kalmanFilter.errorCovPre.at<float>(21) = 1;
			kalmanFilter.errorCovPre.at<float>(28) = 1; 
			kalmanFilter.errorCovPre.at<float>(35) = 1; 

			state.at<float>(0) = meas.at<float>(0);
			state.at<float>(1) = meas.at<float>(1);
			state.at<float>(2) = 0;
			state.at<float>(3) = 0;
			state.at<float>(4) = meas.at<float>(2);
			state.at<float>(5) = meas.at<float>(3);

			kalmanFilter.statePost = state;
			
			setFoundball(true);
		}
		else
			kalmanFilter.correct(meas);
	}
}

cv::Mat detection::transformToHSV(cv::Mat& image)
{
	cv::Mat hsvImage;
	cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);
	cv::Mat lowerHueRange;
	cv::Mat upperHueRange;
	cv::inRange(hsvImage, cv::Scalar(5, 121, 102), cv::Scalar(21, 255, 203), lowerHueRange);
	cv::inRange(hsvImage, cv::Scalar(5, 121, 102), cv::Scalar(21, 255, 203), upperHueRange);

	cv::Mat hueImage;
	cv::addWeighted(lowerHueRange, 1.0, upperHueRange, 1.0, 0.0, hueImage);

    cv::erode(hueImage, hueImage, cv::Mat(), cv::Point(-1, -1), 2);
    cv::dilate(hueImage, hueImage, cv::Mat(), cv::Point(-1, -1), 2);

	cv::GaussianBlur(hueImage, hueImage, cv::Size(9, 9), 2, 2);
	return hueImage;
}

void detection::circlesDetection(cv::Mat& hueImage, cv::Mat& image )
{
	std::vector<cv::Vec3f> circles;
    cv::HoughCircles(hueImage, circles, CV_HOUGH_GRADIENT,
					 1, hueImage.rows/8, 100, 20, 0, 0);

	for(size_t currentCircle = 0; currentCircle < circles.size(); ++currentCircle)
	{
	   	cv::Point center(std::round(circles[currentCircle][0]),
	        		     std::round(circles[currentCircle][1]));
	    int radius = std::round(circles[currentCircle][2]);
	    cv::circle(image, center, radius, cv::Scalar(0, 255, 0), 5);
	}
}