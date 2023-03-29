#include "nicontrol.h"
#include <iostream>

NIControl::NIControl()
{
    // Control air pressure 0~90kPa
    //DAQmxCreateTask("",&taskHandle_OUT);
    //DAQmxCreateAOVoltageChan(taskHandle_OUT, output_physical_channel,"",0.0,10.0,DAQmx_Val_Volts,NULL);
    //DAQmxCfgSampClkTiming(taskHandle_OUT,"",1000.0/1.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1);
    //DAQmxStartTask(taskHandle_OUT);

    // Control On Off pulse
    //DAQmxCreateTask("",&taskHandle_OUT);
    //DAQmxCreateAOVoltageChan(taskHandle_OUT, output_physical_channel,"",0.0,10.0,DAQmx_Val_Volts,NULL);
    //DAQmxCfgSampClkTiming(taskHandle_OUT,"",1000.0/1.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1);
    //DAQmxStartTask(taskHandle_OUT);
}

NIControl::~NIControl()
{
    if (taskHandle_OUT != 0) {
        DAQmxStopTask(taskHandle_OUT);
        DAQmxClearTask(taskHandle_OUT);
    }
    if (taskHandle_OUT2 != 0) {
        DAQmxStopTask(taskHandle_OUT2);
        DAQmxClearTask(taskHandle_OUT2);
    }
}

void NIControl::NI_INIT_ON_OFF(){
    DAQmxCreateTask("",&taskHandle_OUT);
    DAQmxCreateAOVoltageChan(taskHandle_OUT, output_physical_channel,"",0.0,10.0,DAQmx_Val_Volts,NULL);
    DAQmxCfgSampClkTiming(taskHandle_OUT,"",1000.0/1.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1);
    DAQmxStartTask(taskHandle_OUT);
}

void NIControl::NI_INIT_SQUARE_PULSE(double pulse_width_ms){
    //--------- Square Pulse ------------
    //          ┌─────────┐
    //          │         │
    //          │         │
    //          │  Pulse  │
    //          │← Width →│
    // ─────────┘         └──────────────

    this->pulse_width_ms = pulse_width_ms;
    if( taskHandle_OUT!=0 ) {
        DAQmxStopTask(taskHandle_OUT);
        DAQmxClearTask(taskHandle_OUT);
    }
    DAQmxCreateTask("", &taskHandle_OUT);
    DAQmxCreateAOVoltageChan(taskHandle_OUT, output_physical_channel, "", 0.0, 10.0, DAQmx_Val_Volts, NULL);
    DAQmxCfgSampClkTiming(taskHandle_OUT,"",1000.0/pulse_width_ms,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,sample_size_square_pulse);
}

void NIControl::NI_INIT_SQUARE_PULSE2(double pulse_width_ms){
    //--------- Square Pulse ------------
    //          ┌─────────┐
    //          │         │
    //          │         │
    //          │  Pulse  │
    //          │← Width →│
    // ─────────┘         └──────────────

    this->pulse_width_ms = pulse_width_ms;
    if( taskHandle_OUT!=0 ) {
        DAQmxStopTask(taskHandle_OUT);
        DAQmxClearTask(taskHandle_OUT);
    }
    DAQmxCreateTask("", &taskHandle_OUT);
    DAQmxCreateAOVoltageChan(taskHandle_OUT, output_physical_channel, "", 0.0, 10.0, DAQmx_Val_Volts, NULL);
    DAQmxStartTask(taskHandle_OUT);
}

void NIControl::NI_INIT_SAWTOOTH_PULSE(double pulse_width_ms){
    //--------- Sawtooth Pulse ------------
    //          ┌─────────┐
    //          │         └─┐
    //          │         : └─┐
    //          │  Pulse  :   └─┐
    //          │← Width →:     └─┐
    // ─────────┘         :       └────────

    this->pulse_width_ms = pulse_width_ms;
    if( taskHandle_OUT!=0 ) {
        DAQmxStopTask(taskHandle_OUT);
        DAQmxClearTask(taskHandle_OUT);
    }
    DAQmxCreateTask("", &taskHandle_OUT);
    DAQmxCreateAOVoltageChan(taskHandle_OUT, output_physical_channel, "", 0.0, 10.0, DAQmx_Val_Volts, NULL);
    DAQmxCfgSampClkTiming(taskHandle_OUT,"",1000.0/pulse_width_ms*sample_size_sawtooth_pulse/2,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,sample_size_sawtooth_pulse);
}

void NIControl::NI_INIT_SAWTOOTH_PULSE2(double pulse_width_ms){
    //--------- Sawtooth Pulse ------------
    //          ┌─────────┐
    //          │         └─┐
    //          │         : └─┐
    //          │  Pulse  :   └─┐
    //          │← Width →:     └─┐
    // ─────────┘         :       └────────

    this->pulse_width_ms = pulse_width_ms;
    if( taskHandle_OUT!=0 ) {
        DAQmxStopTask(taskHandle_OUT);
        DAQmxClearTask(taskHandle_OUT);
    }
    DAQmxCreateTask("", &taskHandle_OUT);
    DAQmxCreateAOVoltageChan(taskHandle_OUT, output_physical_channel, "", 0.0, 10.0, DAQmx_Val_Volts, NULL);
    DAQmxStartTask(taskHandle_OUT);
}

void NIControl::NI_GENERATE_SQUARE_PULSE(float64 voltage){
    //auto t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    float64 pulse[sample_size_square_pulse];
    pulse[0] = voltage;
    pulse[1] = 0;
    DAQmxWriteAnalogF64(taskHandle_OUT, sample_size_square_pulse, 1, -1, DAQmx_Val_GroupByScanNumber, pulse, &written, NULL);
    DAQmxStartTask(taskHandle_OUT);
    DAQmxWaitUntilTaskDone(taskHandle_OUT,0.1);
    DAQmxStopTask(taskHandle_OUT);
    //auto t_check3 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    //double t_comp = (t_check3- t_check1)/1000.0;
    //std::cout << t_comp << "ms" << std::endl;
}

void NIControl::NI_GENERATE_SQUARE_PULSE2(float64 voltage){
    auto t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    float64 data_OUT[1];
    data_OUT[0] = voltage;
    DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL);
    auto t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    while(t_check2 - t_check1 < 1000){
        t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
    data_OUT[0] = 0;
    DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL);
    //auto t_check3 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    //double t_comp = (t_check3- t_check1)/1000.0;
    //std::cout << t_comp << "ms" << std::endl;
}

void NIControl::NI_GENERATE_SAWTOOTH_PULSE(float64 voltage){
    // It takes 1ms more than square pulse.
    auto t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    float64 pulse[sample_size_sawtooth_pulse];
    for(int i=0; i<sample_size_sawtooth_pulse/2; i++){
        pulse[i] = voltage;
    }
    for(int i=sample_size_sawtooth_pulse/2; i<sample_size_sawtooth_pulse; i++){
        pulse[i] = voltage-voltage*(i+1-sample_size_sawtooth_pulse/2)/(sample_size_sawtooth_pulse/2);
    }
    DAQmxWriteAnalogF64(taskHandle_OUT, sample_size_sawtooth_pulse, 1, -1, DAQmx_Val_GroupByScanNumber, pulse, &written, NULL);
    DAQmxStartTask(taskHandle_OUT);
    DAQmxWaitUntilTaskDone(taskHandle_OUT,0.1);
    DAQmxStopTask(taskHandle_OUT);
    auto t_check3 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    double t_comp = (t_check3- t_check1)/1000.0;
    std::cout << t_comp << "ms" << std::endl;

}

void NIControl::NI_GENERATE_SAWTOOTH_PULSE2(float64 voltage){

    auto t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    //float64 data_OUT[1];
    //data_OUT[0] = voltage;
    DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, &voltage, &written, NULL);
    auto t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    float64 tmp[1];
    double interval = 1000.0/int(sample_size_sawtooth_pulse/2);
    while(t_check2 - t_check1 < 1000){
        t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    for(int i=0; i<int(sample_size_sawtooth_pulse/2); i++){
        t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        tmp[0] = voltage-voltage*(i+1)/(int(sample_size_sawtooth_pulse/2));
        DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, tmp, &written, NULL);
        while(t_check2 - t_check1 < interval){
            t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
    }

    //auto t_check3 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    //double t_comp = (t_check3- t_check1)/1000.0;
    //std::cout << t_comp << "ms" << std::endl;
}

void NIControl::NI_GENERATE_SAWTOOTH_PULSE2(float64 voltage, double duration){

    auto t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    float64 data_OUT[1];
    data_OUT[0] = voltage;
    DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL);
    auto t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    while(t_check2 - t_check1 < 1000){
        t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
    float64 tmp[1];
    for(int i=0; i<int(sample_size_sawtooth_pulse/2); i++){
        auto t_check4 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        tmp[0] = voltage-voltage*(i+1)/(int(sample_size_sawtooth_pulse/2));
        DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, tmp, &written, NULL);
        while(t_check2 - t_check4 < 1000.0/int(sample_size_sawtooth_pulse/2)*duration){
            t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
    }
    //auto t_check3 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    //double t_comp = (t_check3- t_check1)/1000.0;
    //std::cout << t_comp << "ms" << std::endl;
}

void NIControl::NI_GENERATE_SAWTOOTH_PULSE3(float64 voltage){
    auto t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    float64 data_OUT[1];
    data_OUT[0] = voltage;
    DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL);
    auto t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    while(t_check2 - t_check1 < 1000){
        t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
    for(int i=0; i<int(sample_size_sawtooth_pulse/2); i++){
        t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        //data_OUT[0] = voltage-voltage*(i+1)/(int(sample_size_sawtooth_pulse/2));
        data_OUT[0] = voltage*(int(sample_size_sawtooth_pulse/2)-i-1)/int(sample_size_sawtooth_pulse/2);
        DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL);
        while(t_check2 - t_check1 < 100){
            t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
    }


    /*auto t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    float64 tmp[1];
    for(int i=0; i<int(sample_size_sawtooth_pulse); i++){
        auto t_check4 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        if(i < int(sample_size_sawtooth_pulse/2)){
           tmp[0] = voltage;
        }else{
           tmp[0] = voltage-voltage*(i+1-int(sample_size_sawtooth_pulse/2))/(int(sample_size_sawtooth_pulse/2));
        }
        std::cout << tmp[0] << " V" << std::endl;
        DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, tmp, &written, NULL);
        while(t_check2 - t_check4 < 1000.0/int(sample_size_sawtooth_pulse/2)){
            t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
    }*/
}

void NIControl::NI_WRITE(float64 voltage)
{
    float64 data_OUT[1];
    data_OUT[0] = voltage;
    if( taskHandle_OUT2!=0 ) {
        DAQmxStopTask(taskHandle_OUT2);
        DAQmxClearTask(taskHandle_OUT2);
    }
    DAQmxCreateTask("", &taskHandle_OUT2);
    DAQmxCreateAOVoltageChan(taskHandle_OUT2, output_physical_channel2, "", 0.0, 10.0, DAQmx_Val_Volts, NULL);
    DAQmxCfgSampClkTiming(taskHandle_OUT2,"",1000.0/1.0,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,1);
    DAQmxWriteAnalogF64(taskHandle_OUT2, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL);
    DAQmxStartTask(taskHandle_OUT2);
    DAQmxWaitUntilTaskDone(taskHandle_OUT2,0.1);
    DAQmxStopTask(taskHandle_OUT2);
}

void NIControl::NI_WRITE2(float64 voltage)
{
    float64 data_OUT[1];
    data_OUT[0] = voltage;
    DAQmxWriteAnalogF64(taskHandle_OUT2,1,1,-1,DAQmx_Val_GroupByChannel,data_OUT,&written,NULL);
    //DAQmxStartTask(taskHandle_OUT2);
    //DAQmxWaitUntilTaskDone(taskHandle_OUT2,0.1);
    //DAQmxStopTask(taskHandle_OUT2);
}

void NIControl::NI_WRITE3(float64 voltage)
{
    float64 data_OUT[1];
    data_OUT[0] = voltage;
    DAQmxWriteAnalogF64(taskHandle_OUT,1,1,-1,DAQmx_Val_GroupByChannel,data_OUT,&written,NULL);
}

