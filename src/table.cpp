#include "table.hpp"

namespace table {
    static double euclideanDistance2(const cv::Point2f &a, const cv::Point2f &b) 
    {
        cv::Point2f r = a - b;
        return r.x*r.x + r.y*r.y;    
    }


    void Table::updateTableOnFrame(const std::vector<aruco::ArucoMarker> &arucoMarkers) 
    {
        for (const aruco::ArucoMarker &marker : arucoMarkers)
            corners[marker.getId()] = marker.getMiddle();

        std::array<int, 4> indexes = { 0, 1, 2, 3 };
        for (const aruco::ArucoMarker &marker : arucoMarkers) 
        {
            int markerId = marker.getId();
            int markerNextId = (markerId + 1) % 4;
            int markerPrevId = (markerId + 3) % 4;

            int bestCornerId = *std::min_element(indexes.begin(), indexes.end(), 
                [markerId, markerNextId, markerPrevId, this, marker] (int a, int b) -> bool {
                double distanceFromPrevA = euclideanDistance2(corners[markerPrevId],
                    marker.getCorners()[a]);
                double distanceFromNextA = euclideanDistance2(corners[markerNextId],
                    marker.getCorners()[a]);
                double distanceFromPrevB = euclideanDistance2(corners[markerPrevId],
                    marker.getCorners()[b]);
                double distanceFromNextB = euclideanDistance2(corners[markerNextId],
                    marker.getCorners()[b]);

                
                return markerId % 2 ? 
                    (distanceFromNextA - distanceFromPrevA <
                    distanceFromNextB - distanceFromPrevB) :
                    (distanceFromPrevA - distanceFromNextA < 
                    distanceFromPrevB - distanceFromNextB);
            });

            corners[markerId] = marker.getCorners()[bestCornerId];
        }

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