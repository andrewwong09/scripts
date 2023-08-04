#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <stdio.h>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
using namespace cv;
using namespace std;
using namespace std::chrono;

void cap_read(Mat* frame, int* count, bool* stop) {
	const char* gst_launch_str = "tcambin tcam-properties=tcam,ExposureAuto=Off,serial=46810510,ExposureTime=10000"
				     " ! video/x-raw,format=BGRx,width=3072,height=2048,framerate=60/1"
				     " ! appsink";
	VideoCapture cap(gst_launch_str, CAP_GSTREAMER);

	if (!cap.isOpened()) {
		cerr << "ERROR! Unable to open camera\n";
		return;
	}
	while(!*stop) {
		cap.read(*frame);
		if (frame->empty()) {
			cerr << "ERROR! blank frame grabbed\n";
		} else {
			*count = *count + 1;
		}
	}
}

void display(Mat* frame, int* count, bool* stop) {
	int local_count = 0;
	namedWindow("Display", WINDOW_AUTOSIZE);
	while(!*stop) {
		if (local_count != *count) {
			local_count = *count;
			imshow("Display", *frame);
			if (waitKey(30) == 27) {
				break;
			}	
		}

	}
}

int main(int, char**) {
	Mat frame;
	int shared_count = 0;
	int count = 0;
	bool stop = false;
	std::thread t1 (cap_read, &frame, &shared_count, &stop);
	std::thread t2 (display, &frame, &shared_count, &stop);
	char img_name_buffer[100];
	auto start = high_resolution_clock::now();
	auto beginning = high_resolution_clock::now();
	cout << setprecision(2) << fixed;
	float fps = 0;
	int fps_window = 10;
	for (int i=0; i < 200; i++) {
		if (count != shared_count) {
			count = shared_count;
			sprintf(img_name_buffer, "./display_img%03d_%03d.jpeg", i, count);
			imwrite(img_name_buffer, frame);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			if (i % fps_window == 0) {
				printf("Enter \n");
				auto total_duration = duration_cast<microseconds>(stop - beginning);
				fps = (float) fps_window / total_duration.count() * 1e6;
				beginning = stop;
			}
			cout << i << ": " << duration.count() << " μS / " << fps << " fps" << endl;
			start = stop;
		} else {
			i--;
		}
	}
	stop = true;
	t1.join();
	t2.join();
	return 0;
}
