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
            bool transformationValid;

            const cv::Point2f output[4];
            const cv::Size output_size;
        public:
            Table(int width, int height) : corners(4), output_size({width, height}),
                output({{(float)width, 0}, {(float)width, (float)height}, {0, (float)height}, {0, 0}}),
                transformationValid(false) {};
            void updateTableOnFrame(const std::vector<aruco::ArucoMarker> &arucoMarkers);
            void drawTableOnFrame(cv::Mat &frame);
            cv::Mat getTableFromFrame(const cv::Mat &frame);
    };
}