#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
	Mat frame;
	string path = "E:\\Filmy\\c64.avi";
	VideoCapture capture(path);
	namedWindow("my_window");

	for (;;) {
		capture >> frame;
		if (frame.empty()) break;
		imshow("my_window", frame);
		if (cv::waitKey(30) >= 0) break;
	}
}