#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include "yaml-cpp/yaml.h"

#include <opencv2/core.hpp>
#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
using namespace cv;
using namespace std;
using namespace std::chrono;


bool stop = false;

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
	cap.release();
}

void display(Mat* frame, int* count, bool* stop, int fps, int width, int height, 
		vector<vector<Point>>* contours) {
	int local_count = 0;
	int sleep_us = (int) (1e6 / fps / 2);
	cv::namedWindow("Display", WINDOW_AUTOSIZE);
	while(!*stop) {
		if (local_count != *count) {
			local_count = *count;
			cv::Mat display_img = frame->clone();
			cv::Mat resized_display_img;
			cv::drawContours(display_img, *contours, -1, cv::Scalar(255, 255, 0), 5);
			cv::resize(display_img, resized_display_img, cv::Size(width, height), cv::INTER_LINEAR);
			cv::imshow("Display", resized_display_img);
			if (cv::waitKey(30) == 27) {
				*stop = true;
				break;
			}
		}
		usleep(sleep_us);
	}
}

void detect(Mat* frame, int* count, bool* stop, int fps, int* x, int* y, int min_area, int max_area,
		vector<vector <Point>>* contours_shared) {
	cv::Mat hsv_img, mask1, mask2, mask3;
	vector<Vec4i> hierarchy;
	int skip_num = 10;
	int sleep_us = (int) (1e6 / fps / 3 / skip_num);
	int local_count = 0;
	while(!*stop) {
		int x_total = 0;
		int y_total = 0;
		int object_count = 0;
		if (local_count < *count - skip_num) {
			local_count = *count;
			vector<vector<Point>> contours;
			contours_shared->clear();
			cv::cvtColor(*frame, hsv_img, cv::COLOR_BGR2HSV);
			// Gen lower mask (0-5) and upper mask (175-180) of RED
			cv::inRange(hsv_img, cv::Scalar(0, 150, 20), cv::Scalar(10, 255, 255), mask1);
			cv::inRange(hsv_img, cv::Scalar(170, 150, 20), cv::Scalar(180, 255, 255), mask2);
			cv::bitwise_or(mask1, mask2, mask3);
			cv::findContours(mask3, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
			vector<cv::Moments> mu(contours.size());
			for(size_t i=0; i < contours.size(); i++) {
				mu[i] = moments( contours[i] );
			}
			for (size_t i=0; i < mu.size(); i++) {
				if ((mu[i].m00 > min_area) & (mu[i].m00 < max_area)) {
					object_count += 1;
					int cx = (int) (mu[i].m10 / mu[i].m00);
					int cy = (int) (mu[i].m01 / mu[i].m00);
					x_total += cx;
					y_total += cy;
					contours_shared->push_back(contours[i]);
				}	
			}
			if (object_count > 0) {
				*x = (int) (x_total / object_count);
				*y = (int) (y_total / object_count);
			} else {
				*x = -1;
				*y = -1;
			}
		}
		usleep(sleep_us);

	}  // end while
}

void my_handler(int s){
	printf("\nCaught signal %d\n", s);
	stop = true;
}

int main(int, char**) {
	Mat frame;
	int shared_count = 0;
	int count = 0;
	int detect_x = 0;
	int detect_y = 42;
	vector<vector<Point>> contours;
	YAML::Node config = YAML::LoadFile("config.yaml");
	signal (SIGINT, my_handler);
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
				config["framerate"].as<int>(),
				(int) config["width"].as<int>() / scale,
				(int) config["height"].as<int>() / scale,
				&contours);
		threads.push_back(move(t2));
	}
	if (config["detect"].as<bool>()) {
		std::thread t3 (detect,
				&frame,
				&shared_count,
				&stop,
				config["framerate"].as<int>(),
				&detect_x,
				&detect_y,
				config["min_area"].as<int>(),
				config["max_area"].as<int>(),
				&contours);
		threads.push_back(move(t3));
	}
	cv::VideoWriter video(config["video_filepath"].as<std::string>(),
			cv::VideoWriter::fourcc('a', 'v', 'c', '1'),
			config["emperical_framerate"].as<double>(),
			cv::Size(config["width"].as<int>(), config["height"].as<int>()),
			true);
	auto start = high_resolution_clock::now();
	auto beginning = high_resolution_clock::now();
	cout << setprecision(2) << fixed;
	float fps = 0;
	int fps_window = 5;
	cv::Mat temp_write;
	// two hour recording
	for (int i=0; i < config["framerate"].as<int>() * 60 * 60 * 2; i++) {
		if (count != shared_count) {
			count = shared_count;
			cv::cvtColor(frame, temp_write, cv::COLOR_BGRA2BGR);
			video.write(temp_write);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			if (i % fps_window == 0) {
				auto total_duration = duration_cast<microseconds>(stop - beginning);
				fps = (float) fps_window / total_duration.count() * 1e6;
				beginning = stop;
			}
			cout << i << ": " << duration.count() << " μS / " << fps << " fps, " << detect_x << ", " << detect_y << endl;
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
	video.release();
	printf("Main: Video Released\n");
	return 0;
}
