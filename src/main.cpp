#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <windows.h>
#include <windowsx.h>

using namespace cv;
using namespace std;

HWND hwnd;              
HANDLE hf;              

int main()
{
	string path;
	OPENFILENAME ofn;
	char filename[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Pliki wideo (*.avi)\0*.avi\0Wszystkie pliki\0*.*\0";
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = filename;
	ofn.lpstrDefExt = "avi";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	Mat frame;
	if (GetOpenFileName(&ofn)==TRUE)
		path = ofn.lpstrFile;
	VideoCapture capture(path);
	namedWindow("my_window");

	for (;;) {
		capture >> frame;
		if (frame.empty()) break;
		imshow("my_window", frame);
		if (cv::waitKey(30) >= 0) break;
	}
}