//
// Created by wangjian on 18-4-28.
//

#include "action_checker.h"

ActionChecker::ActionChecker() {
    // 规定动作列表
    // 注意：最后一个状态一定要为Action::END表示最后一个状态
    action_sequence_ = {Action::HEAD_LEFT, Action::HEAD_RIGHT, Action::END};
    cur_ruled_action_ = action_sequence_.begin();
    pre_action_ = Action::HEAD_MIDDLE;
    cur_count_ = 0;
    required_count_ = 4;

    head_left_thres_ = -20;
    head_right_thres_ = 20;
    head_up_thres_ = -30;
    head_down_thres_ = 12;
}


/**
 *
 * @param vert_angle
 * @param hori_angle
 * @return
 */
Action ActionChecker::checkAction(double vert_angle, double hori_angle) {
    Action cur_vert_action = Action::HEAD_MIDDLE;
    Action cur_hori_action = Action::HEAD_MIDDLE;

    // 判断垂直方向的状态
    if (vert_angle < head_up_thres_) {
        cur_vert_action = Action::HEAD_UP;
    } else if (vert_angle > head_down_thres_) {
        cur_vert_action = Action::HEAD_DOWN;
    }

    // 判断水平方向的状态
    if (hori_angle < head_left_thres_) {
        cur_hori_action = Action::HEAD_LEFT;
    } else if (hori_angle > head_right_thres_) {
        cur_hori_action = Action::HEAD_RIGHT;
    }

    // 如果当前水平方向和垂直方向都出现了偏移状态,则认为该状态无效
    // 该特性由产品经理确定
    if (cur_vert_action != Action::HEAD_MIDDLE and cur_hori_action != Action::HEAD_MIDDLE) {
        pre_action_ = Action::HEAD_MIDDLE;
        cur_count_ = 0;
        return *cur_ruled_action_;
    }
    if (cur_hori_action != Action::HEAD_MIDDLE) {
        if (cur_hori_action == pre_action_ and cur_hori_action == *cur_ruled_action_) {
            cur_count_++;
            if (cur_count_ >= required_count_) {
                cur_count_ = 0;
                cur_ruled_action_++;
            }
        } else {
            cur_count_ = 0;
        }
        pre_action_ = cur_hori_action;
    } else if (cur_vert_action != Action::HEAD_MIDDLE) {
        if (cur_vert_action == pre_action_ and cur_vert_action == *cur_ruled_action_) {
            cur_count_++;
            if (cur_count_ >= required_count_) {
                cur_count_ = 0;
                cur_ruled_action_++;
            }
        } else {
            cur_count_ = 0;
        }
        pre_action_ = cur_hori_action;
    } else {
        pre_action_ = Action::HEAD_MIDDLE;
        cur_count_ = 0;
    }
    return *cur_ruled_action_;


}