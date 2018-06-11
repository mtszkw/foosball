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
	bool originalEnabled = true;
	bool trackingEnabled = true;
	bool blueDetectionEnabled = false;
	bool redDetectionEnabled = false;
	bool debugMode = false;
	bool pause = false;
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
	cv::Mat nextFrame;
    cv::VideoCapture capture(config["videoPath"].get<string>());

    while(capture.read(frame))
    {
		if(originalEnabled)
		{
			cv::imshow("Original frame", frame);
		}
		else
		{
			cv::destroyWindow("Original frame");
		}

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

		if(trackingEnabled){
			if (foundBallsState.getFoundball())
			{
				foundBallsState.detectedBalls(restul, deltaTicks);
			}
			cv::Mat rangeRes = detection::transformToHSV(frame, detection::Mode::BALL);
			cv::Mat rangeRes2 = detection::transformToHSV(nextFrame, detection::Mode::BALL);
			cv::Mat trackingFrame = detection::tracking(rangeRes, rangeRes2);
			foundBallsState.contoursFiltering(trackingFrame);
			foundBallsState.detectedBallsResult(restul);
			foundBallsState.updateFilter();
			if(debugMode)
			{
				cv::imshow("Tracking ball frame", trackingFrame);
			}
			else
			{
				cv::destroyWindow("Tracking ball frame");
			}

			if (foundBallsState.balls.size())
			    founded++;

			counter++;
			}
		else
		{
			cv::destroyWindow("Tracking ball frame");
		}
		
		// Players detection
		if(redDetectionEnabled){
			cv::Mat hsvPlayerFrameRed = detection::transformToHSV(frame, detection::Mode::RED_PLAYERS);
			redPlayersFinder.contoursFiltering(hsvPlayerFrameRed);
			redPlayersFinder.detectedPlayersResult(restul, detection::Mode::RED_PLAYERS);
			if(debugMode)
			{
				cv::imshow("Red player detection frame", hsvPlayerFrameRed);
			}
			else
			{
				cv::destroyWindow("Red player detection frame");
			}
		}
		else
		{
				cv::destroyWindow("Red player detection frame");
		}
		if(blueDetectionEnabled){
			cv::Mat hsvPlayerFrameBlue = detection::transformToHSV(frame, detection::Mode::BLUE_PLAYERS);
			bluePlayersFinder.contoursFiltering(hsvPlayerFrameBlue);
			bluePlayersFinder.detectedPlayersResult(restul, detection::Mode::BLUE_PLAYERS);
			if(debugMode)
			{
				cv::imshow("Blue player detection frame", hsvPlayerFrameBlue);
			}
			else
			{
				cv::destroyWindow("Blue player detection frame");
			}
		}
		else
		{
				cv::destroyWindow("Blue player detection frame");
		}
        // Calculate and show ball position and score
        cv::copyMakeBorder(restul, restul, 65, 5, 5, 5, cv::BORDER_CONSTANT);
        foundBallsState.showCenterPosition(restul, 10, 15);
        foundBallsState.showStatistics(restul, founded, counter, 10, 35);
        scoreCounter.trackBallAndScore(foundBallsState.getCenter(), foundBallsState.getFoundball());
        scoreCounter.printScoreBoard(restul, 10, 55);

		cv::imshow("Implementacje Przemyslowe", restul);
		redPlayersFinder.clearVectors();
		bluePlayersFinder.clearVectors();
        foundBallsState.clearVectors();

		switch(cv::waitKey(10)){
			case 27: //'esc' key has been pressed, exit program.
				return 0;
			case 'o': //'t' has been pressed. this will toggle tracking
				originalEnabled = !originalEnabled;
				if(originalEnabled == false) cout<<"Origin frame disabled."<<endl;
				else cout<<"Origin frame enabled."<<endl;
				break;
			case 't': //'t' has been pressed. this will toggle tracking
				trackingEnabled = !trackingEnabled;
				if(trackingEnabled == false) cout<<"Tracking ball disabled."<<endl;
				else cout<<"Tracking ball enabled."<<endl;
				break;
			case 'b': //'b' has been pressed. this will toggle blue players detection
				blueDetectionEnabled = !blueDetectionEnabled;
				if(blueDetectionEnabled == false) cout<<"Blue players detection disabled."<<endl;
				else cout<<"Blue players detection enabled."<<endl;
				break;
			case 'r': //'r' has been pressed. this will toggle red players detection
				redDetectionEnabled = !redDetectionEnabled;
				if(redDetectionEnabled == false) cout<<"Red players detection disabled."<<endl;
				else cout<<"Red players detection enabled."<<endl;
				break;
			case 'd': //'d' has been pressed. this will debug mode
				debugMode = !debugMode;
				if(debugMode == false) cout<<"Debug mode disabled."<<endl;
				else cout<<"Debug mode enabled."<<endl;
				break;
			case 'p': //'p' has been pressed. this will pause/resume the code.
				pause = !pause;
				if(pause == true){ 
					cout<<"Code paused, press 'p' again to resume"<<endl;
					while (pause == true){
						//stay in this loop until 
						switch (cv::waitKey()){
							//a switch statement inside a switch statement? Mind blown.
							case 'p': 
							//change pause back to false
							pause = false;
							cout<<"Code resumed."<<endl;
							break;
						}
					}
				}
		}
    }

    return 0;
}
