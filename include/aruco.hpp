#ifndef ARUCO_H_
#define ARUCO_H_

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <vector>

namespace aruco {
    class ArucoMarker 
    {
        private:
            int id;
            std::vector<cv::Point2f> corners;

        public:
            ArucoMarker(int id, const std::vector<cv::Point2f> &corners)
                : id(id), corners(corners) {};
            
            // Lets do it inline
            bool isValid() const { return id != INVALID_ID; };
            int getId() const { return id; };
            const std::vector<cv::Point2f> &getCorners() const {
                return corners;
            }

            const cv::Point2f getMiddle() const;
            const static int INVALID_ID = -1;
    };

    cv::Ptr<cv::aruco::Dictionary> createDictionary(std::string path, int correction);
    void detectArucoOnFrame(cv::Mat &frame, cv::Ptr<cv::aruco::Dictionary> arucoDictionary,
        std::vector<ArucoMarker> &found, std::vector<ArucoMarker> &rejected,
        cv::Ptr<cv::aruco::DetectorParameters> detectorParameters);
    void drawMarkersOnFrame(cv::Mat &frame, const std::vector<ArucoMarker> &markers);
}

#endif