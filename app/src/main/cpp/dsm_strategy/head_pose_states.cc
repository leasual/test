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

bool HeadPoseStatus::GetHeadLeftRightStatus(float yaw) {
    return yaw > hori_left_thres_ or yaw < hori_right_thres_;
}

bool HeadPoseStatus::GetHeadUpDownStatus(float pitch) {
    return pitch < vert_upper_thres_ or  pitch > vert_bottom_thres_;
}

