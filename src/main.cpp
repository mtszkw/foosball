#include <iostream>
#include <filesystem>

#include <opencv2/opencv.hpp>

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

int main(int argc, const char *argv[])
{
	cxxopts::Options options(argv[0], "Implementacje Przemyslowe");
	options.add_options("")
		("h,help", "Display help")
		("i,input_path", "Input video file path", cxxopts::value<std::string>());

	const auto config = parseConfiguration(options, argc, argv);
	const std::string input_path = config["input_path"].as<std::string>();

	if (!std::experimental::filesystem::exists(input_path))
	{
		std::cerr << "FAILURE: Input file \"" << input_path << "\" does not exist.\n";
		exit(EXIT_FAILURE);
	}

	cv::Mat frame;
	cv::VideoCapture capture(input_path);
	cv::namedWindow("my_window");

	for (;;) {
		capture >> frame;
		if (frame.empty()) break;
		imshow("my_window", frame);
		if (cv::waitKey(30) >= 0) break;
	}
}