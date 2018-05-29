#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <opencv2/aruco.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "json.hpp"
#include "aruco/aruco.hpp"
#include "calib/cameraCalibration.hpp"
#include "detection/detection.hpp"
#include "detection/score.hpp"
#include "detection/table.hpp"

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

    // Initialize aruco markers detector
    vector<aruco::ArucoMarker> found, rejected; 
    auto aruco_dict = aruco::createDictionary(config["arucoDictionaryPath"].get<string>(), 5);
    auto detector = aruco::loadParametersFromFile(config["arucoDetectorConfigPath"].get<string>());
	
    // Initialize camera calibration module
    // Run calibration if calibration file path was not provided
    calibration::CameraCalibration cameraCalibration(config["calibInitConfigPath"].get<string>(),
                                                     config["calibConfigPath"].get<string>());
    if (config["calibConfigPath"].get<string>().empty()) {
        cameraCalibration.init();
    }

    detection::Table gameTable(config["gameTableWidth"].get<int>(), config["gameTableHeight"].get<int>());
    detection::ScoreCounter scoreCounter(gameTable.getSize(), 48);

    detection::FoundBallsState foundBallsState(0.0, false, 0);
	detection::PlayersFinder redPlayersFinder;
	detection::PlayersFinder bluePlayersFinder;
    int counter = 0, founded = 0;

    // Initialize video capture object with video file and start processing
    cv::Mat frame;
    cv::VideoCapture capture(config["videoPath"].get<string>());

    while(capture.read(frame))
    {
        double precTick = foundBallsState.getTicks();
        foundBallsState.setTicks(static_cast<double>(cv::getTickCount()));
        double deltaTicks = (foundBallsState.getTicks() - precTick) / cv::getTickFrequency();

        frame = cameraCalibration.getUndistortedImage(frame);
        aruco::detectArucoOnFrame(frame, aruco_dict, found, rejected, detector);

        gameTable.updateTableOnFrame(found);
        frame = gameTable.getTableFromFrame(frame);

		// Ball detection
        if (foundBallsState.getFoundball())
        {
            foundBallsState.detectedBalls(frame, deltaTicks);
        }

        cv::Mat rangeRes = detection::transformToHSV(frame, detection::Mode::BALL);
        foundBallsState.contoursFiltering(rangeRes);
        foundBallsState.detectedBallsResult(frame);
        foundBallsState.updateFilter();

        if (foundBallsState.balls.size())
            founded++;

        counter++;

		// Players detection
		cv::Mat hsvPlayerFrameRed = detection::transformToHSV(frame, detection::Mode::RED_PLAYERS);
		redPlayersFinder.contoursFiltering(hsvPlayerFrameRed);
        redPlayersFinder.detectedPlayersResult(frame, detection::Mode::RED_PLAYERS);
		
		cv::Mat hsvPlayerFrameBlue = detection::transformToHSV(frame, detection::Mode::BLUE_PLAYERS);
		bluePlayersFinder.contoursFiltering(hsvPlayerFrameBlue);
        bluePlayersFinder.detectedPlayersResult(frame, detection::Mode::BLUE_PLAYERS);

        // Calculate and show ball position and score
        cv::copyMakeBorder(frame, frame, 65, 5, 5, 5, cv::BORDER_CONSTANT);
        foundBallsState.showCenterPosition(frame, 10, 15);
        foundBallsState.showStatistics(frame, founded, counter, 10, 35);
        scoreCounter.trackBallAndScore(foundBallsState.getCenter(), foundBallsState.getFoundball());
        scoreCounter.printScoreBoard(frame, 10, 55);

		cv::imshow("Implementacje Przemyslowe", frame);
		redPlayersFinder.clearVectors();
		bluePlayersFinder.clearVectors();
        foundBallsState.clearVectors();

        if (cv::waitKey(10) >= 0)
            break;

        for (int i=0; i<config["videoSkipFramesStep"].get<int>(); ++i) capture >> frame;
    }

    return 0;
}
