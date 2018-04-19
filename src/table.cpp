#include "table.hpp"

namespace table {
    void Table::updateTableOnFrame(const std::vector<aruco::ArucoMarker> &arucoMarkers) 
    {
        for (const aruco::ArucoMarker &marker : arucoMarkers)
            corners[marker.getId()] = marker.getMiddle();

        if (arucoMarkers.size() == 4)
            transformationMatrix = cv::getPerspectiveTransform(corners.data(), output);
    }

    void Table::drawTableOnFrame(cv::Mat &frame) 
    {
        for (int i = 0; i < 4; ++i) 
        {
            cv::line(frame, corners[i], corners[(i+1)%4], CV_RGB(0, 255, 0));
        }
    }

    cv::Mat Table::getTableFromFrame(const cv::Mat &frame)
    {
        cv::Mat result;

        cv::warpPerspective(frame, result, transformationMatrix, output_size);

        return result;
    }
}