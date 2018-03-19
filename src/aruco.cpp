#include "aruco.hpp"

static void _drawMarkerOnFrame(cv::Mat &frame, const ArucoMarker &marker) 
{
    auto color = (marker.id == ArucoMarker::INVALID_ID ? CV_RGB(255, 0, 0) : CV_RGB(0, 255, 0));


    cv::line(frame, marker.corners[0], marker.corners[1], color);
    cv::line(frame, marker.corners[2], marker.corners[1], color);
    cv::line(frame, marker.corners[2], marker.corners[3], color);
    cv::line(frame, marker.corners[0], marker.corners[3], color);
}

/* 
 * Little explanation here. This procedure create aruco dictionary from given path.
 * Under that path it expects a png file with n x mn pixels, where n is size of aruco with
 * black frame (eg. 7 for aruco with 5 points) and m is the number of markers in dictionary.
 * There is no error checking so make sure this file is correct.
 * Also file should be rbg with black (0, 0, 0) and white (255, 255, 255) pixels.
 * 
 * Cancer. Just like OpenCV. God forgive me.
 * */
cv::Ptr<cv::aruco::Dictionary> createDictionary(std::string path, int correction) 
{
    cv::Mat bitmap, tmp = cv::imread(path);
    cv::cvtColor(tmp, bitmap, cv::COLOR_BGR2GRAY);

    int size_x = bitmap.size().width;
    int size_y = bitmap.size().height;
    int elements = size_x / size_y;
    
    cv::Ptr<cv::aruco::Dictionary> arucoDictionary(new cv::aruco::Dictionary());
    arucoDictionary.get()->markerSize = size_y - 2;
    arucoDictionary.get()->maxCorrectionBits = correction;

    bitmap.forEach<uint8_t>([&](uint8_t& x, const int * pos) -> void { x /= 255; });

    for (int i = 0; i < elements; ++i)
    {
        int current_x = 1 + i * size_y;
        arucoDictionary.get()->bytesList.push_back(
            cv::aruco::Dictionary::getByteListFromBits(
                bitmap(cv::Rect(
                    current_x, 
                    1, 
                    arucoDictionary.get()->markerSize,
                    arucoDictionary.get()->markerSize
        ))));
    }

    return arucoDictionary;
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

