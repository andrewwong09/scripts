#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include "yaml-cpp/yaml.h"

#include <opencv2/core.hpp>
#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
using namespace cv;
using namespace std;
using namespace std::chrono;

void cap_read(Mat* frame, int* count, bool* stop, int e_time, int width, 
		int height, int gain, int serial_num, int framerate) {
	char gst_launch_str[500];
	sprintf(gst_launch_str,
		"tcambin tcam-properties=tcam,ExposureAuto=Off,GainAuto=Off,"
		"serial=%d,ExposureTime=%d,Gain=%d"
		" ! video/x-raw,format=BGRx,width=%d,height=%d,framerate=%d/1"
		" ! timeoverlay ! appsink", serial_num, e_time, gain, width, height, framerate
		);
	printf("%s\n", gst_launch_str);
	cv::VideoCapture cap(gst_launch_str, CAP_GSTREAMER);

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

void display(Mat* frame, int* count, bool* stop, int width, int height) {
	int local_count = 0;
	cv::Mat resized_display_img;

	cv::namedWindow("Display", WINDOW_AUTOSIZE);
	while(!*stop) {
		if (local_count != *count) {
			local_count = *count;
			cv::resize(*frame, resized_display_img, Size(width, height), cv::INTER_LINEAR);
			cv::imshow("Display", resized_display_img);
			if (cv::waitKey(30) == 27) {
				*stop = true;
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
	YAML::Node config = YAML::LoadFile("config.yaml");
	std::vector<std::thread> threads;
	std::thread t1 (cap_read,
			&frame, 
			&shared_count,
			&stop,
			config["ExposureTime_us"].as<int>(),
			config["width"].as<int>(),
			config["height"].as<int>(),
			config["gain"].as<int>(),
			config["serial_num"].as<int>(),
			config["framerate"].as<int>()
			);
	threads.push_back(move(t1));
	if (config["display"].as<bool>()) {
		cout << "Display Video" << endl;
		int scale = config["display_down_scale"].as<int>();
		std::thread t2 (display,
				&frame,
				&shared_count,
				&stop,
				(int) config["width"].as<int>() / scale,
				(int) config["height"].as<int>() / scale);
		threads.push_back(move(t2));
	}
	char img_name_buffer[100];
	auto start = high_resolution_clock::now();
	auto beginning = high_resolution_clock::now();
	cout << setprecision(2) << fixed;
	float fps = 0;
	int fps_window = 5;
	for (int i=0; i < 200; i++) {
		if (count != shared_count) {
			count = shared_count;
			sprintf(img_name_buffer, "./display_img%03d_%03d.jpeg", i, count);
			imwrite(img_name_buffer, frame);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			if (i % fps_window == 0) {
				auto total_duration = duration_cast<microseconds>(stop - beginning);
				fps = (float) fps_window / total_duration.count() * 1e6;
				beginning = stop;
			}
			cout << i << ": " << duration.count() << " μS / " << fps << " fps" << endl;
			start = stop;
		} else {
			i--;
		}
		if (stop) {
			break;
		}
	}
	stop = true;
	for (auto &th : threads) {
		th.join();
	}
	return 0;
}
