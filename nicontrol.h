#ifndef NICONTROL_H
#define NICONTROL_H
#include "NIDAQmx.h"
#include <thread>
#include <ctime>

#define sample_size_square_pulse 2
#define sample_size_sawtooth_pulse 20 // 10 is MAX VALUE(NI USB 6008), IT MUST BE EVEN NUMBER.


class NIControl
{
public:
    NIControl();
    ~NIControl();
    void NI_INIT_SQUARE_PULSE(double pulse_width_ms);
    void NI_INIT_SQUARE_PULSE2(double pulse_width_ms);
    void NI_INIT_SAWTOOTH_PULSE(double pulse_width_ms);
    void NI_INIT_SAWTOOTH_PULSE2(double pulse_width_ms);
    void NI_INIT_ON_OFF();
    void NI_WRITE(float64 voltage);
    void NI_WRITE2(float64 voltage);
    void NI_WRITE3(float64 voltage);
    void NI_GENERATE_SQUARE_PULSE(float64 voltage);
    void NI_GENERATE_SQUARE_PULSE2(float64 voltage);
    void NI_GENERATE_SAWTOOTH_PULSE(float64 voltage);
    void NI_GENERATE_SAWTOOTH_PULSE2(float64 voltage);
    void NI_GENERATE_SAWTOOTH_PULSE2(float64 voltage, double duration);
    void NI_GENERATE_SAWTOOTH_PULSE3(float64 voltage);

private:
    TaskHandle 	taskHandle_OUT{0}, taskHandle_OUT2{0};
    const char* output_physical_channel = "Dev1/ao1"; // Dev1/ao0 is broken.
    const char* output_physical_channel2 = "Dev1/ao1";

    int32 written{0};
    double pulse_width_ms{1.0};
    //const int sample_size_square_pulse = 2;
    //const int sample_size_sawtooth_pulse = 20;

};

#endif // NICONTROL_H
