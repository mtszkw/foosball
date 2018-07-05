#include "detection/detection.hpp"

void detection::detectPlayers(bool detectionEnabled, bool debugMode, Mode mode,
                              PlayersFinder& playersFinder, cv::Mat& frame, cv::Mat& restul)
{
	const string title = mode == detection::Mode::BLUE_PLAYERS ?
        "Blue players detection frame" : "Red players detection frame";

	if(detectionEnabled)
    {
		cv::Mat hsvPlayerFrameBlue = detection::transformToHSV(frame, mode);
		playersFinder.contoursFiltering(hsvPlayerFrameBlue);
		playersFinder.detectedPlayersResult(restul, mode);

		if(debugMode)
		{
			cv::imshow(title, hsvPlayerFrameBlue);	
		}
		else try { cv::destroyWindow(title); } catch(...) {}
	}
	else try { cv::destroyWindow(title); } catch(...){  }
}

void detection::trackBall(bool trackingEnabled, bool debugMode,
                          FoundBallsState& foundBallsState, double deltaTicks,
                          int& founded, int& counter, cv::Mat& frame, cv::Mat& nextFrame, cv::Mat& restul)
{
	const string title = "Tracking ball frame";

	if(trackingEnabled)
    {
		if (foundBallsState.getFoundball())
		{
			foundBallsState.detectedBalls(restul, deltaTicks);
		}

		cv::Mat rangeRes = detection::transformToHSV(frame, detection::Mode::BALL);
		cv::Mat rangeRes2 = detection::transformToHSV(nextFrame, detection::Mode::BALL);
		cv::Mat trackingFrame = detection::tracking(rangeRes, rangeRes2);
	
        foundBallsState.contoursFiltering(trackingFrame);
		foundBallsState.detectedBallsResult(restul);
		foundBallsState.updateFilter();
		
        if(debugMode) cv::imshow(title, trackingFrame);
		else try { cv::destroyWindow(title); } catch(...){}
	
		if (foundBallsState.balls.size()) founded++;
		counter++;
	}
	else try { cv::destroyWindow(title); } catch(...){}
}

cv::Mat detection::tracking(cv::Mat image1, cv::Mat image2)
{
	cv::Mat result;
	cv::absdiff(image1, image2, result);
	threshold(result, result, 5, 255, cv::THRESH_BINARY);
	blur(result, result, cv::Size(15, 15));
	threshold(result, result, 5, 255, cv::THRESH_BINARY);
	cv::bitwise_or(image1, result, result);
	cv::bitwise_or(image2, result, result);
	threshold(result, result, 5, 255, cv::THRESH_BINARY);
	return result;
}

cv::Scalar detection::getColorForMode(detection::Mode mode, int colorIndex)
{
	if(mode == detection::Mode::BALL)
		if(colorIndex == 0)
			return cv::Scalar(11, 120, 160);
		else return cv::Scalar(29, 255, 203);
	if(mode == detection::Mode::BLUE_PLAYERS)
		if(colorIndex == 0)
			return cv::Scalar(90, 50, 50);
		else return cv::Scalar(130, 255, 255);
	if(mode == detection::Mode::RED_PLAYERS)
		if(colorIndex == 0)
			return cv::Scalar(160, 20, 20);
		else return cv::Scalar(200, 255, 255);
}

cv::Mat detection::getMaskForMode(Mode mode, cv::Size size)
{
	cv::Mat mask = cv::Mat::zeros( size, CV_8UC1 );
	bitwise_not(mask, mask);
	if(mode != Mode::BALL)
	{
		mask(cv::Range(0, mask.rows), cv::Range(0, 9 * mask.cols / 240 )) = 0;
		mask(cv::Range(0, mask.rows / 3), cv::Range( 9 * mask.cols / 240 , 33 * mask.cols / 240)) = 0;
		mask(cv::Range(41 * mask.rows / 60, mask.rows), cv::Range( 9 * mask.cols / 240,  33 * mask.cols / 240)) = 0;
		mask(cv::Range(0, mask.rows), cv::Range(33 * mask.cols / 240, mask.cols / 6)) = 0;
		mask(cv::Range(0, mask.rows), cv::Range(mask.cols / 4, 9 * mask.cols / 24)) = 0;
		mask(cv::Range(0, mask.rows), cv::Range(29 * mask.cols / 60, 73 * mask.cols / 120)) = 0;
		mask(cv::Range(0, mask.rows), cv::Range(3 * mask.cols / 4, mask.cols)) = 0;
		if(mode == Mode::RED_PLAYERS)
		{
			cv::Mat flipped;
			cv::flip(mask, flipped, 1);
			mask = flipped;
		}
	}
	return mask;
}

cv::Mat detection::transformToHSV(cv::Mat image, Mode mode)
{
	cv::Mat hsvImage;
	cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);
	cv::Mat maskedHSVImage;
	hsvImage.copyTo(maskedHSVImage, getMaskForMode(mode, cv::Size(hsvImage.cols, hsvImage.rows)));
	cv::Mat lowerHueRange;
	cv::Mat upperHueRange;
	cv::inRange(maskedHSVImage, getColorForMode(mode, 0), getColorForMode(mode, 1), lowerHueRange);
	cv::inRange(maskedHSVImage, getColorForMode(mode, 0), getColorForMode(mode, 1), upperHueRange);

	cv::Mat hueImage;
	cv::addWeighted(lowerHueRange, 1.0, upperHueRange, 1.0, 0.0, hueImage);
    cv::erode(hueImage, hueImage, cv::Mat(), cv::Point(-1, -1), 2);
    cv::dilate(hueImage, hueImage, cv::Mat(), cv::Point(-1, -1), 2);
	cv::GaussianBlur(hueImage, hueImage, cv::Size(9, 9), 2, 2);
	return hueImage;
}

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
       	             CV_CHAIN_APPROX_SIMPLE);
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

void detection::FoundBallsState::setCenter(cv::Point x)
{
	center = x;
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
	setCenter(center);
    cv::circle(res, center, 2, CV_RGB(255,0,255), -1);
    cv::rectangle(res, predRect, CV_RGB(255,0,255), 2);
}

void detection::FoundBallsState::detectedBallsResult(cv::Mat& res)
{
	for (size_t i = 0; i < balls.size(); i++)
   	{
       	cv::drawContours(res, balls, i, CV_RGB(20,150,20), 1);
       	cv::rectangle(res, ballsBox[i], CV_RGB(0,255,0), 2);

		cv::Point c;
		c.x = ballsBox[i].x + ballsBox[i].width / 2;
       	c.y = ballsBox[i].y + ballsBox[i].height / 2;
		setCenter(c);
       	cv::circle(res, center, 2, CV_RGB(20,150,20), -1);
   	}
}


void detection::FoundBallsState::updateFilter() 
{
    if (balls.size() == 0)
    {
    	setNotFoundCount(getNotFoundCount() + 1);
    	if( getNotFoundCount() >= 10 )
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

void detection::PlayersFinder::contoursFiltering(cv::Mat& rangeRes)
{
    cv::findContours(rangeRes, players, CV_RETR_EXTERNAL,
       	             CV_CHAIN_APPROX_NONE);
   	for (size_t i = 0; i < players.size(); i++)
   	{
       	cv::Rect bBox;
       	bBox = cv::boundingRect(players[i]);
        playersBox.push_back(bBox);
	}
}

void detection::PlayersFinder::detectedPlayersResult(cv::Mat& res, Mode mode)
{
	for (size_t i = 0; i < players.size(); i++)
   	{	
		if(mode == Mode::BLUE_PLAYERS)
		{
			cv::drawContours(res, players, i, CV_RGB(100, 100, 255), 1);
       		cv::rectangle(res, playersBox[i], CV_RGB(0, 0, 255), 2);
		}
		else{
       		cv::drawContours(res, players, i, CV_RGB(255, 100, 100), 1);
       		cv::rectangle(res, playersBox[i], CV_RGB(255, 0, 0), 2);
		}
   	}
}
