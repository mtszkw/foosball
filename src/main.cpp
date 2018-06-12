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
#include "gui/gui.hpp"

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
	bool originalEnabled = false, trackingEnabled = true, blueDetectionEnabled = false, redDetectionEnabled = false, debugMode = false, pause = false;
    // Parse JSON configuration
    nlohmann::json config = readConfiguration("configuration.json");
	gui::consoleKeyDoc();
    // Initialize aruco markers detector
    vector<aruco::ArucoMarker> found, rejected; 
    auto aruco_dict = aruco::createDictionary(config["arucoDictionaryPath"].get<string>(), 5);
    auto detector = aruco::loadParametersFromFile(config["arucoDetectorConfigPath"].get<string>());
	
    // Initialize camera calibration module
    // Run calibration if calibration file path was not provided
    calibration::CameraCalibration cameraCalibration(config["calibInitConfigPath"].get<string>(),
                                                     config["calibConfigPath"].get<string>());
    if (config["calibConfigPath"].get<string>().empty())
	{
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
	cv::Mat nextFrame;
    cv::VideoCapture capture(config["videoPath"].get<string>());

    while(capture.read(frame))
    {
		gui::showOriginalFrame(originalEnabled, frame);
        for (int i=0; i<config["videoSkipFramesStep"].get<int>(); ++i) capture >> frame;
		capture.read(nextFrame);
        double precTick = foundBallsState.getTicks();
        foundBallsState.setTicks(static_cast<double>(cv::getTickCount()));
        double deltaTicks = (foundBallsState.getTicks() - precTick) / cv::getTickFrequency();
		
        frame = cameraCalibration.getUndistortedImage(frame);
        nextFrame = cameraCalibration.getUndistortedImage(nextFrame);
        aruco::detectArucoOnFrame(frame, aruco_dict, found, rejected, detector);
        gameTable.updateTableOnFrame(found);
        frame = gameTable.getTableFromFrame(frame);
		
		aruco::detectArucoOnFrame(nextFrame, aruco_dict, found, rejected, detector);
        gameTable.updateTableOnFrame(found);
        nextFrame = gameTable.getTableFromFrame(nextFrame);

		cv::Mat restul;
		frame.copyTo(restul);
		// Ball detection
		detection::trackBall(trackingEnabled, debugMode, foundBallsState, deltaTicks, founded, counter, frame, nextFrame, restul);
		
		// Players detection		
		detection::detectPlayers(redDetectionEnabled, debugMode, detection::Mode::RED_PLAYERS, redPlayersFinder, frame, restul);
		detection::detectPlayers(blueDetectionEnabled, debugMode, detection::Mode::BLUE_PLAYERS, bluePlayersFinder, frame, restul);
        
		// Calculate and show ball position and score
        cv::copyMakeBorder(restul, restul, 45, 45, 5, 5, cv::BORDER_CONSTANT);
        scoreCounter.trackBallAndScore(foundBallsState.getCenter(), foundBallsState.getFoundball());
        gui::printScoreBoard(scoreCounter, restul, (int)(5.0 / 12 * config["gameTableWidth"].get<int>()), 30);
        gui::showCenterPosition(restul, foundBallsState.getCenter(), 10, config["gameTableHeight"].get<int>() + 65);
        gui::showStatistics(restul, founded, counter, 10, config["gameTableHeight"].get<int>() + 80);
		gui::printKeyDoc(restul, 300, config["gameTableHeight"].get<int>() + 65);
		cv::imshow("Implementacje Przemyslowe", restul);
		
		redPlayersFinder.clearVectors();
		bluePlayersFinder.clearVectors();
        foundBallsState.clearVectors();

		gui::handlePressedKeys(cv::waitKey(10), originalEnabled, trackingEnabled, blueDetectionEnabled, redDetectionEnabled, pause, debugMode);
    }
    return 0;
}
