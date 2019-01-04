//
// Created by slam on 18-10-24.
//
#include "transform.h"

void findNonreflectiveSimilarity(const cv::Mat src, const cv::Mat dst, cv::Mat &trans, cv::Mat &trans_inv)
{
    int K = 2;
    int M = 5;
    cv::Mat x = dst.col(0);
    cv::Mat y = dst.col(1);

    cv::Mat tmp1 = cv::Mat::zeros(5, 4, CV_32F);
    x.copyTo(tmp1.col(0));
    y.copyTo(tmp1.col(1));
    tmp1.col(2) = cv::Mat::ones(5, 1, CV_32F);
    tmp1.col(3) = cv::Mat::zeros(5, 1, CV_32F);

    cv::Mat tmp2 = cv::Mat::zeros(5, 4, CV_32F);
    y.copyTo(tmp2.col(0));
    cv::Mat x_ = -x;
    x_.copyTo(tmp2.col(1));
    tmp2.col(2) = cv::Mat::zeros(5, 1, CV_32F);
    tmp2.col(3) = cv::Mat::ones(5, 1, CV_32F);

    cv::Mat X = cv::Mat::zeros(5 * 2, 4, CV_32F);
    tmp1.copyTo(X.rowRange(0, 5));
    tmp2.copyTo(X.rowRange(5, 10));

    cv::Mat U = cv::Mat::zeros(5 * 2, 1, CV_32F);
    src.col(0).copyTo(U.rowRange(0, 5));
    src.col(1).copyTo(U.rowRange(5, 10));


    cv::Mat r;
    cv::solve(X, U, r, CV_SVD );
    cv::Mat U_new = X * r;
    //r = r.reshape(0, 1);
    float sc = r.at<float>(0,0);
    float ss = r.at<float>(1,0);
    float tx = r.at<float>(2,0);
    float ty = r.at<float>(3,0);

    cv::Mat Tinv = cv::Mat::zeros(3, 3, CV_32F);
    Tinv.at<float>(0,0) = sc;
    Tinv.at<float>(0,1) = -ss;
    Tinv.at<float>(0,2) = 0.f;
    Tinv.at<float>(1,0) = ss;
    Tinv.at<float>(1,1) = sc;
    Tinv.at<float>(1,2) = 0.f;
    Tinv.at<float>(2,0) = tx;
    Tinv.at<float>(2,1) = ty;
    Tinv.at<float>(2,2) = 1.f;

    cv::Mat T = Tinv.inv();
    T.at<float>(0,2) = 0.f;
    T.at<float>(1,2) = 0.f;
    T.at<float>(2,2) = 1.f;
    trans = T.clone();
    trans_inv = Tinv.clone();
}
void tformfwd(const cv::Mat trans, const cv::Mat uv, cv::Mat &xy)
{
    cv::Mat uv_ = cv::Mat::ones(uv.rows, uv.cols + 1, CV_32F);
    uv.copyTo(uv_.colRange(0,2));
    xy = uv_ * trans;
    xy = xy.colRange(0, xy.cols - 1).clone();
}
cv::Mat myGetAffineTransform( const std::vector<cv::Point2f> &src_pts, const std::vector<cv::Point2f> &dst_pts)
{

    cv::Mat uv = cv::Mat::zeros(5, 2, CV_32F);
    cv::Mat xy = cv::Mat::zeros(5, 2, CV_32F);
    for(int r = 0; r < 5; r++)
    {
        uv.at<float>(r, 0) = src_pts[r].x;
        uv.at<float>(r, 1) = src_pts[r].y;
        xy.at<float>(r, 0) = dst_pts[r].x;
        xy.at<float>(r, 1) = dst_pts[r].y;
    }
    cv::Mat trans1, trans1_inv;
    findNonreflectiveSimilarity(uv, xy, trans1, trans1_inv);
    cv::Mat trans2r, trans2r_inv;
    cv::Mat xyR = xy;
    xyR.col(0) = -xyR.col(0);
    findNonreflectiveSimilarity(uv, xyR, trans2r, trans2r_inv);
    cv::Mat TreflectY = cv::Mat::zeros(3, 3, CV_32F);
    TreflectY.at<float>(0,0) = -1;
    TreflectY.at<float>(1,1) = 1;
    TreflectY.at<float>(2,2) = 1;
    cv::Mat trans2 = trans2r * TreflectY;

    cv::Mat xy1, xy2;
    tformfwd(trans1, uv, xy1);
    float norm1 = norm((xy1 - xy), CV_L2);
    tformfwd(trans2, uv, xy2);
    float norm2 = norm((xy2 - xy), CV_L2);

    cv::Mat trans;
    if(norm1 <= norm2)
        trans = trans1;
    else
        trans = trans2;
    cv::Mat tfm = trans.colRange(0,2).t();
    return tfm;
}

