#ifndef ARUCO_H_
#define ARUCO_H_

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <vector>

struct ArucoMarker 
{
    int id;
    std::vector<cv::Point2f> corners;

    ArucoMarker(int id, const std::vector<cv::Point2f> &corners)
        : id(id), corners(corners) {};

    const static int INVALID_ID = -1;
};

void detectArucoOnFrame(cv::Mat &frame, cv::Ptr<cv::aruco::Dictionary> arucoDictionary,
    std::vector<ArucoMarker> &found, std::vector<ArucoMarker> &rejected,
    cv::Ptr<cv::aruco::DetectorParameters> detectorParameters);
void drawMarkersOnFrame(cv::Mat &frame, const std::vector<ArucoMarker> &markers);

#endif