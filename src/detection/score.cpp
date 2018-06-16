#include "detection/score.hpp"

namespace detection
{
    bool ScoreCounter::isBallOutOfTable(const cv::Point &lastPosition)
    {
        return lastPosition.x < 60.0 || lastPosition.y < 0.0 || lastPosition.x > tableSize.x - 60.0 ||
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
            if (lastPosition.y > (tableSize.y / 3) && lastPosition.y < tableSize.y) 
            {
                if (lastPosition.x < 60) {
                    lastEvent = LastEventType::EV_GOOL_RIGHT;
                } else if (lastPosition.x > tableSize.x - 60) {
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
} // namespace detection
