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

void cap_read(VideoCapture cap) {
	cap.read()
}

int main(int, char**) {
	Mat frame;
	//--- INITIALIZE VIDEOCAPTURE
	const char* gst_launch_str = "tcambin tcam-properties=tcam,ExposureAuto=Off,serial=46810510,ExposureTime=10000"
				     " ! video/x-raw,format=BGRx,width=3072,height=2048,framerate=60/1 "
				     " ! appsink";
	char img_name_buffer[100];
	VideoCapture cap(gst_launch_str, CAP_GSTREAMER);

	if (!cap.isOpened()) {
		cerr << "ERROR! Unable to open camera\n";
		return -1;
	}
	cout << "Start grabbing" << endl
		<< "Press any key to terminate" << endl;
	//std::thread t1 (cap_read, cap)

	for (int i=0; i < 100; i++) {
		auto start = high_resolution_clock::now();
		cap.read(frame);
		// check if we succeeded
		if (frame.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		} else {
			sprintf(img_name_buffer, "./display_img%03d.bmp", i);
			imwrite(img_name_buffer, frame);
		}
		// show live and wait for a key with timeout long enough to show images
		// imshow("Live", frame);
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);

		cout << i << ": " << duration.count() << " microseconds" << endl;
		//if (waitKey(5) >= 0)
		//	break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}
