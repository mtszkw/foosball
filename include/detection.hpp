#ifndef DETECTION_HPP
#define DETECTION_HPP

#include <opencv2/opencv.hpp>
#include <vector>

namespace detection 
{
    cv::Mat transformToHSV(cv::Mat* image)
    {
	// Convert input image to HSV
	cv::Mat hsvImage;
	cv::cvtColor(image[0], hsvImage, cv::COLOR_BGR2HSV);
	cv::Mat lowerHueRange;
	cv::Mat upperHueRange;
	cv::inRange(hsvImage, cv::Scalar(5, 121, 102), cv::Scalar(21, 255, 203), lowerHueRange);  
	cv::inRange(hsvImage, cv::Scalar(5, 121, 102), cv::Scalar(21, 255, 203), upperHueRange);

	// Combine the above two images
	cv::Mat hueImage;
	cv::addWeighted(lowerHueRange, 1.0, upperHueRange, 1.0, 0.0, hueImage);

	cv::GaussianBlur(hueImage, hueImage, cv::Size(9, 9), 2, 2);
	return hueImage;
	
    }
    
    
    void circlesDetection(cv::Mat* hueImage, cv::Mat* image )
    {
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(hueImage[0], circles, CV_HOUGH_GRADIENT, 
			 1, hueImage[0].rows/8, 100, 20, 0, 0);
  
	// Loop over all detected circles and outline them on the original image
	for(size_t currentCircle = 0; currentCircle < circles.size(); ++currentCircle) 
	{
	    cv::Point center(std::round(circles[currentCircle][0]), 
			     std::round(circles[currentCircle][1]));
	    int radius = std::round(circles[currentCircle][2]);
	    cv::circle(image[0], center, radius, cv::Scalar(0, 255, 0), 5);
	}
    }
    
}

#endif //DETECTION_HPP