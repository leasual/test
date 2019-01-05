
/*! \file alignment_detection.h
*  \brief 在系统设定的校准时间内，对输入的角度进行校准
*  \author wangjian
*  \date 2018-4-27
*/

#ifndef DSM_STRATEGY_ALIGNMENT_DETECTION_H
#define DSM_STRATEGY_ALIGNMENT_DETECTION_H

#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <tuple>
#include <opencv2/opencv.hpp>
#include <numeric>

///// \brief 校准后输出的结果类型
//struct Calibrate{
//    float pitch;    ///< 校准后的角度值
//    float yaw;      ///< 校准后的参数值
//    double delta_time;   ///< 已经运行的校准时间
//};


/*! \brief 在校准时间内，根据输入的角度值计算校准后的值
 *
 *  校准方式是将角度范围划分为多个区间，然后计算每个区间内的角度数量，取数量前10的区间，
 *  计算落在这些区间内的角度的平均值
 */
class AlignmentDetection {
public:
    /// \brief 构造函数
    AlignmentDetection(int alignment_time = 0);

    /// \brief 析构函数
    virtual ~AlignmentDetection();

    /// \brief 校准角度的接口
    /// \param [in] pitch 垂直方向的角度
    /// \param [in] yaw 水平方向的角度
    cv::Vec3d Alignment(float pitch, float yaw);

    /// \brief 在校准过程中，如果检测不到人脸，则需要暂停计时器，直到找到人脸位置
    void Pause();

    /// \brief 设置参数，主要为校准时间参数
    /// \param alignement_time 校准时间
    void SetParam(int alignment_time);

private:
    /// \brief 计算数组的平均值
    /// \param [in] data_list 要计算的角度数组
    /// \retval 计算得到的数组平均值
    float CalAverage(const std::vector<float>& data_list) const;

    /// \brief 计算角度数组分布数量最大的10个区间的平均值，作为校准后的角度值
    /// \param [in] data_list 角度数组
    /// \param [in] interval_count 区间数量
    /// \retval 校准后的角度值
    float CalDistributionMax10Mean(const std::vector<float>& data_list, int interval_count);

private:
    bool first_read_time_;   ///<
    std::chrono::steady_clock::time_point start_time_;  ///< 校准开始时间
    std::chrono::steady_clock::time_point end_time_;      ///< 校准结束时间
    std::chrono::seconds diff_time_;        ///< 校准持续时间
    int threshold_time_;        ///<
    std::vector<float> pitch_list_;     ///< 垂直方向的角度列表
    std::vector<float> yaw_list_;         ///< 水平方向的角度列表
    float pitch_thres_;     ///<
    float yaw_thres_;       ///<
};


#endif //DSM_STRATEGY_ALIGNMENT_DETECTION_H
