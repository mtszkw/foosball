#pragma once

#include <opencv2/opencv.hpp>

namespace detection
{
    class ScoreCounter
    {
    private:
        enum class LastEventType
        {
            EV_OUT, 
            EV_NONE,
            EV_GOOL_LEFT, 
            EV_GOOL_RIGHT
        };

        int scoreLeft, scoreRight;
        int scoreOuts;
        const cv::Point tableSize;
        const int fps;
        bool clearFlag;

        LastEventType lastEvent;
        int notFoundSince;
        void confirmLastEvent();
        bool isBallOutOfTable(const cv::Point &lastPosition);

    public:
        ScoreCounter(const cv::Point &tableSize, int fps)
            : scoreLeft(0),
              scoreRight(0),
              scoreOuts(0),
              tableSize(tableSize), 
              clearFlag(false),
              fps(fps) {}

        void trackBallAndScore(const cv::Point &lastPosition, bool isValid);

        int getScoreLeft() const { return scoreLeft; }
        int getScoreRight() const { return scoreRight; }
        int getScoreOuts() const { return scoreOuts; }
    };
} // namespace score
