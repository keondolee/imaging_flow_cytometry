#ifndef MYTRACKER_V2_H
#define MYTRACKER_V2_H

#include "defines.h"

class mytracker_v2
{
public:
    mytracker_v2();
    ~mytracker_v2();

    void update_boundingbox(int, int, int, int);
    void update_frame_interval(double);
    void update_previous_trackings(tracking_t);
    void update_class_id(int track_id, int class_id);
    void update_sort_status(int);
    void update_nxt_pos(int, double);
    void reset_previous_tracking();
    tracking_t get_previous_trackings();

private:
    int boundx1{0};
    int boundy1{0};
    int boundx2{0};
    int boundy2{0};
    int track_id_totalnum{0};
    double frame_interval{1000}; //us
    double max_skip_num{0.0};
    tracking_t previous_trackings;

};

#endif // MYTRACKER_V2_H
