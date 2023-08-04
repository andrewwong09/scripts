#include <chrono>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
using namespace cv;
using namespace std;
using namespace std::chrono;

void cap_read(Mat* frame, int* count) {
	//--- INITIALIZE VIDEOCAPTURE
	const char* gst_launch_str = "tcambin tcam-properties=tcam,ExposureAuto=Off,serial=46810510,ExposureTime=10000"
				     " ! video/x-raw,format=BGRx,width=3072,height=2048,framerate=60/1 "
				     " ! appsink";
	VideoCapture cap(gst_launch_str, CAP_GSTREAMER);

	if (!cap.isOpened()) {
		cerr << "ERROR! Unable to open camera\n";
		return;
	}
	for(int i=0;i < 400; i++) {
		cap.read(*frame);
		if (frame->empty()) {
			cerr << "ERROR! blank frame grabbed\n";
		} else {
			*count = *count + 1;
		}
	}
}

int main(int, char**) {
	Mat frame;
	int shared_count = 0;
	int count = 0;
	std::thread t1 (cap_read, &frame, &shared_count);
	char img_name_buffer[100];
	auto start = high_resolution_clock::now();
	for (int i=0; i < 100; i++) {
		// check if we succeeded
		if (count != shared_count) {
			count = shared_count;
			sprintf(img_name_buffer, "./display_img%03d_%03d.jpeg", i, count);
			imwrite(img_name_buffer, frame);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			cout << i << ": " << duration.count() << " microseconds" << endl;
			start = stop;
		} else {
			i--;
		}
	}
	t1.join();
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}
