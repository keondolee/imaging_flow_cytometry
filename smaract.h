#ifndef SMARACT_H
#define SMARACT_H
#include "SmarActControl.h"
#include <iostream>

#if defined(_WIN32)
#define strtok_r strtok_s
#endif

class SmarAct
{
private:
    SA_CTL_Result_t result;
    SA_CTL_DeviceHandle_t dHandle;
    int8_t channel = 0;
    int32_t moveMode = SA_CTL_MOVE_MODE_CL_RELATIVE;
    int64_t position;
    int velocity{296}; // um/ms
    int stepsize{2}; // um

public:
    SmarAct();
    void move(int stepsize, int8_t direction);
    void move(int8_t channel, int stepsize, int velocity, int8_t direction);

};

#endif // SMARACT_H
