#include <chrono>
#include <ctime>
#include <thread>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include "yaml-cpp/yaml.h"

#include <opencv2/core.hpp>
#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>


#include "serial_com.h"

using namespace cv;
using namespace std;
using namespace std::chrono;


bool stop = false;

void cap_read(Mat* frame, int* count, bool* stop, int e_time, int width, 
        int height, int gain, int serial_num, int framerate, int binning) {
    char gain_str[100];
    char exposure_str[100];
    char gst_launch_str[500];
    if (gain < 0) {
	sprintf(gain_str, "GainAuto=On");
    } else {
	sprintf(gain_str, "GainAuto=Off,Gain=%d", gain);
    }
    if (e_time < 0) {
	sprintf(exposure_str, "ExposureAuto=On");
    } else {
	sprintf(exposure_str, "ExposureAuto=Off,ExposureTime=%d", e_time);
    }
    sprintf(gst_launch_str,
            "tcambin tcam-properties=tcam,%s,%s,serial=%d"
            " ! video/x-raw,format=BGRx,width=%d,height=%d,framerate=%d/1,"
	    "binning=%dx%d"
            " ! timeoverlay ! appsink", 
	    exposure_str, gain_str, serial_num, width, height, framerate,
	    binning, binning
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
            cv::inRange(hsv_img, cv::Scalar(0, 150, 20), cv::Scalar(5, 255, 255), mask1);
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


void track(int* x, int* y, int width, int height, int center_window_px, int p_lim, int t_lim, std::string port, bool* stop) {
	ThreadSafeStringQueue cmd_queue;
        std::thread t1 (serial_connection, port, std::ref(cmd_queue), stop);
        usleep(ARDUINO_RESET_TIME);
	int x_count = 0;
	int y_count = 0;
	
	int window_x_min = (int)((width / 2) - center_window_px);
	int window_x_max = (int)((width / 2) + center_window_px);
	int window_y_min = (int)((height / 2) - center_window_px);
	int window_y_max = (int)((height / 2) + center_window_px);

	while(!*stop) {
	    std::vector<std::string> commands;	
	    if (*x > 0 && *y > 0) {
		if (*x < window_x_min && abs(x_count) < p_lim) {
		    commands.push_back("p-1");
		    x_count -= 1;
		} else if (*x > window_x_max && abs(x_count) < p_lim) {
		    commands.push_back("p1");
		    x_count += 1;
		}
		if (*y < window_y_min && abs(y_count) < t_lim) {
		    commands.push_back("t1");
		    y_count += 1;
		} else if (*x > window_x_max && abs(y_count) < t_lim) {
		    commands.push_back("t-1");  // Flipping here because know it's flipped. Make configurable.
		    y_count -= 1;
		}
		for (const std::string& cmd : commands) {
                    cmd_queue.addString(cmd.c_str());
		    std::cout << cmd << std::endl;
		}
	    } else {
		usleep(1000);
	    }
	}
        t1.join();
}


void my_handler(int s){
    printf("\nCaught signal %d\n", s);
    stop = true;
}

std::string get_dt_str(std::string ext) {
    std::time_t currentTime = std::time(nullptr);
    std::tm* timeInfo = std::localtime(&currentTime);
    char buffer[80]; // Buffer to store the formatted date and time
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", timeInfo);

    std::string dateTimeString(buffer);
    dateTimeString += ext;
    return dateTimeString;
}

int main(int, char**) {
    Mat frame;
    ofstream timestamp_file;
    int shared_count = 0;
    int count = 0;
    int detect_x = 0;
    int detect_y = 42;
    vector<vector<Point>> contours;
    YAML::Node config = YAML::LoadFile("config.yaml");

    timestamp_file.open(get_dt_str(".txt"));
    signal (SIGINT, my_handler);
    std::vector<std::thread> threads;

    int width = config["width"].as<int>();
    int height = config["height"].as<int>();
    int binning = config["binning"].as<int>();
    if (binning > 1) {
        width = (int) (width / binning);
	height = (int) (height / binning);
    }
    std::thread t1 (cap_read,
            &frame, 
            &shared_count,
            &stop,
            config["ExposureTime_us"].as<int>(),
            width,
            height,
            config["gain"].as<int>(),
            config["serial_num"].as<int>(),
            config["framerate"].as<int>(),
	    binning
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
    if (config["track"] .as<bool>()) {   
	std::thread t4 (track,
                &detect_x,
                &detect_y,
                config["width"].as<int>(),
                config["height"].as<int>(),
                config["center_window_px"].as<int>(),
                config["pan_limit"].as<int>(),
                config["tilt_limit"].as<int>(),
		config["port"].as<std::string>(),
                &stop);
        threads.push_back(move(t4));
    }
    cv::VideoWriter video(get_dt_str(".mp4"),
            cv::VideoWriter::fourcc('a', 'v', 'c', '1'),
            config["emperical_framerate"].as<double>(),
            cv::Size(width, height),
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
            uint64_t us_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            timestamp_file << i << ": " << us_since_epoch << "," << endl;
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
    timestamp_file.close();
    printf("Main: Video Released\n");
    return 0;
}
