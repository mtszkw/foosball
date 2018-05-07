#include "detection.hpp"

int main(int argc, char **argv) 
{
    cv::VideoCapture video(argv[1]);
    cv::Mat frame;
    
    while(true)
    {
	video >> frame;
	
	cv::Mat hueImage = detection::transformToHSV(frame);
	detection::circlesDetection(hueImage, frame);
	
	cv::imshow("Detected circles", frame);
	
	if (cv::waitKey(30) == 27)
	    break;
    }

    return 0;
}