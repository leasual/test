//
// Created by wangjian on 18-4-27.
//

#include "dsm_strategy/judgement_strategy/alignment_detection.h"

AlignmentDetection::AlignmentDetection(int alignment_time) {
    first_read_time_ = true;
    threshold_time_ = alignment_time;
    pitch_thres_ = 0.0f;
    yaw_thres_ = 0.0f;
}


AlignmentDetection::~AlignmentDetection() {

}


float AlignmentDetection::CalAverage(const std::vector<float> &data_list) const {
    if (data_list.empty()) {
        return 0.0f;
    } else {
        return std::accumulate(data_list.begin(), data_list.end(), 0.0f) / data_list.size();
    }
}


float AlignmentDetection::CalDistributionMax10Mean(const std::vector<float> &origin_data_list, int interval_count) {
    std::vector<float> data_list(origin_data_list);

    //对区间值进行排序
    std::sort(data_list.begin(), data_list.end());

    //计算区间大小
    float delta = (data_list.back() - data_list.front()) / interval_count;
    //定义区间
    std::vector<int> interval(interval_count, 0);
    int i = 1;
    //查找每个区间内落入的数据个数
    for (std::vector<float>::const_iterator iter = data_list.begin(); iter != data_list.end(); iter++) {
        if (*iter <= (data_list[0] + i * delta)) {
            interval[i - 1] = interval[i - 1] + 1;
        } else {
            interval[i] = interval[i] + 1;
            i++;
        }
    }
    //对落入区间的个数进行排序
    std::multimap<int, int> pdict;
    for (int j = 0; j < interval.size(); j++) {
        pdict.insert(std::pair<int, int>(interval[j], j));
    }
    //获取最大的前十个区间索引
    std::vector<int> pindex(10, 0);
    int k = 0;
    std::multimap<int, int>::reverse_iterator rpIter;
    for (rpIter = pdict.rbegin(); rpIter != pdict.rend(); rpIter++) {
        if (k < 10) {
            pindex[k++] = rpIter->second;
        } else {
            break;
        }
    }
    //获取相应索引对应的区间均值
    std::vector<float> pdata;
    for (int m = 0; m < pindex.size(); m++) {
        pdata.push_back(((data_list[0] + pindex[m] * delta) + (data_list[0] + (pindex[m] + 1) * delta)) / 2);
    }
    //获取所有值的均值
    float result = CalAverage(pdata);

    return result;
}

cv::Vec3d AlignmentDetection::Alignment(float pitch, float yaw) {
    if (first_read_time_) {
        first_read_time_ = false;
        start_time_ = std::chrono::steady_clock::now();
    }
    pitch_list_.push_back(pitch);
    yaw_list_.push_back(yaw);

    end_time_ = std::chrono::steady_clock::now();
//    diffTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
    diff_time_ = std::chrono::duration_cast<std::chrono::seconds>(end_time_ - start_time_);
//    std::cout << "(AlignmentDetection)the time is: " << diffTime << std::endl;
    if (diff_time_.count() >= threshold_time_) {
        //校准眼睛和嘴巴
//        status.feed(lm, true);
        //计算头上下的基准值
        pitch_thres_ = CalDistributionMax10Mean(pitch_list_, 70);
        //计算头左右的基准值
        yaw_thres_ = CalDistributionMax10Mean(yaw_list_, 40);
        return cv::Vec3d(pitch_thres_,yaw_thres_,std::chrono::duration_cast<std::chrono::seconds>(diff_time_).count());
    } else {
        cv::Vec3d ret;
        ret[2] = diff_time_.count();
        return ret;
    }
}


void AlignmentDetection::Pause() {
    start_time_ = std::chrono::steady_clock::now() - diff_time_;
}


void AlignmentDetection::SetParam(int alignment_time) {
    threshold_time_ = alignment_time;
}
