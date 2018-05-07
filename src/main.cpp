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
#include "detection.hpp"

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
   
    detection::FoundBallsState foundBallsState(0.0, false, 0);

    while (1) {
        double precTick = foundBallsState.getTicks();
        foundBallsState.setTicks((double) cv::getTickCount());

        double dT = (foundBallsState.getTicks() - precTick) / cv::getTickFrequency(); 

        capture >> frame;
        if (frame.empty())
            break;

        frame = cameraCalibration.getUndistortedImage(frame);

        aruco::detectArucoOnFrame(frame, aruco_dict, found, rejected, detector);
        
        gameTable.updateTableOnFrame(found);
        frame = gameTable.getTableFromFrame(frame);

        cv::Mat restul;
        frame.copyTo(restul);

        if (foundBallsState.getFoundball())
        {
            foundBallsState.detectedBalls(restul, dT);
        }

        cv::Mat rangeRes = detection::transformToHSV(frame);
        foundBallsState.contoursFiltering(rangeRes);
        foundBallsState.detectedBallsResult(restul);
        
        foundBallsState.updateFilter();

        cv::imshow("Implementacje Przemyslowe", restul);

        foundBallsState.clearVectors();

        if (cv::waitKey(10) >= 0) 
            break;
    }

    return 0;
}
