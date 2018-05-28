#include <iostream>
#include <filesystem>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <cameraExtensions/cameraExtensions.h>
#include <json.hpp>

nlohmann::json readConfiguration(const std::string &filename)
{
    nlohmann::json config;
    if (std::ifstream configFile(filename); configFile.is_open())
    {
        std::stringstream buffer;
        buffer << configFile.rdbuf();
        config = nlohmann::json::parse(buffer.str());
        std::cout << "Loaded configuration file:\n" << std::setw(4) << config << '\n';
    }
    else
    {
        std::cout << "Cannot open configuration file\n";
        exit(EXIT_FAILURE);
    }

    return config;
}

int main(int argc, const char *argv[])
{
    nlohmann::json config = readConfiguration(argv[1]);
    const std::string input_path = config["input_path"].get<std::string>();
    const std::string mode = config["mode"].get<std::string>();
    cv::Mat image;
    cv::Mat result_image;
    
    switch(mode.at(0))
    {
    case 'r':
        CameraExtensions::reduceShadowEffect(input_path);
        break;
    case 'l':
        image = cv::imread(input_path);
        result_image = CameraExtensions::normalizeLuminance(image);
        cv::namedWindow("Main", CV_WINDOW_NORMAL);
        cv::resizeWindow("Main", 500, 500);
        cv::imshow("Main", result_image);
        break;
    case 'b':
        image = cv::imread(input_path);
        result_image = CameraExtensions::removeShadowInBlackAndWhite(image);
        cv::namedWindow("Main", CV_WINDOW_NORMAL);
        cv::resizeWindow("Main", 500, 500);
        cv::imshow("Main", result_image);
        break;
    }

    return 0;
}



