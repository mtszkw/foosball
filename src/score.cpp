#include "score.hpp"

namespace score
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
        if (lastPosition.y > 200 && lastPosition.y < 400) 
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
    ss << "Left " << getScoreLeft() << " - " << getScoreRight() 
        << " Right. Number of outs: " << getScoreOuts();
    cv::putText(res, ss.str(), cv::Point(x, y), cv::FONT_HERSHEY_COMPLEX_SMALL, 
        1.0, cv::Scalar(255, 255, 255), 1, CV_AA);
}

} // namespace score
