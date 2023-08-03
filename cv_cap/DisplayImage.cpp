#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
using namespace cv;
using namespace std;
int main(int, char**)
{
 Mat frame;
 //--- INITIALIZE VIDEOCAPTURE
 VideoCapture cap("tcambin ! video/x-raw,format=BGRx,width=1920,height=1080,framerate=5/1 ! appsink", CAP_GSTREAMER);
 // open the default camera using default API
 // cap.open(0);
 // OR advance usage: select any API backend
 int deviceID = 0; // 0 = open default camera
 int apiID = cv::CAP_ANY; // 0 = autodetect default API
 // open selected camera using selected API
 //cap.open(deviceID, apiID);
 //cap.open();
 // check if we succeeded
 if (!cap.isOpened()) {
 cerr << "ERROR! Unable to open camera\n";
 return -1;
 }
 //--- GRAB AND WRITE LOOP
 cout << "Start grabbing" << endl
 << "Press any key to terminate" << endl;
 for (;;)
 {
 // wait for a new frame from camera and store it into 'frame'
 cap.read(frame);
 // check if we succeeded
 if (frame.empty()) {
 cerr << "ERROR! blank frame grabbed\n";
 break;
 }
 // show live and wait for a key with timeout long enough to show images
 imshow("Live", frame);
 if (waitKey(5) >= 0)
 break;
 }
 // the camera will be deinitialized automatically in VideoCapture destructor
 return 0;
}
