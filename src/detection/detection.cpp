#include "detection/detection.hpp"

namespace detection
{
    FoundBallsState::FoundBallsState(double ticks, bool foundball, int notFoundCount)
        : ticks(ticks), foundball(foundball), notFoundCount(notFoundCount)
    {
        int stateSize = 6;
        int measSize = 4;
        int contrSize = 0;

        unsigned int type = CV_32F;

        state = cv::Mat(stateSize, 1, type);
        meas = cv::Mat(measSize, 1, type);

        cv::KalmanFilter kf(stateSize, measSize, contrSize, type);

        cv::setIdentity(kf.transitionMatrix);

        kf.measurementMatrix = cv::Mat::zeros(measSize, stateSize, type);
        kf.measurementMatrix.at<float>(0) = 1.0f;
        kf.measurementMatrix.at<float>(7) = 1.0f;
        kf.measurementMatrix.at<float>(16) = 1.0f;
        kf.measurementMatrix.at<float>(23) = 1.0f;

        kf.processNoiseCov.at<float>(0) = 1e-2f;
        kf.processNoiseCov.at<float>(7) = 1e-2f;
        kf.processNoiseCov.at<float>(14) = 5.0f;
        kf.processNoiseCov.at<float>(21) = 5.0f;
        kf.processNoiseCov.at<float>(28) = 1e-2f;
        kf.processNoiseCov.at<float>(35) = 1e-2f;

        cv::setIdentity(kf.measurementNoiseCov, cv::Scalar(1e-1f));
        detection::FoundBallsState::kalmanFilter = kf;
    }

    void FoundBallsState::contoursFiltering(cv::Mat& rangeRes)
    {
        cv::findContours(rangeRes, contours, CV_RETR_EXTERNAL,
            CV_CHAIN_APPROX_NONE);
        for (size_t i = 0; i < contours.size(); i++)
        {
            cv::Rect bBox;
            bBox = cv::boundingRect(contours[i]);

            float ratio = (float)bBox.width / (float)bBox.height;
            if (ratio > 1.0f)
                ratio = 1.0f / ratio;

            if (ratio > 0.75 && bBox.area() >= 100)
            {
                balls.push_back(contours[i]);
                ballsBox.push_back(bBox);
            }
        }
    }

    void FoundBallsState::setCenter(cv::Point x)
    {
        center = x;
    }

    void FoundBallsState::detectedBalls(cv::Mat& res, double dT)
    {
        kalmanFilter.transitionMatrix.at<float>(2) = static_cast<float>(dT);
        kalmanFilter.transitionMatrix.at<float>(9) = static_cast<float>(dT);

        state = kalmanFilter.predict();

        cv::Rect predRect;
        predRect.width = static_cast<int>(state.at<float>(4));
        predRect.height = static_cast<int>(state.at<float>(5));
        predRect.x = static_cast<int>(state.at<float>(0) - predRect.width / 2);
        predRect.y = static_cast<int>(state.at<float>(1) - predRect.height / 2);

        cv::Point center;
        center.x = static_cast<int>(state.at<float>(0));
        center.y = static_cast<int>(state.at<float>(1));
        setCenter(center);
        cv::circle(res, center, 2, CV_RGB(255, 0, 0), -1);
        cv::rectangle(res, predRect, CV_RGB(255, 0, 0), 2);
    }

    void FoundBallsState::showCenterPosition(cv::Mat& res, int x, int y)
    {
        std::string xOfCenter = std::to_string(center.x);
        std::string yOfCenter = std::to_string(center.y);
        std::string tmp = '(' + xOfCenter + ", " + yOfCenter + ')';
        cv::putText(res, tmp, cv::Point(x, y), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0,
            cv::Scalar(255, 255, 255), 1, CV_AA);
    }

    void FoundBallsState::showStatistics(cv::Mat& res, int founded, int all, int x, int y)
    {
        int tmp = static_cast<int>(founded*1.0 / (all*1.0) * 100);
        std::string t = std::to_string(tmp);
        std::string result = t + '%';

        cv::putText(res, result, cv::Point(x, y), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0,
            cv::Scalar(255, 255, 255), 1, CV_AA);
    }

    void FoundBallsState::detectedBallsResult(cv::Mat& res)
    {
        for (size_t i = 0; i < balls.size(); i++)
        {
            cv::drawContours(res, balls, static_cast<int>(i), CV_RGB(20, 150, 20), 1);
            cv::rectangle(res, ballsBox[i], CV_RGB(0, 255, 0), 2);

            cv::Point c;
            c.x = ballsBox[i].x + ballsBox[i].width / 2;
            c.y = ballsBox[i].y + ballsBox[i].height / 2;
            setCenter(c);
            cv::circle(res, center, 2, CV_RGB(20, 150, 20), -1);
        }
    }


    void FoundBallsState::updateFilter()
    {
        if (balls.size() == 0)
        {
            setNotFoundCount(getNotFoundCount() + 1);
            if (getNotFoundCount() >= 10)
            {
                setFoundball(false);
            }
        }
        else
        {
            setNotFoundCount(0);

            meas.at<float>(0) = ballsBox[0].x + ballsBox[0].width / 2.0f;
            meas.at<float>(1) = ballsBox[0].y + ballsBox[0].height / 2.0f;
            meas.at<float>(2) = static_cast<float>(ballsBox[0].width);
            meas.at<float>(3) = static_cast<float>(ballsBox[0].height);

            if (!getFoundball())
            {
                kalmanFilter.errorCovPre.at<float>(0) = 1;
                kalmanFilter.errorCovPre.at<float>(7) = 1;
                kalmanFilter.errorCovPre.at<float>(14) = 1;
                kalmanFilter.errorCovPre.at<float>(21) = 1;
                kalmanFilter.errorCovPre.at<float>(28) = 1;
                kalmanFilter.errorCovPre.at<float>(35) = 1;

                state.at<float>(0) = meas.at<float>(0);
                state.at<float>(1) = meas.at<float>(1);
                state.at<float>(2) = 0;
                state.at<float>(3) = 0;
                state.at<float>(4) = meas.at<float>(2);
                state.at<float>(5) = meas.at<float>(3);

                kalmanFilter.statePost = state;

                setFoundball(true);
            }
            else
                kalmanFilter.correct(meas);
        }
    }

    cv::Mat transformToHSV(cv::Mat& image)
    {
        cv::Mat hsvImage;
        cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);
        cv::Mat lowerHueRange;
        cv::Mat upperHueRange;
        cv::inRange(hsvImage, cv::Scalar(5, 121, 102), cv::Scalar(21, 255, 203), lowerHueRange);
        cv::inRange(hsvImage, cv::Scalar(5, 121, 102), cv::Scalar(21, 255, 203), upperHueRange);

        cv::Mat hueImage;
        cv::addWeighted(lowerHueRange, 1.0, upperHueRange, 1.0, 0.0, hueImage);

        cv::erode(hueImage, hueImage, cv::Mat(), cv::Point(-1, -1), 2);
        cv::dilate(hueImage, hueImage, cv::Mat(), cv::Point(-1, -1), 2);

        cv::GaussianBlur(hueImage, hueImage, cv::Size(9, 9), 2, 2);
        return hueImage;
    }

    void circlesDetection(cv::Mat& hueImage, cv::Mat& image)
    {
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(hueImage, circles, CV_HOUGH_GRADIENT,
            1, hueImage.rows / 8, 100, 20, 0, 0);

        for (size_t currentCircle = 0; currentCircle < circles.size(); ++currentCircle)
        {
            cv::Point center(static_cast<int>(std::round(circles[currentCircle][0])),
                             static_cast<int>(std::round(circles[currentCircle][1])));
            int radius = static_cast<int>(std::round(circles[currentCircle][2]));
            cv::circle(image, center, radius, cv::Scalar(0, 255, 0), 5);
        }
    }
}
