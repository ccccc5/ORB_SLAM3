#include<iostream>
#include<System.h>
#include<unistd.h>

using namespace std;

int main(int argc, char **argv)
{
    if(argc != 3) {
        cerr << "Usage: ./test_shutdown path_to_vocabulary path_to_settings" << endl;
        return 1;
    }

    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::MONOCULAR, true);

    cout << "System initialized. Waiting 2 seconds..." << endl;
    sleep(2);

    cout << "Initiating shutdown..." << endl;
    SLAM.Shutdown();
    cout << "Shutdown complete." << endl;

    return 0;
}
