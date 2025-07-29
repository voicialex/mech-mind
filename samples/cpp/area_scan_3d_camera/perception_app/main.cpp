#include "CameraManager.hpp"

int main()
{
    CameraManager cameraManager;
    cameraManager.Init();
    cameraManager.Connect();
    cameraManager.Start();
    cameraManager.Stop();
    return 0;
}