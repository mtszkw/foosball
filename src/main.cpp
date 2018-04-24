#include <iostream>
#include <filesystem>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

#include "cxxopts.hpp"
#include "aruco.hpp"
#include "table.hpp"
#include "cameraCalibration.h"

cxxopts::ParseResult parseConfiguration(cxxopts::Options &options, int argc, const char *argv[])
{
    try
    {
        const auto config = options.parse(argc, argv);
        if (!config.count("input_path") || config.count("help"))
        {
            std::cerr << options.help({ "" }) << std::endl;
            exit(EXIT_FAILURE);
        }
        return config;
    }
    catch (const cxxopts::OptionException& ex)
    {
        std::cerr << "FAILURE: Error while parsing options (" << ex.what() << ")\n";
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char *argv[])
{
    cxxopts::Options options(argv[0], "Implementacje Przemyslowe");
    options.add_options("")
        ("h,help", "Display help")
        ("i,input_path", "Input video file path", cxxopts::value<std::string>())
        ("d,aruco_path", "Aruco dictionary configuration", 
            cxxopts::value<std::string>()->default_value("data/dictionary.png"))
        ("c,calibration", "Calibration config file",
            cxxopts::value<std::string>()->default_value("data/out_camera_data.xml"))
        ("a,aruco_conf", "Configuration of aruco detector",
            cxxopts::value<std::string>()->default_value(""));

    const auto config = parseConfiguration(options, argc, argv);
    const std::string input_path = config["input_path"].as<std::string>();
    const std::string aruco_path = config["aruco_path"].as<std::string>();
    const std::string calibration_path = config["calibration"].as<std::string>();
    const std::string aruco_conf_path = config["aruco_conf"].as<std::string>();

    if (!std::experimental::filesystem::exists(input_path))
    {
        std::cerr << "FAILURE: Input file \"" << input_path << "\" does not exist.\n";
        exit(EXIT_FAILURE);
    }

    cv::Mat frame;
    cv::VideoCapture capture(input_path);

    cv::Ptr<cv::aruco::Dictionary> aruco_dict = aruco::createDictionary(aruco_path, 5);
    cv::Ptr<cv::aruco::DetectorParameters> detector = aruco::loadParametersFromFile(aruco_conf_path);

    std::vector<aruco::ArucoMarker> found, rejected;

    calibration::CameraCalibration cameraCalibration("data/default.xml", calibration_path);
    table::Table gameTable(1200, 600); // Default table size

    //KLAM FILTER
     int stateSize = 6;
    int measSize = 4;
    int contrSize = 0;

    unsigned int type = CV_32F;
    cv::KalmanFilter kf(stateSize, measSize, contrSize, type);

    cv::Mat state(stateSize, 1, type);  
    cv::Mat meas(measSize, 1, type);    

    cv::setIdentity(kf.transitionMatrix);

    kf.measurementMatrix = cv::Mat::zeros(measSize, stateSize, type);
    kf.measurementMatrix.at<float>(0) = 1.0f;
    kf.measurementMatrix.at<float>(7) = 1.0f;
    kf.measurementMatrix.at<float>(16) = 1.0f;
    kf.measurementMatrix.at<float>(23) = 1.0f;

    kf.processNoiseCov.at<float>(0) = 1e-2;
    kf.processNoiseCov.at<float>(7) = 1e-2;
    kf.processNoiseCov.at<float>(14) = 5.0f;
    kf.processNoiseCov.at<float>(21) = 5.0f;
    kf.processNoiseCov.at<float>(28) = 1e-2;
    kf.processNoiseCov.at<float>(35) = 1e-2;

    cv::setIdentity(kf.measurementNoiseCov, cv::Scalar(1e-1));
    // <<<< Kalman Filter

    // Camera Index
    int idx = 0;

    char ch = 0;

    double ticks = 0;
    bool foundball = false;

    int notFoundCount = 0;

    while (1) {
        double precTick = ticks;
        ticks = (double) cv::getTickCount();

        double dT = (ticks - precTick) / cv::getTickFrequency(); 

        capture >> frame;
        if (frame.empty())
            break;

        frame = cameraCalibration.getUndistortedImage(frame);

        aruco::detectArucoOnFrame(frame, aruco_dict, found, rejected, detector);
        
        gameTable.updateTableOnFrame(found);
        frame = gameTable.getTableFromFrame(frame);

        cv::Mat res;
        frame.copyTo( res );

        if (foundball)
        {
            // >>>> Matrix A
            kf.transitionMatrix.at<float>(2) = dT;
            kf.transitionMatrix.at<float>(9) = dT;
            // <<<< Matrix A

            state = kf.predict();
            
            cv::Rect predRect;
            predRect.width = state.at<float>(4);
            predRect.height = state.at<float>(5);
            predRect.x = state.at<float>(0) - predRect.width / 2;
            predRect.y = state.at<float>(1) - predRect.height / 2;

            cv::Point center;
            center.x = state.at<float>(0);
            center.y = state.at<float>(1);
            cv::circle(res, center, 2, CV_RGB(255,0,0), -1);

            cv::rectangle(res, predRect, CV_RGB(255,0,0), 2);
        }
	

        // >>>>> Noise smoothing
        cv::Mat blur;
        cv::GaussianBlur(frame, blur, cv::Size(5, 5), 3.0, 3.0);
        // <<<<< Noise smoothing

        // >>>>> HSV conversion
	
        cv::Mat frmHsv;
        cv::cvtColor(blur, frmHsv, cv::COLOR_BGR2HSV);
        // <<<<< HSV conversion

    	cv::Mat lowerHueRange;
	    cv::Mat upperHueRange;
	    cv::inRange(frmHsv, cv::Scalar(5, 130, 90), cv::Scalar(21, 255, 203), lowerHueRange);
	    cv::inRange(frmHsv, cv::Scalar(5, 130, 90), cv::Scalar(21, 255, 203), upperHueRange);
	
        // >>>>> Color Thresholding
        cv::Mat rangeRes; 
	    cv::addWeighted(lowerHueRange, 1.0, upperHueRange, 0.8, 0.0, rangeRes);
        // <<<<< Color Thresholding

        // >>>>> Improving the result
        cv::erode(rangeRes, rangeRes, cv::Mat(), cv::Point(-1, -1), 2);
        cv::dilate(rangeRes, rangeRes, cv::Mat(), cv::Point(-1, -1), 2);
        // <<<<< Improving the result

        // >>>>> Contours detection
        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(rangeRes, contours, CV_RETR_EXTERNAL,
                         CV_CHAIN_APPROX_NONE);
        // <<<<< Contours detection

        // >>>>> Filtering
        std::vector<std::vector<cv::Point> > balls;
        std::vector<cv::Rect> ballsBox;
        for (size_t i = 0; i < contours.size(); i++)
        {
            cv::Rect bBox;
            bBox = cv::boundingRect(contours[i]);

            float ratio = (float) bBox.width / (float) bBox.height;
            if (ratio > 1.0f)
                ratio = 1.0f / ratio;

            if(ratio > 0.75 && bBox.area() >= 100){
                balls.push_back(contours[i]);
                ballsBox.push_back(bBox);            
            }           
        }
        // <<<<< Filtering

        // >>>>> Detection result
        for (size_t i = 0; i < balls.size(); i++)
        {
            cv::drawContours(res, balls, i, CV_RGB(20,150,20), 1);
            cv::rectangle(res, ballsBox[i], CV_RGB(0,255,0), 2);

            cv::Point center;
            center.x = ballsBox[i].x + ballsBox[i].width / 2;
            center.y = ballsBox[i].y + ballsBox[i].height / 2;
            cv::circle(res, center, 2, CV_RGB(20,150,20), -1);
        }
        // <<<<< Detection result

        // >>>>> Kalman Update
        if (balls.size() == 0)
        {
            notFoundCount++;
            if( notFoundCount >= 100 )
            {
                foundball = false;
            }
            /*else
                kf.statePost = state;*/
        }
        else
        {
            notFoundCount = 0;

            meas.at<float>(0) = ballsBox[0].x + ballsBox[0].width / 2;
            meas.at<float>(1) = ballsBox[0].y + ballsBox[0].height / 2;
            meas.at<float>(2) = (float)ballsBox[0].width;
            meas.at<float>(3) = (float)ballsBox[0].height;

            if (!foundball) // First detection!
            {
                // >>>> Initialization
                kf.errorCovPre.at<float>(0) = 1; // px
                kf.errorCovPre.at<float>(7) = 1; // px
                kf.errorCovPre.at<float>(14) = 1;
                kf.errorCovPre.at<float>(21) = 1;
                kf.errorCovPre.at<float>(28) = 1; // px
                kf.errorCovPre.at<float>(35) = 1; // px

                state.at<float>(0) = meas.at<float>(0);
                state.at<float>(1) = meas.at<float>(1);
                state.at<float>(2) = 0;
                state.at<float>(3) = 0;
                state.at<float>(4) = meas.at<float>(2);
                state.at<float>(5) = meas.at<float>(3);
                // <<<< Initialization

                kf.statePost = state;
                
                foundball = true;
            }
            else
                kf.correct(meas); // Kalman Correction
        }

        cv::imshow("Implementacje Przemyslowe", res);

        if (cv::waitKey(10) >= 0) 
            break;
    }

    return 0;
}
