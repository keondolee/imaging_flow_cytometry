#include "mytracker_v2.h"

bool compareTrack(const Trackinginfo &rects1, const Trackinginfo &rects2)
{
    return (rects1.rect.x > rects2.rect.x);
}


mytracker_v2::mytracker_v2()
{

}

mytracker_v2::~mytracker_v2()
{

}

void mytracker_v2::update_boundingbox(int boundx1, int boundy1, int boundx2, int boundy2){
    this->boundx1 = boundx1;
    this->boundy1 = boundy1;
    this->boundx2 = boundx2;
    this->boundy2 = boundy2;
}

void mytracker_v2::update_previous_trackings(tracking_t current_trackings){
    std::sort(current_trackings.begin(), current_trackings.end(), compareTrack); // 300 296 280 ...
    std::sort(previous_trackings.begin(), previous_trackings.end(), compareTrack); // 300 296 280 ...

    for(int i = 0; i < int(previous_trackings.size()); i++){
        // Case.2 Since we don't know the speed of the particles that just appeared,
        // we assume that the rightmost particle on the image is the particle.
        if(previous_trackings[i].centerPositions[0].size()==1 && !previous_trackings[i].is_not_tracked){
            int u_j = -1, max_cnt_x = 0;
            double tolerance_y = 2.0;
            double u_cnt_x, u_cnt_y;
            for(int j = 0; j < int(current_trackings.size()); j++){
                if(current_trackings[j].is_used_tracking == false){
                    double cnt_x = current_trackings[j].rect.x + 0.5*current_trackings[j].rect.width;
                    double cnt_y = current_trackings[j].rect.y + 0.5*current_trackings[j].rect.height;
                    if(previous_trackings[i].center[1] - previous_trackings[i].rect.height/tolerance_y < cnt_y &&
                            previous_trackings[i].center[1] + previous_trackings[i].rect.height/tolerance_y > cnt_y &&
                            previous_trackings[i].center[0] - 3 <= cnt_x){
                        // moving direction : left -> right
                        if(cnt_x >= max_cnt_x){
                            max_cnt_x = cnt_x;
                            u_j = j;
                            u_cnt_x = cnt_x;
                            u_cnt_y = cnt_y;
                        }
                    }
                }
            }

            if(u_j != -1){
                double time_lapsed = std::chrono::duration_cast<std::chrono::microseconds>(current_trackings[u_j].curtime - previous_trackings[i].times[0]).count();
                double speed_x = (u_cnt_x - previous_trackings[i].centerPositions[0].at(0))/(time_lapsed);
                double speed_y = (u_cnt_y - previous_trackings[i].centerPositions[1].at(0))/(time_lapsed);

                // Update
                previous_trackings[i].area = current_trackings[u_j].area;
                previous_trackings[i].center[0] = u_cnt_x;
                previous_trackings[i].center[1] = u_cnt_y;
                previous_trackings[i].speed_x = speed_x;
                previous_trackings[i].speed_y = speed_y;
                previous_trackings[i].next_center[0] = u_cnt_x+speed_x*frame_interval;
                previous_trackings[i].next_center[1] = u_cnt_y+speed_y*frame_interval;
                previous_trackings[i].rect = current_trackings[u_j].rect;
                previous_trackings[i].cropped_img = current_trackings[u_j].cropped_img;
                previous_trackings[i].centerPositions[0].push_back(u_cnt_x);
                previous_trackings[i].centerPositions[1].push_back(u_cnt_y);
                previous_trackings[i].curtime = current_trackings[u_j].curtime;
                previous_trackings[i].times.push_back(current_trackings[u_j].curtime);
                previous_trackings[i].skip_frames = 0;
                //if(previous_trackings[i].class_id == -1) previous_trackings[i].is_classified = false;
                current_trackings[u_j].is_used_tracking = true;
            }else{
                previous_trackings[i].skip_frames++; //?
            }

        }
        // Case.3 The particle closest to the next predicted position is the particle.
        else if(previous_trackings[i].centerPositions[0].size()>1 && !previous_trackings[i].is_not_tracked){
            int u_j = -1;
            double u_cnt_x, u_cnt_y;
            double dblLeastDistance = 30000.0, tolerance = 50.0;

            for(int j = 0; j < int(current_trackings.size()); j++){
                if(current_trackings[j].is_used_tracking) continue;
                double cnt_x = current_trackings[j].rect.x + 0.5*current_trackings[j].rect.width;
                double cnt_y = current_trackings[j].rect.y + 0.5*current_trackings[j].rect.height;
                double dblDistance = sqrt((previous_trackings[i].next_center[0] - cnt_x)*(previous_trackings[i].next_center[0] - cnt_x) + (previous_trackings[i].next_center[1] - cnt_y)*(previous_trackings[i].next_center[1] - cnt_y));
                // If next position is front of
                if(dblDistance < tolerance && dblDistance < dblLeastDistance && previous_trackings[i].center[0]-3 <= cnt_x){  // new
                    dblLeastDistance = dblDistance;
                    u_j = j;
                    u_cnt_x = cnt_x;
                    u_cnt_y = cnt_y;
                }
            }

            if(u_j != -1){
                double time_lapsed = std::chrono::duration_cast<std::chrono::microseconds>(current_trackings[u_j].curtime - previous_trackings[i].times[0]).count();
                double speed_x = (u_cnt_x - previous_trackings[i].centerPositions[0].at(0))/(time_lapsed);
                double speed_y = (u_cnt_y - previous_trackings[i].centerPositions[1].at(0))/(time_lapsed);

                // Update
                previous_trackings[i].area = current_trackings[u_j].area;
                previous_trackings[i].center[0] = u_cnt_x;
                previous_trackings[i].center[1] = u_cnt_y;
                previous_trackings[i].speed_x = speed_x;
                previous_trackings[i].speed_y = speed_y;
                previous_trackings[i].next_center[0] = u_cnt_x+speed_x*frame_interval;
                previous_trackings[i].next_center[1] = u_cnt_y+speed_y*frame_interval;
                previous_trackings[i].rect = current_trackings[u_j].rect;
                previous_trackings[i].cropped_img = current_trackings[u_j].cropped_img; //?
                previous_trackings[i].centerPositions[0].push_back(u_cnt_x);
                previous_trackings[i].centerPositions[1].push_back(u_cnt_y);
                previous_trackings[i].curtime = current_trackings[u_j].curtime;
                previous_trackings[i].times.push_back(current_trackings[u_j].curtime);
                previous_trackings[i].skip_frames = 0;
                //if(previous_trackings[i].class_id == -1) previous_trackings[i].is_classified = false;
                current_trackings[u_j].is_used_tracking = true;

                if(previous_trackings[i].centerPositions[0].size() > 5){
                    previous_trackings[i].centerPositions[0].erase(previous_trackings[i].centerPositions[0].begin());
                    previous_trackings[i].centerPositions[1].erase(previous_trackings[i].centerPositions[1].begin());
                    previous_trackings[i].times.erase(previous_trackings[i].times.begin());
                }
            }else{
                previous_trackings[i].skip_frames++;
            }

        }else if(previous_trackings[i].is_not_tracked){
            previous_trackings[i].skip_frames++;
        }else{
            std::cerr << "previous_trackings center size is 0!!!!" << std::endl;
        }
        // If skip_frame > 1, we don't track it.
        if(previous_trackings[i].skip_frames > 1){
            previous_trackings[i].is_not_tracked = true;
        }
        // If skip_frame > 5, we delete it.
        if(previous_trackings[i].skip_frames > 20){
            previous_trackings[i].is_time_to_remove = true;
        }
    }

    // Delete elements
    tracking_t::iterator it = previous_trackings.begin();
    while (it != previous_trackings.end()) {
        if(it->is_time_to_remove){
            it = previous_trackings.erase(it);
        }else{
            ++it;
        }
    }

    // Case.1 When a new particle apperars on the image, tracker registers it.
    for(int i = 0; i < int(current_trackings.size()); i++){
        if(current_trackings[i].is_used_tracking) continue;
        double cnt_x = current_trackings[i].rect.x + 0.5*current_trackings[i].rect.width;
        double cnt_y = current_trackings[i].rect.y + 0.5*current_trackings[i].rect.height;

        Trackinginfo tmp;
        tmp.track_id = track_id_totalnum++;
        tmp.area = current_trackings[i].area;
        tmp.cropped_img = current_trackings[i].cropped_img;
        tmp.rect = current_trackings[i].rect;
        tmp.center[0] = cnt_x;
        tmp.center[1] = cnt_y;
        tmp.next_center[0] = cnt_x;
        tmp.next_center[1] = cnt_y;
        tmp.centerPositions[0].push_back(cnt_x);
        tmp.centerPositions[1].push_back(cnt_y);
        tmp.curtime = current_trackings[i].curtime;
        tmp.times.push_back(current_trackings[i].curtime);
        tmp.is_not_tracked = false;
        tmp.skip_frames = 0;
        previous_trackings.push_back(tmp);
    }

}

void mytracker_v2::update_frame_interval(double arg){
    frame_interval = arg;
    //max_skip_num =  7000.0/frame_interval;
}

void mytracker_v2::update_class_id(int track_id, int class_id){
    for(int i=0; i<int(previous_trackings.size()); i++){
        if(previous_trackings[i].track_id == track_id){
            previous_trackings[i].class_id = class_id;
            previous_trackings[i].is_classified = true;
        }
    }
}

void mytracker_v2::update_sort_status(int track_id){
    for(int i=0; i<int(previous_trackings.size()); i++){
        if(previous_trackings[i].track_id == track_id){
           previous_trackings[i].is_sorted = true;
           previous_trackings[i].is_time_to_remove = true;
        }
    }
}

void mytracker_v2::update_nxt_pos(int track_id, double nxt_pos){
    for(int i=0; i<int(previous_trackings.size()); i++){
        if(previous_trackings[i].track_id == track_id){
           previous_trackings[i].sorting_nxt_pos = nxt_pos;
        }
    }
}

void mytracker_v2::reset_previous_tracking(){
    track_id_totalnum = 0;
    previous_trackings.clear();
}

tracking_t mytracker_v2::get_previous_trackings(){
    return previous_trackings;
}
