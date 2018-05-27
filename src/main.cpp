#include <filesystem>
#include <iostream>
#include <vector>

#include <opencv2/aruco.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "json.hpp"
#include "aruco.hpp"
#include "cameraCalibration.h"
#include "detection.hpp"
#include "score.hpp"
#include "table.hpp"

using namespace std;

nlohmann::json readConfiguration(const string &filename)
{
    nlohmann::json config;
    if (ifstream configFile(filename); configFile.is_open())
    {
        stringstream buffer;
        buffer << configFile.rdbuf();
        config = nlohmann::json::parse(buffer.str());
        cout << "Loaded configuration file:\n" << setw(4) << config << '\n';
    }
    else
    {
        cout << "Cannot open configuration file\n";
        exit(EXIT_FAILURE);
    }

    return config;
}

int main()
{
    // Parse JSON configuration
    nlohmann::json config = readConfiguration("configuration.json");

    // Initialize video capture object with video file
    cv::Mat frame;
    cout << "Initializing video capturing\n";
    cv::VideoCapture capture(config["videoPath"].get<string>());

    // Initialize aruco markers detector
    vector<aruco::ArucoMarker> found, rejected; 
    cout << "Initializing aruco markers detector\n";
    auto aruco_dict = aruco::createDictionary(config["arucoDictionaryPath"].get<string>(), 5);
    auto detector = aruco::loadParametersFromFile(config["arucoDetectorConfigPath"].get<string>());

    // Initialize camera calibration module
    // Run calibration if calibration file path was not provided
    cout << "Initializing camera calibration module\n";
    calibration::CameraCalibration cameraCalibration(config["calibInitConfigPath"].get<string>(),
                                                     config["calibConfigPath"].get<string>());
    if (config["calibConfigPath"].get<string>().empty()) {
        cameraCalibration.init();
    }

    detection::Table gameTable(config["gameTableWidth"].get<int>(), config["gameTableHeight"].get<int>());
    detection::ScoreCounter scoreCounter(gameTable.getSize(), 48);

    detection::FoundBallsState foundBallsState(0.0, false, 0);
    int counter = 0, founded = 0;

    while (1)
    {
        double precTick = foundBallsState.getTicks();
        foundBallsState.setTicks(static_cast<double>(cv::getTickCount()));

        double dT = (foundBallsState.getTicks() - precTick) / cv::getTickFrequency();

        capture >> frame;
        if (frame.empty())
            break;

        frame = cameraCalibration.getUndistortedImage(frame);
        aruco::detectArucoOnFrame(frame, aruco_dict, found, rejected, detector);

        gameTable.updateTableOnFrame(found);
        frame = gameTable.getTableFromFrame(frame);

        cv::Mat result;
        frame.copyTo(result);

        if (foundBallsState.getFoundball())
        {
            foundBallsState.detectedBalls(result, dT);
        }

        cv::Mat rangeRes = detection::transformToHSV(frame);
        foundBallsState.contoursFiltering(rangeRes);
        foundBallsState.detectedBallsResult(result);
        foundBallsState.updateFilter();

        if (foundBallsState.balls.size())
            founded++;
        counter++;

        foundBallsState.showCenterPosition(result, 20, 20);
        foundBallsState.showStatistics(result, founded, counter, 180, 20);
        scoreCounter.trackBallAndScore(foundBallsState.getCenter(), foundBallsState.getFoundball());
        scoreCounter.printScoreBoard(result, 20, 50);
        cv::imshow("Implementacje Przemyslowe", result);

        foundBallsState.clearVectors();

        if (cv::waitKey(10) >= 0)
            break;

        for (int i=0; i<config["videoSkipFramesStep"].get<int>(); ++i) capture >> frame;
    }

    return 0;
}
