#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

using namespace std;

namespace aruco
{
    class ArucoMarker 
    {
        int id;
        vector<cv::Point2f> corners;

        public:
            ArucoMarker(int id, const vector<cv::Point2f> &corners) : id(id), corners(corners) {}
            
            bool isValid() const
            {
                return id != INVALID_ID;
            }

            int getId() const
            {
                return id;
            }

            const vector<cv::Point2f> &getCorners() const
            {
                return corners;
            }

            const cv::Point2f getMiddle() const;

            const static int INVALID_ID = -1;
    };

    cv::Ptr<cv::aruco::Dictionary> createDictionary(string path, int correction);

    cv::Ptr<cv::aruco::DetectorParameters> loadParametersFromFile(string path = "");

    void detectArucoOnFrame(cv::Mat &frame, cv::Ptr<cv::aruco::Dictionary> arucoDictionary,
                            vector<ArucoMarker> &found, vector<ArucoMarker> &rejected,
                            cv::Ptr<cv::aruco::DetectorParameters> detectorParameters);

    void drawMarkersOnFrame(cv::Mat &frame, const vector<ArucoMarker> &markers);
}
