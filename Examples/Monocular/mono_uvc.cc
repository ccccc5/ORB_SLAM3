#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<System.h>

using namespace std;

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cerr << endl << "Usage: ./mono_uvc path_to_vocabulary path_to_settings" << endl;
        return 1;
    }

    // Open the default UVC camera (index 0) using V4L2 explicitly
    // You can alter this to 1, 2, etc., if you have multiple cameras
    cv::VideoCapture cap(0, cv::CAP_V4L2);
    
    // Explicitly set the format to MJPG, which the Arducam supports natively
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    if (!cap.isOpened())
    {
        cerr << "ERROR: Cannot open the UVC camera." << endl;
        return -1;
    }

    // Attempt to set a standard webcam resolution, e.g., 640x480
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    // Explicitly set the camera framerate to 120 FPS
    cap.set(cv::CAP_PROP_FPS, 120);

    // Read the actual FPS to confirm what the camera negotiated
    double actual_fps = cap.get(cv::CAP_PROP_FPS);
    cout << "Negotiated Camera FPS: " << actual_fps << endl;

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::MONOCULAR, true);

    cout << endl << "-------" << endl;
    cout << "Start processing live camera feed ..." << endl;
    cout << "Press 'ESC' on the image window to close." << endl;
    cout << "-------" << endl << endl;

    cv::Mat frame;
    double tframe = 0;

    // Main real-time tracking loop
    while(true)
    {
        // 1. Capture the new frame
        cap >> frame;
        if(frame.empty())
        {
            cerr << "ERROR: Failed to capture frame from the camera. Camera might be disconnected." << endl;
            break;
        }

        // 2. Fetch the current timestamp for the frame
        tframe = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now().time_since_epoch()).count();

        // 3. Pass the image to the SLAM system
        SLAM.TrackMonocular(frame, tframe);

        // Allow some time for OpenCV to update the viewing window (if one exists).
        // 30ms sleep roughly caps input at 33fps max.
        char key = (char)cv::waitKey(30);
        if(key == 27 || key == 'q' || key == 'Q') // ESC or 'q'
        {
            break;
        }
    }

    // Stop all threads
    cout << "Stopping processing ..." << endl;
    SLAM.Shutdown();

    cap.release();
    cv::destroyAllWindows();

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
    SLAM.SaveMapPointCloud("MapPoints.ply");

    return 0;
}
