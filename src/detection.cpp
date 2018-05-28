#include "detection.hpp"

cv::Scalar getColorForMode(detection::Mode mode, int colorIndex)
{
	if(mode == detection::Mode::BALL)
		if(colorIndex == 0)
			return cv::Scalar(5, 121, 102);
		else return cv::Scalar(21, 255, 203);
	if(mode == detection::Mode::BLUE_PLAYERS)
		if(colorIndex == 0)
			return cv::Scalar(90, 50, 50);
		else return cv::Scalar(130, 255, 255);
	if(mode == detection::Mode::RED_PLAYERS)
		if(colorIndex == 0)
			return cv::Scalar(160, 20, 20);
		else return cv::Scalar(200, 255, 255);
}


cv::Mat detection::transformToHSV(cv::Mat image, Mode mode)
{
	cv::Mat hsvImage;
	if(mode != Mode::BALL)
	{
		cv::Mat drawing = cv::Mat::zeros( cv::Size(1200, 600), CV_8UC1 );
		drawing(cv::Range(0, drawing.rows), cv::Range(0, 45)) = 255;
		drawing(cv::Range(0, 200), cv::Range(45, 165)) = 255;
		drawing(cv::Range(410, drawing.rows), cv::Range(45, 165)) = 255;
		drawing(cv::Range(0, drawing.rows), cv::Range(165, 200)) = 255;
		drawing(cv::Range(0, drawing.rows), cv::Range(300, 450)) = 255;
		drawing(cv::Range(0, drawing.rows), cv::Range(580, 730)) = 255;
		drawing(cv::Range(0, drawing.rows), cv::Range(900, 1200)) = 255;
		bitwise_not(drawing, drawing);
		if(mode == Mode::BLUE_PLAYERS)
		{
			cv::Mat res;
			image.copyTo(res, drawing);
			image = res;
		}
		else
		{
			cv::Mat flipped;
			cv::flip(drawing, flipped, 1);
			drawing = flipped;
			cv::Mat res;
			image.copyTo(res, drawing);
			image = res;
		}
	}

	cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);
	cv::Mat lowerHueRange;
	cv::Mat upperHueRange;
	cv::inRange(hsvImage, getColorForMode(mode, 0), getColorForMode(mode, 1), lowerHueRange);
	cv::inRange(hsvImage, getColorForMode(mode, 0), getColorForMode(mode, 1), upperHueRange);

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

void detection::FoundBallsState::showCenterPosition(cv::Mat& res,  int x, int y)
{
	std::string xOfCenter = std::to_string(center.x);
	std::string yOfCenter = std::to_string(center.y);
	std::string tmp = '(' + xOfCenter + ", " + yOfCenter + ')';
	cv::putText(res, tmp, cv::Point(x,y), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, 
				cv::Scalar(255,255,255), 1, CV_AA);
}

void detection::FoundBallsState::showStatistics(cv::Mat& res, int founded, int all, int x, int y)
{
	int tmp = founded*1.0/(all*1.0)*100;
	std::string t = std::to_string(tmp);
	std::string result = t + '%';
	        
    cv::putText(res, result, cv::Point(x,y), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0,
				 cv::Scalar(255,255,255), 1, CV_AA);
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
