#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <opencv2/aruco.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

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

    std::string trackerTypes[6] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN"};
 

    std::string trackerType = trackerTypes[4];
    
    cv::Ptr<cv::Tracker> tracker;

        if (trackerType == "BOOSTING")
            tracker = cv::TrackerBoosting::create();
        if (trackerType == "MIL")
            tracker = cv::TrackerMIL::create();
        if (trackerType == "KCF")
            tracker = cv::TrackerKCF::create();
        if (trackerType == "TLD")
            tracker = cv::TrackerTLD::create();
        if (trackerType == "MEDIANFLOW")
            tracker = cv::TrackerMedianFlow::create();
        if (trackerType == "GOTURN")
            tracker = cv::TrackerGOTURN::create();

    bool tracingInit = false;

    while(capture.read(frame))
    {
        double precTick = foundBallsState.getTicks();
        foundBallsState.setTicks(static_cast<double>(cv::getTickCount()));
        double deltaTicks = (foundBallsState.getTicks() - precTick) / cv::getTickFrequency();

        frame = cameraCalibration.getUndistortedImage(frame);
        aruco::detectArucoOnFrame(frame, aruco_dict, found, rejected, detector);

        gameTable.updateTableOnFrame(found);
        frame = gameTable.getTableFromFrame(frame);
		cv::Mat restul;
		frame.copyTo(restul);
		// Ball detection
        if (foundBallsState.getFoundball())
        {
            foundBallsState.detectedBalls(restul, deltaTicks);
        }

        cv::Mat rangeRes = detection::transformToHSV(frame, detection::Mode::BALL);
        foundBallsState.contoursFiltering(rangeRes);
        foundBallsState.detectedBallsResult(restul);
        //foundBallsState.updateFilter();
        if(foundBallsState.balls.size() > 0 && !tracingInit){
            tracker->init(restul, foundBallsState.bbox);
            tracingInit = true;
        }
                double timer = (double)cv::getTickCount();
                bool ok = tracker->update(frame, foundBallsState.bbox); 
                float fps = cv::getTickFrequency() / ((double)cv::getTickCount() - timer);
                
                if (ok)
                {
                    rectangle(restul, foundBallsState.bbox, cv::Scalar( 215, 214, 0 ), 2, 1 );
                }
                else
                {
                    cv::putText(restul, "Tracking failure detected", cv::Point(100,80), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0,0,255),2);
                }
        
        if (foundBallsState.balls.size())
            founded++;

        counter++;

		// Players detection
		cv::Mat hsvPlayerFrameRed = detection::transformToHSV(frame, detection::Mode::RED_PLAYERS);
		redPlayersFinder.contoursFiltering(hsvPlayerFrameRed);
		redPlayersFinder.detectedPlayersResult(restul, detection::Mode::RED_PLAYERS);

		cv::Mat hsvPlayerFrameBlue = detection::transformToHSV(frame, detection::Mode::BLUE_PLAYERS);
		bluePlayersFinder.contoursFiltering(hsvPlayerFrameBlue);
		bluePlayersFinder.detectedPlayersResult(restul, detection::Mode::BLUE_PLAYERS);

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

        if (cv::waitKey(10) >= 0)
            break;

        for (int i=0; i<config["videoSkipFramesStep"].get<int>(); ++i) capture >> frame;
    }

    return 0;
}
