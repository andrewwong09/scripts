#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <opencv2/core.hpp>
#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>


void detect(cv::Mat* frame, int* x, int* y, int min_area, int max_area,
		std::vector<std::vector <cv::Point>>* contours_shared) {
	cv::Mat hsv_img, mask1, mask2, mask3;
	std::vector<cv::Vec4i> hierarchy;
	int x_total = 0;
	int y_total = 0;
	int object_count = 0;
	std::vector<std::vector<cv::Point>> contours;
	contours_shared->clear();
	cv::cvtColor(*frame, hsv_img, cv::COLOR_BGR2HSV);
	// Gen lower mask (0-5) and upper mask (175-180) of RED
	cv::inRange(hsv_img, cv::Scalar(0, 150, 70), cv::Scalar(15, 255, 255), mask1);
	cv::inRange(hsv_img, cv::Scalar(170, 150, 70), cv::Scalar(180, 255, 255), mask2);
	cv::bitwise_or(mask1, mask2, mask3);
	cv::findContours(mask3, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
	std::vector<cv::Moments> mu(contours.size());
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

int main() {
	std::string test_file = "test.mkv";
	cv::VideoCapture capture(test_file);
	if (!capture.isOpened()) {
		std::cout << "Can not load video" << std::endl;
	}
	else {
		int fps = capture.get(cv::CAP_PROP_FPS);
		std::cout << "Frames per second: " << fps << std::endl;
		int frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);
		std::cout << "Frame count: " << frame_count << std::endl;

		for (int i=0; i < frame_count; i++) {
			cv::Mat frame;
			int x = 0; 
			int y = 0;
                	std::vector<std::vector <cv::Point>> contours;
			
			bool isSuccess = capture.read(frame);
			if (isSuccess == true) {
				cv::Mat resize_large, resize_small;
				cv::resize(frame, resize_large, cv::Size(3072, 2048), cv::INTER_LINEAR);
				detect(&resize_large, &x, &y, 0, 1000, &contours);
				cv::drawContours(resize_large, contours, -1, cv::Scalar(255, 255, 0), 5);
				cv::resize(resize_large, resize_small, cv::Size(960, 540), cv::INTER_LINEAR);
				cv::imshow("Frame", resize_small);
				std::cout << "Location: " << x << ", " << y << std::endl;
			}
			if (isSuccess == false) {
				std::cout << "Video camera is disconnected" << std::endl;
				break;
			}        
			int key = cv::waitKey(20);
			if (key == 'q') {
				std::cout << "q key is pressed by the user. Stopping the video" << std::endl;
				break;
			}
		}	
	}

}
