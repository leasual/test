/*! \file types.h
*  \brief 定义DSM项目中使用到的各种枚举类型
*  \author wangjian zhangbo
*  \date 2018-5-22
*/
#ifndef STRATEGY_ACTION_H
#define STRATEGY_ACTION_H

#include <iostream>
#include <vector>
#include <cstring>

#include "enum_types.h"

//! \brief 根据输入的角度判断用户需要执行的下一个动作,动作顺序提前指定
/*! \details 在构造函数中指定用户需要执行的规定动作顺序,每次都根据输入的角度判断用户
 * 是否完成了当前
 *
 */
class ActionChecker {
public:
    //! \brief 构造函数
    ActionChecker();


    //! \brief 根据用户当前角度判断要执行的下一个动作
    //! \param[in] vert_angle 用户头部在垂直方向的旋转角度
    //! \param[in] hori_angle 用户头部在水平方向的旋转角度
    //! \param[out] 用户需要执行的下一个动作,返回Action::END则表示已经完成规定动作
    Action checkAction(double vert_angle, double hori_angle);

private:
    std::vector<Action> action_sequence_;  // 动作序列组合
    std::vector<Action>::iterator cur_ruled_action_;   //当前规定动作
    Action pre_action_;     // 上一个动作
    int cur_count_;         //当前动作的连续次数
    int required_count_;     //需要的连续次数

    // 临界值
    int head_left_thres_;
    int head_right_thres_;
    int head_up_thres_;
    int head_down_thres_;
};


#endif //STRATEGY_ACTION_H
