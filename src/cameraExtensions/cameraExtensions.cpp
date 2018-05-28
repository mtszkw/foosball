#include <opencv2/core/mat.hpp>
#include <cameraExtensions/cameraExtensions.h>
#include <opencv2/core/utility.hpp>
#include <opencv2/shape/hist_cost.hpp>
#include <calib/cameraCalibration.hpp>
#include <opencv2/video/background_segm.hpp>

cv::Mat CameraExtensions::normalizeLuminance(cv::Mat src)
{
    cv::Mat result;
    cv::cvtColor(src, result, CV_BGR2YUV);
    std::vector<cv::Mat> channels;
    cv::split(result, channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels, result);
    cv::cvtColor(result, result, CV_YUV2BGR);
    return result;
}

cv::Mat CameraExtensions::removeShadowInBlackAndWhite(cv::Mat src, bool normalize)
{
    cv::Mat rgb[3];
    cv::split(src, rgb);
    std::vector<cv::Mat> result_planes;
    std::vector<cv::Mat> result_norm_planes;
    for (cv::Mat plane : rgb)
    {
        cv::Mat dilated_img;
        cv::dilate(plane, dilated_img, cv::Mat(7, 7, CV_8UC1, cv::Scalar(1)));
        cv::Mat bg_img;
        cv::medianBlur(dilated_img, bg_img, 21);
        cv::Mat diff_img;
        cv::absdiff(plane, bg_img, diff_img);
        diff_img = 255 - diff_img;
        cv::Mat norm_img;
        cv::normalize(diff_img, norm_img, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        result_planes.push_back(diff_img);
        if(normalize)
        {
            result_norm_planes.push_back(norm_img);
        }
    }
    cv::Mat result_in_black_and_white;
    if (normalize)
    {
        cv::merge(result_norm_planes, result_in_black_and_white);
    } else
    {
        cv::merge(result_planes, result_in_black_and_white);
    }
    return result_in_black_and_white;
}


void CameraExtensions::reduceShadowEffect(std::string src)
{
    cv::Mat frame;
    cv::Mat back;
    cv::Mat fore;
    cv::VideoCapture cap(src);

    cv::Ptr<cv::BackgroundSubtractorMOG2> bg;
    bg = cv::createBackgroundSubtractorMOG2(50, 16.0, true);
//    Additional Properties
//    bg->setNMixtures(3);
//    bg->setHistory(20);
//    bg->setShadowThreshold(0.9);
//    bg->setShadowValue(0);
//    bg->setDetectShadows(true);
    std::vector<std::vector<cv::Point>> contours;

    cv::namedWindow("Frame");
    cv::namedWindow("Background");
    for (;;)
    {
        cap >> frame;
        bg->apply(frame, fore);
        bg->getBackgroundImage(back);

        cv::dilate(fore, fore, cv::Mat());
        cv::erode(fore, fore, cv::Mat());
        cv::threshold(fore, fore, 127, 255, cv::THRESH_BINARY);
        cv::findContours(fore, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        cv::drawContours(frame, contours, -1, cv::Scalar(0, 0, 255), 2);

        cv::imshow("Frame", frame);
        cv::imshow("Background", back);
        
        if (cv::waitKey(30) >= 0) break;
    }
}
