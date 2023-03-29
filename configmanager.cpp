#include "configmanager.h"

std::string ConfigManager::search_conf(std::string optionBuf)
{
    default_conf();
        //VAR
    std::string lineBuf;
    std::ifstream confFile(conf_path);
    std::string search_result = "-1";
    optionBuf += " = ";
    if ( confFile.is_open() )
    {
        while ( getline( confFile, lineBuf ) )
        {
            if ( ( ( int )lineBuf.find( optionBuf ) ) != -1 )
            {
                lineBuf.erase( 0, optionBuf.length() );
                search_result =  lineBuf;
            }

        }
        confFile.close();
    }
    return search_result;
}
int ConfigManager::StringtoInt(std::string txt)
{
        return atoi(txt.c_str());
}

double ConfigManager::StringtoDouble(std::string txt)
{
        return atof(txt.c_str());
}

void ConfigManager::update_conf(int system_latency_us, int filter_size_min, int filter_size_max, int crop_size_width,  int crop_size_height, int threshold_value, int target_cell_no1, int target_cell_no2, int debug_fps, int gap_um, int pulse_repetition_interval, int sorting_line1, int sample_air_pressure, double pulse_on_voltage, double pulse_voltage1, double pulse_voltage2, std::string debug_background_img_path, std::string debug_dir_path)
{

    std::ofstream out(conf_path);
    if(out)
    {
        out << "[IMAGE_PROCESSING]\n";
        out << "FILTER_SIZE_MIN = " << filter_size_min << "\n";
        out << "FILTER_SIZE_MAX = " << filter_size_max << "\n";
        out << "THRESHOLD_VALUE = " << threshold_value << "\n";
        out << "CROP_SIZE_WIDTH = " << crop_size_width << "\n";
        out << "CROP_SIZE_HEIGTH = " << crop_size_height << "\n";
        out << "[SORTING]\n";
        out << "TARGET_CELL_NO1 = " << target_cell_no1 << "\n";
        out << "PULSE_VOLTAGE1 = " << pulse_voltage1 << "\n";
        out << "TARGET_CELL_NO2 = " << target_cell_no2 << "\n";
        out << "PULSE_VOLTAGE2 = " << pulse_voltage2 << "\n";
        out << "GAP_UM = " << gap_um << "\n";
        out << "SYSTEM_LATENCY_US = " << system_latency_us << "\n";
        out << "SORTING_LINE1 = " << sorting_line1 << "\n";
        out << "[SAMPLE_INJECTION]\n";
        out << "SAMPLE_AIR_PRESSURE = " << sample_air_pressure << "\n";
        out << "[PULSE_GENERATION]\n";
        out << "PULSE_REPETITION_INTERVAL = " << pulse_repetition_interval << "\n";
        out << "PULSE_ON_VOLTAGE = " << pulse_on_voltage << "\n";
        out << "[DEBUG]\n";
        out << "DEBUG_FPS = " << debug_fps << "\n";
        out << "DEBUG_BACKGROUND_IMG_PATH = " << debug_background_img_path << "\n";
        out << "DEBUG_DIR_PATH = " << debug_dir_path << "\n";
        out.close();
    }
}

void ConfigManager::default_conf()
{
    std::ifstream infile(conf_path);
    if (!infile.good()){
        reset_conf();
    }
}
void ConfigManager::reset_conf()
{
    std::ofstream out(conf_path);
    if(out)
    {
        out << "[IMAGE_PROCESSING]\n";
        out << "FILTER_SIZE_MIN = " << 5 << "\n";
        out << "FILTER_SIZE_MAX = " << 100 << "\n";
        out << "THRESHOLD_VALUE = " << 10 << "\n";
        out << "CROP_SIZE_WIDTH = " << 50 << "\n";
        out << "CROP_SIZE_HEIGTH = " << 50 << "\n";
        out << "[SORTING]\n";
        out << "TARGET_CELL_NO1 = " << -1 << "\n";
        out << "PULSE_VOLTAGE1 = " << 1 << "\n";
        out << "TARGET_CELL_NO2 = " << -1 << "\n";
        out << "PULSE_VOLTAGE2 = " << 1 << "\n";
        out << "GAP_UM = " << 500 << "\n";
        out << "SYSTEM_LATENCY_US = " << 3000 << "\n";
        out << "SORTING_LINE1 = " << 630 << "\n";
        out << "[SAMPLE_INJECTION]\n";
        out << "SAMPLE_AIR_PRESSURE = " << 0 << "\n";
        out << "[PULSE_GENERATION]\n";
        out << "PULSE_REPETITION_INTERVAL = " << 250 << "\n";
        out << "PULSE_ON_VOLTAGE = " << 1 << "\n";
        out << "[DEBUG]\n";
        out << "DEBUG_FPS = " << 480 << "\n";
        out << "DEBUG_BACKGROUND_IMG_PATH = " << "" << "\n";
        out << "DEBUG_DIR_PATH = " << "" << "\n";
        out.close();
    }
}
