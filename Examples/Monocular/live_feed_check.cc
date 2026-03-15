#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;

int main(int argc, char **argv)
{
    cout << "Checking Live Camera Feed..." << endl;

    // Open the default UVC camera (index 0) using V4L2 backend explicitly
    // If you have multiple cameras and the wrong one opens, you can try changing the index to 1, 2, etc.
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

    const string window_name = "UVC Camera Feed Check";
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);

    cout << "Camera successfully opened. Press 'ESC' or 'q' on the image window to close." << endl;

    cv::Mat frame;
    while (true)
    {
        cap >> frame;
        
        if (frame.empty())
        {
            cerr << "ERROR: Captured empty frame. Camera might be disconnected." << endl;
            break;
        }

        cv::imshow(window_name, frame);

        char key = (char)cv::waitKey(30);
        if (key == 27 || key == 'q' || key == 'Q') // ESC or 'q'
        {
            cout << "Exiting feed check..." << endl;
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
