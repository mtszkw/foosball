#include "detection/score.hpp"

namespace detection
{
    bool ScoreCounter::isBallOutOfTable(const cv::Point &lastPosition)
    {
        return lastPosition.x < 0.0 || lastPosition.y < 0.0 || lastPosition.x > tableSize.x ||
               lastPosition.y > tableSize.y;
    }

    void ScoreCounter::confirmLastEvent() {
        switch (lastEvent)
        {
            case LastEventType::EV_OUT:
                ++scoreOuts; break;
            case LastEventType::EV_GOOL_LEFT:
                ++scoreLeft; break;
            case LastEventType::EV_GOOL_RIGHT:
                ++scoreRight; break;
            default:
                return;
        }
        lastEvent = LastEventType::EV_NONE;
    }

    void ScoreCounter::trackBallAndScore(const cv::Point &lastPosition, bool isValid)
    {
        if (!clearFlag && isValid) {
            clearFlag = true;
        }

        if (!isValid && clearFlag && isBallOutOfTable(lastPosition))
        {
            if (lastPosition.y > (tableSize.y / 3) && lastPosition.y < (2 * tableSize.y / 3)) 
            {
                if (lastPosition.x < 0) {
                    lastEvent = LastEventType::EV_GOOL_RIGHT;
                } else if (lastPosition.x > tableSize.x) {
                    lastEvent = LastEventType::EV_GOOL_LEFT;
                }

            }
            else
            {
                lastEvent = LastEventType::EV_OUT;
            }
            notFoundSince = 0;
            clearFlag = false;
        } 
        else if (!isValid) 
        {
            if(++notFoundSince == fps)
                confirmLastEvent();
        }
    }

    void ScoreCounter::printScoreBoard(cv::Mat &res, int x, int y)
    {
        std::stringstream ss;
        ss << "Game score: " << getScoreLeft() << " - "
           << getScoreRight() << " (outs: " << getScoreOuts() << ')';
        cv::putText(res, ss.str(), cv::Point(x, y), cv::FONT_HERSHEY_DUPLEX,
            0.55, cv::Scalar(255, 255, 255), 1, CV_AA);
    }
} // namespace detection
