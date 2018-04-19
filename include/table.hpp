#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <vector>

#include "aruco.hpp"

namespace table {
    class Table 
    {
        private:
            std::vector<cv::Point2f> corners;
            cv::Mat transformationMatrix;

            const cv::Point2f output[4] = {
                {900, 0}, {900, 500}, {0, 500}, {0, 0}
            };
        public:
            Table() : corners(4) {};
            void updateTableOnFrame(const std::vector<aruco::ArucoMarker> &arucoMarkers);
            void drawTableOnFrame(cv::Mat &frame);
            cv::Mat getTableFromFrame(const cv::Mat &frame);
    };
}