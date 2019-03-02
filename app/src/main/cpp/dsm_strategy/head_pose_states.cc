//
// Created by untouch on 19-1-16.
//

#include "head_pose_states.h"

void HeadPoseStatus::SetParam(float left_diff, float right_diff, float up_diff, float down_diff) {
    vert_upper_thres_ = up_diff;
    vert_bottom_thres_ = down_diff;

    hori_left_thres_ = left_diff;
    hori_right_thres_ = right_diff;
}

bool HeadPoseStatus::GetHeadStatus(float pitch, float yaw) {
    bool pitch_state = pitch < vert_upper_thres_ or  pitch > vert_bottom_thres_;
    bool yaw_state = yaw > hori_left_thres_ or yaw < hori_right_thres_;

    return pitch_state or yaw_state;
}


