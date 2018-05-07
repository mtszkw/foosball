#include <iostream>
#include <filesystem>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "cameraCalibration.h"
#include "cxxopts.hpp"

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

bool endsWith(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

int main(int argc, const char *argv[])
{
    cxxopts::Options options(argv[0], "Implementacje Przemyslowe");
    options.add_options("")
    ("h,help", "Display help")
    ("i,input_path", "Input settings file path", cxxopts::value<std::string>())
    ("d,calibration_path", "Input camera calibration file", cxxopts::value<std::string>())
    ("c,image_list_path", "Images to be undistored path", cxxopts::value<std::string>());

    const auto config = parseConfiguration(options, argc, argv);
    const std::string input_path = config["input_path"].as<std::string>();
    const std::string calibration_path = config["calibration_path"].as<std::string>();
    const std::string image_list_path = config["image_list_path"].as<std::string>();

    if (!std::experimental::filesystem::exists(input_path))
    {
        std::cerr << "FAILURE: Input file \"" << input_path << "\" does not exist.\n";
        exit(EXIT_FAILURE);
    }

    if(calibration_path.empty())
    {
        calibration::CameraCalibration cameraCalibration(input_path);
        cameraCalibration.init();
    }
    else
    {
        if(!image_list_path.empty())
        {
            calibration::CameraCalibration cameraCalibration(input_path, calibration_path);
            namedWindow("DistortedImage", cv::WINDOW_NORMAL);
            namedWindow("UndistortedImage", cv::WINDOW_NORMAL);
            for (auto & p : std::experimental::filesystem::directory_iterator(image_list_path))
            {
                if(endsWith(p.path().string(), ".MP4"))
                {
                    cv::VideoCapture capture(p.path().string());
                    cv::Mat frame;
                    for (;;)
                    {
                        capture >> frame;
                        if (frame.empty())
                            break;
                        cv::imshow("DistortedImage", frame);
                        imshow("UndistortedImage", cameraCalibration.getUndistortedImage(frame));
                        if(char(cv::waitKey(5)) == 27)  // if ESC
                        {
                            break;
                        }
                    }
                }
                else
                {
                    cv::Mat image = cv::imread(p.path().string(), CV_LOAD_IMAGE_COLOR);
                    cv::imshow("DistortedImage", image);
                    imshow("UndistortedImage", cameraCalibration.getUndistortedImage(image));
                    int keyCode;
                    while((keyCode = cv::waitKey(20)) == -1) {}
                    if(char(keyCode) == 27)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            std::cerr << "FAILURE: You have to provide image list path.\n";
        }

    }


}



