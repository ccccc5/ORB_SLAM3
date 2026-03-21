#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<System.h>
#include<csignal>

using namespace std;

volatile sig_atomic_t b_stop_tracking = 0;

void signal_handler(int signum)
{
    b_stop_tracking = 1;
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cerr << endl << "Usage: ./mono_uvc path_to_vocabulary path_to_settings" << endl;
        return 1;
    }

    // 1. Create SLAM system first. 
    // Loading the vocabulary takes several seconds. If we open the camera before this,
    // the V4L2 buffer fills up with stale frames, causing the camera feed to drop or freeze.
    cout << endl << "Initializing SLAM system... This may take a few seconds." << endl;
    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::MONOCULAR, true);

    // 2. Open the default UVC camera AFTER SLAM initialization
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
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    // cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    // Explicitly set the camera framerate to 120 FPS -> currently using 30
    cap.set(cv::CAP_PROP_FPS, 30);

    // Read the actual FPS to confirm what the camera negotiated
    double actual_fps = cap.get(cv::CAP_PROP_FPS);
    cout << "Negotiated Camera FPS: " << actual_fps << endl;

    cout << endl << "-------" << endl;
    cout << "Start processing live camera feed ..." << endl;
    cout << "Press 'Ctrl+C' in the terminal to close and save the trajectory." << endl;
    cout << "-------" << endl << endl;

    std::signal(SIGINT, signal_handler);

    cv::Mat frame;
    double tframe = 0;

    // Main real-time tracking loop
    while(!b_stop_tracking)
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
        auto t1 = std::chrono::steady_clock::now();
        SLAM.TrackMonocular(frame, tframe);
        auto t2 = std::chrono::steady_clock::now();
        
        double ttrack = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
        int state = SLAM.GetTrackingState();
        int nKeypoints = SLAM.GetTrackedKeyPointsUn().size();
        
        cout << "Frame Size: " << frame.cols << "x" << frame.rows 
             << " | Tracker State: " << state 
             << " | Keypoints: " << nKeypoints
             << " | Tracking Time: " << ttrack << "s" << endl;

        // NOTE: Do NOT use cv::imshow or cv::waitKey in this main thread!
        // ORB_SLAM3 has its own Viewer thread that creates an OpenCV window 
        // ("ORB-SLAM3: Current Frame") and calls cv::waitKey(). 
        // Calling waitKey from two different threads will break X11/GTK event loop
        // causing the SLAM viewer to freeze and show "WAITING FOR IMAGES".
        
        // cap >> frame already blocks at 30 fps, so no sleep is needed.
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
