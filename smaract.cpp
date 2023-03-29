#include "smaract.h"

SmarAct::SmarAct()
{
    char deviceList[1024];
    size_t ioDeviceListLen = sizeof(deviceList);
    result = SA_CTL_FindDevices("", deviceList, &ioDeviceListLen);
    if (result != SA_CTL_ERROR_NONE) {
        printf("MCS2 failed to find devices.\n");
    }
    if (strlen(deviceList) == 0) {
        printf("MCS2 no devices found. Exit.\n");
        getchar();
        exit(1);
    } else {
        printf("MCS2 available devices:\n%s\n", deviceList);
    }

    // Open the first MCS2 device from the list
    char *ptr;
    strtok_r(deviceList, "\n",&ptr);
    char *locator = deviceList;
    result = SA_CTL_Open(&dHandle, locator, "");
    if (result != SA_CTL_ERROR_NONE) {
        printf("MCS2 failed to open \"%s\".\n",locator);
    }


    // The hold time specifies how long the position is actively held after reaching the target.
    // This property is also not persistent and set to zero by default.
    // A value of 0 deactivates the hold time feature, a value of SA_CTL_INFINITE (0xffffffff) sets the time to infinite.
    // (Until manually stopped by "SA_CTL_Stop") Here we set the hold time to 1000 ms.
    for(int chan_i=0; chan_i<3; chan_i++){
        result = SA_CTL_SetProperty_i32(dHandle, chan_i, SA_CTL_PKEY_HOLD_TIME, 0);
        result = SA_CTL_SetProperty_i32(dHandle, chan_i, SA_CTL_PKEY_MOVE_MODE, moveMode);
        result = SA_CTL_SetProperty_i32(dHandle, chan_i, SA_CTL_PKEY_MAX_CL_FREQUENCY, 20000);

    }

}

void SmarAct::move(int stepsize, int8_t direction){
    //std::cout << "channel: " << channel << std::endl;
    //std::cout << "stepsize: " << stepsize << std::endl;
    //std::cout << "velocity: " << velocity << std::endl;
    int8_t channel = (int8_t)0;
    int velocity = 0;
    SA_CTL_Result_t result;
    int64_t moveValue;
    int64_t velocity_int64_t = 1000*1000*1000*(int64_t)velocity;
    //1000*1000*um/ms -> 1000*nm/ms -> pm/ms
    //1000*1000*um/ms*1000ms/1s -> pm/ms*1000ms/1s -> pm/s
    // Set move velocity [in pm/s].
    result = SA_CTL_SetProperty_i64(dHandle, channel, SA_CTL_PKEY_MOVE_VELOCITY, velocity_int64_t);
    // Set move acceleration [in pm/s2], a value of 0 disables the acceleration control.
    result = SA_CTL_SetProperty_i64(dHandle, channel, SA_CTL_PKEY_MOVE_ACCELERATION, 0);
    // Specify relative position distance [in pm] and direction.
    moveValue = 1000*1000*(int64_t)stepsize;
    if (direction) moveValue = -moveValue;
    //printf("MCS2 move channel %d relative: %lld pm at  %lld pm/s.\n", channel, moveValue, velocity_int64_t);
    //std::cout << "MCS2 move channel " << (int)channel << " relative: " << moveValue << " pm at " << velocity_int64_t <<" pm/s."<< std::endl;

    // Start actual movement.
    result = SA_CTL_Move(dHandle, channel, moveValue, 0);
}


void SmarAct::move(int8_t channel, int stepsize, int velocity, int8_t direction){

    //std::cout << "channel: " << channel << std::endl;
    //std::cout << "stepsize: " << stepsize << std::endl;
    //std::cout << "velocity: " << velocity << std::endl;

    SA_CTL_Result_t result;
    int64_t moveValue;
    int64_t velocity_int64_t = 1000*1000*1000*(int64_t)velocity;
    //1000*1000*um/ms -> 1000*nm/ms -> pm/ms
    //1000*1000*um/ms*1000ms/1s -> pm/ms*1000ms/1s -> pm/s
    // Set move velocity [in pm/s].
    result = SA_CTL_SetProperty_i64(dHandle, channel, SA_CTL_PKEY_MOVE_VELOCITY, velocity_int64_t);
    // Set move acceleration [in pm/s2], a value of 0 disables the acceleration control.
    result = SA_CTL_SetProperty_i64(dHandle, channel, SA_CTL_PKEY_MOVE_ACCELERATION, 0);
    // Specify relative position distance [in pm] and direction.
    moveValue = 1000*1000*(int64_t)stepsize;
    if (direction) moveValue = -moveValue;
    //printf("MCS2 move channel %d relative: %lld pm at  %lld pm/s.\n", channel, moveValue, velocity_int64_t);
    //std::cout << "MCS2 move channel " << (int)channel << " relative: " << moveValue << " pm at " << velocity_int64_t <<" pm/s."<< std::endl;

    // Start actual movement.
    result = SA_CTL_Move(dHandle, channel, moveValue, 0);
}
