#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

#include "aruco/aruco.hpp"

namespace detection
{
    class Table 
    {
    private:
        std::vector<cv::Point2f> corners;
        cv::Mat transformationMatrix;
        bool transformationValid;

        cv::Point2f output[4];
        const cv::Size output_size;

    public:
        Table(int width, int height)
            : corners(4), output_size(width, height), transformationValid(false)
        {
            output[0] = { (float)width, 0 };
            output[1] = { (float)width, (float)height };
            output[2] = { 0, (float)height };
            output[3] = { 0, 0 };
        }

        void updateTableOnFrame(const std::vector<aruco::ArucoMarker> &arucoMarkers);
        void drawTableOnFrame(cv::Mat &frame);
        cv::Mat getTableFromFrame(const cv::Mat &frame);
        const cv::Point getSize() const { return (cv::Point) output_size; };
    };
}
