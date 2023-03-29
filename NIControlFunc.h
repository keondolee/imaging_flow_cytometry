#pragma once
#include "NIDAQmx.h"

// ADC
TaskHandle 	taskHandle_OUT = 0;
TaskHandle 	taskHandle_IN = 0;
#define DATA_IN 1000
int num_of_output_channel = 2; // 총 사용할 채널. 즉 num_of_input_channel * 1000 = DATA_IN
int SampsPerChan = 1000; // 채널 하나당 받을 샘플
int sampling_rate = 1000; // 1초당 1000개의 샘플 받음
float64 data_OUT[2];
int32 written = 0;
const char* output_physical_channel = "Dev2/ao0";
bool init = false; 

void NI_INIT()
{
	// INPUT
	//	DAQmxCreateTask("",&taskHandle_IN);
	//	DAQmxCreateAIVoltageChan(taskHandle_IN,input_physical_channel,"",terminal_config ,-10.0,10.0,DAQmx_Val_Volts,NULL);
	//	DAQmxCfgSampClkTiming(taskHandle_IN,"",sampling_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,SampsPerChan);

	// OUTPUT
	DAQmxCreateTask("", &taskHandle_OUT);
	DAQmxCreateAOVoltageChan(taskHandle_OUT, output_physical_channel, "", -10.0, 10.0, DAQmx_Val_Volts, NULL);


}

void NI_INIT_AI() {
	DAQmxCreateTask("", &taskHandle_IN);
	DAQmxCreateAIVoltageChan(taskHandle_IN, "Dev3/ai0", "", DAQmx_Val_Diff, 0.0, 5.0, DAQmx_Val_Volts, NULL);
	DAQmxCfgSampClkTiming(taskHandle_IN, "", 3000.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 3000); // last buffer_size
	DAQmxStartTask(taskHandle_IN);
}

bool NI_READ_AI() {
	float64 data[3]; //1s?
	int32   read;
	DAQmxReadAnalogF64(taskHandle_IN, -1, 1.0, DAQmx_Val_GroupByChannel, data, 3, &read, NULL);

	for (int i = 0; i < 3; i++) {
		if (data[i] > 0.5) {
			std::cout << data[i] << std::endl;
			return true;
		}
	}
	return false;
}

void NI_STOP_AI() {
	if (taskHandle_IN != 0) {
		/*********************************************/
		/*/ DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle_IN);
		DAQmxClearTask(taskHandle_IN);
	}
}


void NI_START()
{
	//start
	//	DAQmxStartTask(taskHandle_IN);
	//	DAQmxSetReadRelativeTo(taskHandle_IN,DAQmx_Val_MostRecentSamp);
	//	DAQmxSetReadOffset(taskHandle_IN,-1);
	DAQmxStartTask(taskHandle_OUT);
	//printf("Acquiring samples continuously.\n");
}


void NI_CLEAR()
{
	//	DAQmxClearTask (taskHandle_IN);
	for (int i = 0; i<num_of_output_channel; i++)
	{
		data_OUT[i] = 0;
	}
	(DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL));
	DAQmxClearTask(taskHandle_OUT);
}

void NI_WRITE()
{
	for (int i = 0; i<num_of_output_channel; i++)
	{
		data_OUT[i] = 0;
	}
	// -1 :inifinite wait
	(DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL));
	// 핸들,		샘플 수, 오토 스타트, 대기 시간, 그룹, 데이터, 확인, null

}
void NI_WRITE(float64 voltage)
{
	for (int i = 0; i<num_of_output_channel; i++)
	{
		data_OUT[i] = voltage;
	}
	// -1 :inifinite wait
	(DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL));
	// 핸들,		샘플 수, 오토 스타트, 대기 시간, 그룹, 데이터, 확인, null

}

void NI_WRITE(float64 voltage0, float64 voltage1)
{

	data_OUT[0] = voltage0;
	data_OUT[1] = voltage1;
	(DAQmxWriteAnalogF64(taskHandle_OUT, 1, 1, -1, DAQmx_Val_GroupByScanNumber, data_OUT, &written, NULL));
	// 핸들,		샘플 수, 오토 스타트, 대기 시간, 그룹, 데이터, 확인, null

}

