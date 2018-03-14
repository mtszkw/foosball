#include "aruco.hpp"

static void _drawMarkerOnFrame(cv::Mat &frame, const ArucoMarker &marker) 
{
    auto color = (marker.id == ArucoMarker::INVALID_ID ? CV_RGB(255, 0, 0) : CV_RGB(0, 255, 0));


    cv::line(frame, marker.corners[0], marker.corners[1], color);
    cv::line(frame, marker.corners[2], marker.corners[1], color);
    cv::line(frame, marker.corners[2], marker.corners[3], color);
    cv::line(frame, marker.corners[0], marker.corners[3], color);
}

void detectArucoOnFrame(cv::Mat &frame, cv::Ptr<cv::aruco::Dictionary> arucoDictionary,
    std::vector<ArucoMarker> &found, std::vector<ArucoMarker> &rejected,
    cv::Ptr<cv::aruco::DetectorParameters> detectorParameters)
{
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners;
    std::vector<std::vector<cv::Point2f>> markerRejected;

    found.clear();
    rejected.clear();
    cv::aruco::detectMarkers(frame, arucoDictionary, markerCorners, 
        markerIds, detectorParameters, markerRejected);

    found.reserve(markerIds.size());
    for (int i = 0; i < markerIds.size(); ++i)
        found.push_back(ArucoMarker(markerIds[i], markerCorners[i]));

    rejected.reserve(markerRejected.size());
    for (int i = 0; i < markerRejected.size(); ++i)
        found.push_back(ArucoMarker(ArucoMarker::INVALID_ID, markerRejected[i]));
}

void drawMarkersOnFrame(cv::Mat &frame, const std::vector<ArucoMarker> &markers)
{
    for (ArucoMarker marker : markers)
        _drawMarkerOnFrame(frame, marker);
}