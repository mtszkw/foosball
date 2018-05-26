#include <filesystem>
#include <iostream>
#include <vector>

#include <opencv2/aruco.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "aruco.hpp"
#include "cameraCalibration.h"
#include "cxxopts.hpp"
#include "detection.hpp"
#include "score.hpp"
#include "table.hpp"

int main(int argc, const char *argv[])
{
    cxxopts::Options options(argv[0], "Implementacje Przemyslowe");
    options.add_options("")("help", "Display help")("input_path", "Input video file path",
                                                    cxxopts::value<std::string>())(
        "aruco_path", "Aruco dictionary configuration",
        cxxopts::value<std::string>()->default_value("data/dictionary.png"))(
        "calibration", "Calibration config file",
        cxxopts::value<std::string>()->default_value(
            "")) // for example: data/out_camera_data_240_fps.xml
        ("calibration_conf", "Calibration init config file",
         cxxopts::value<std::string>()->default_value(
             "data/default.xml")) // for example: data/default.xml
        ("aruco_conf", "Configuration of aruco detector",
         cxxopts::value<std::string>()->default_value(""));

    std::string INPUT_PATH, ARUCO_PATH, CALIB_PATH, ARUCO_CFG_PATH, CALIBRATION_CFG_PATH;

    try
    {
        const auto config = options.parse(argc, argv);
        if (config.count("help"))
        {
            std::cout << options.help({""}) << std::endl;
            exit(EXIT_SUCCESS);
        }
        INPUT_PATH = config["input_path"].as<std::string>();
        ARUCO_PATH = config["aruco_path"].as<std::string>();
        CALIB_PATH = config["calibration"].as<std::string>();
        ARUCO_CFG_PATH = config["aruco_conf"].as<std::string>();
        CALIBRATION_CFG_PATH = config["calibration_conf"].as<std::string>();
    }
    catch (const cxxopts::OptionException &ex)
    {
        std::cerr << "FAILURE: Error while parsing options (" << ex.what() << ")\n";
        exit(EXIT_FAILURE);
    }

    cv::Mat frame;
    cv::VideoCapture capture(INPUT_PATH);

    cv::Ptr<cv::aruco::Dictionary> aruco_dict = aruco::createDictionary(ARUCO_PATH, 5);
    cv::Ptr<cv::aruco::DetectorParameters> detector = aruco::loadParametersFromFile(ARUCO_CFG_PATH);

    std::vector<aruco::ArucoMarker> found, rejected;

    calibration::CameraCalibration cameraCalibration(CALIBRATION_CFG_PATH, CALIB_PATH);
    if (CALIB_PATH.empty())
        cameraCalibration.init(); // initalize calibration if user doesnt provide calibration path
    table::Table gameTable(1200, 600); // Default table size
    score::ScoreCounter scoreCounter(gameTable.getSize(), 48);

    detection::FoundBallsState foundBallsState(0.0, false, 0);
    int counter = 0, founded = 0;

    while (1)
    {
        double precTick = foundBallsState.getTicks();
        foundBallsState.setTicks((double)cv::getTickCount());

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

        if (foundBallsState.balls.size())
            founded++;
        counter++;

        foundBallsState.showCenterPosition(restul, 20, 20);
        foundBallsState.showStatistics(restul, founded, counter, 180, 20);
        scoreCounter.trackBallAndScore(foundBallsState.getCenter(), foundBallsState.getFoundball());
        scoreCounter.printScoreBoard(restul, 400, 20);
        cv::imshow("Implementacje Przemyslowe", restul);

        foundBallsState.clearVectors();

        if (cv::waitKey(10) >= 0)
            break;

        const int SKIP_FRAMES = 5;
        for (int i = 0; i < SKIP_FRAMES; ++i)
            capture >> frame;
    }

    return 0;
}
