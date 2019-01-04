/*
 * File:   stats_basic.hpp
 * Author: xieyi
 * Maintainer : zhanhe
 *
 * Created on 2009年10月28日, 下午7:58
 */

#ifndef _STATS_BASIC_HPP
#define	_STATS_BASIC_HPP

#include <numeric>
#include <cmath>
#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>

/** This file will not be given */
using namespace std;

/*
 * 计算期望
 */
template<class Iter_T>
typename Iter_T::value_type computeMean(Iter_T first,Iter_T last) {
  size_t cnt = distance(first, last);
  typename Iter_T::value_type sum = accumulate(first, last, typename Iter_T::value_type( ));
  typename Iter_T::value_type mean = sum / cnt;
  return mean;
}

template<int N, class T>
T nthPower(T x) {
  T ret = x;
  for (int i=1; i < N; ++i) {
    ret *= x;
  }
  return ret;
}

template<class T, int N>
struct SumDiffNthPower {
  SumDiffNthPower(T x) : mean_(x) { };
  T operator( )(T sum, T current) {
    return sum + nthPower<N>(current - mean_);
  }
  T mean_;
};

/*
 * 计算n阶中心矩
 */
template<class T, int N, class Iter_T>
T nthMoment(Iter_T first, Iter_T last, T mean)  {
  size_t cnt = distance(first, last);
  return accumulate(first, last, T( ), SumDiffNthPower<T, N>(mean)) / cnt;
}

/*
 * 计算方差
 */
template<class T, class Iter_T>
T computeVariance(Iter_T first, Iter_T last, T mean) {
  return nthMoment<T, 2>(first, last, mean);
}

/*
 * 计算标准差
 */
template<class T, class Iter_T>
T computeStdDev(Iter_T first, Iter_T last, T mean) {
  return sqrt(computeVariance(first, last, mean));
}

/*
 * 计算偏度
 */
template<class T, class Iter_T>
T computeSkew(Iter_T begin, Iter_T end, T mean) {
  T m3 = nthMoment<T, 3>(begin, end, mean);
  T m2 = nthMoment<T, 2>(begin, end, mean);
  return m3 / (m2 * sqrt(m2));
}

/*
 * 计算峰度
 */
template<class T, class Iter_T>
T computeKurtosisExcess(Iter_T begin, Iter_T end, T mean) {
  T m4 = nthMoment<T, 4>(begin, end, mean);
  T m2 = nthMoment<T, 2>(begin, end, mean);
  return m4 / (m2 * m2) - 3;
}

template<class T, class Iter_T>
void computeStats(Iter_T first, Iter_T last, T& sum, T& mean,
  T& var, T& std_dev, T& skew, T& kurt)
{
  mean = computeMean(first, last);
  var = computeVariance(first, last, mean);
  std_dev = sqrt(var);
  skew = computeSkew(first, last, mean);
  kurt = computeKurtosisExcess(first, last, mean);
}


float norm(const cv::Point2f& p1,const cv::Point2f& p2) {
  return sqrt((p1.x - p2.x) * (p1.x - p2.x) +
	      (p1.y - p2.y) * (p1.y - p2.y)
	      );
}

#endif	/* _STATS_BASIC_H */
