//
// Created by slam on 18-10-24.
//
#include <align_method.h>
#include <transform.h>

////////////////////////////////////////////////  FivePtsAlign  ////////////////////////////////////////////////////
FivePtsAlign::FivePtsAlign(const cv::Mat &im, const std::vector<cv::Point2f> &pts, const cv::Size &size):
        AlignMethod(),
        _im(im),
        _pts(pts),
        _size(size) {}

FivePtsAlign::FivePtsAlign(const cv::Size &size):
        AlignMethod(),
        _size(size) {}
FivePtsAlign::FivePtsAlign(const int w, const int h):
        AlignMethod(),
        _size(cv::Size(w, h)){}


void FivePtsAlign::set_im(const cv::Mat &im) {
    this->_im = im;
}

void FivePtsAlign::set_pts(const std::vector<cv::Point2f> &pts) {
    this->_pts = pts;
}

cv::Mat FivePtsAlign::Align() {
    std::vector<cv::Point2f> ref_pts(5);
//    ref_pts[0] = cv::Point2f(38.29459953, 51.69630051);
//    ref_pts[1] = cv::Point2f(73.53179932, 51.50139999);
//    ref_pts[2] = cv::Point2f(56.02519989, 71.73660278);
//    ref_pts[3] = cv::Point2f(41.54930115, 92.3655014);
//    ref_pts[4] = cv::Point2f(70.72990036, 92.20410156);

//    ref_pts[0] = cv::Point2f(30.29459953, 43.69630051);
//    ref_pts[1] = cv::Point2f(65.53179932, 43.50139999);
//    ref_pts[2] = cv::Point2f(48.02519989, 63.73660278);
//    ref_pts[3] = cv::Point2f(33.54930115, 84.3655014);
//    ref_pts[4] = cv::Point2f(62.72990036, 84.20410156);
//    cv::Point2f sum_pt, mean_pt;
//    for (size_t i = 0; i < 5; i++) {
//        sum_pt += ref_pts[i];
//    }
//    mean_pt = sum_pt / 5;
    std::vector<cv::Point2f> norm_pts(5);
    norm_pts[0] = cv::Point2f(-0.18470377, -0.21046333);
    norm_pts[1] = cv::Point2f(0.1823504, -0.21249354);
    norm_pts[2] = cv::Point2f(-1.001358e-05, -0.0017101765);
    norm_pts[3] = cv::Point2f(-0.15080063, 0.21317418);
    norm_pts[4] = cv::Point2f(0.15316395, 0.21149294);

    for (size_t i = 0; i < 5; i++) {
        ref_pts[i] = norm_pts[i] * 96 + cv::Point2f(this->_size/2);
    }


    cv::Mat tfm_my = myGetAffineTransform(this->_pts, ref_pts);
    cv::Mat face_img;
    cv::warpAffine(this->_im, face_img, tfm_my, this->_size);
    return face_img;
}

#ifdef USE_DLIB
/////////////////////////////////////////////////  DlibAlign  /////////////////////////////////////////////////////
DlibAlign::DlibAlign(const cv::Mat &im, const dlib::full_object_detection &dlib_shape, const int size) :
        _im(im),
        _dlib_shape(dlib_shape),
        _size(size) {}

DlibAlign::DlibAlign(const int size) :
        _size(size) {}

DlibAlign::DlibAlign(const int w, const int h) {
    if(w!=h) {
        std::cout <<"wrong size: w, h should be equal" << std::endl;
        exit(0);
    }
    this->_size = w;
}

void DlibAlign::set_im(const cv::Mat &im) {
    this->_im = im;
}

void DlibAlign::set_shape(const dlib::full_object_detection &dlib_shape) {
    this->_dlib_shape = dlib_shape;
}
void DlibAlign::set_pts(const std::vector<cv::Point2f> &pts) {
    std::vector<dlib::point> dlib_pts;
    for(auto &pt : pts) {
        dlib::point dlib_pt(long(pt.x), long(pt.y));
        dlib_pts.emplace_back(dlib_pt);
    }
    cv::Rect2i rect(cv::boundingRect(pts));
    dlib::rectangle dlib_rect(rect.x, rect.y, rect.x+rect.width, rect.y+rect.height);
    this->_dlib_shape = dlib::full_object_detection(dlib_rect, dlib_pts);
}

cv::Mat DlibAlign::Align(){
    dlib::cv_image<dlib::bgr_pixel> dlib_img(this->_im);
    dlib::matrix<dlib::rgb_pixel> face_chip;
    dlib::extract_image_chip(dlib_img, dlib::get_face_chip_details(this->_dlib_shape, this->_size), face_chip);
    assert(face_chip.nr() == this->_size && face_chip.nc());
    cv::Mat rgb_face, bgr_face;
    rgb_face = dlib::toMat(face_chip);
    cv::cvtColor(rgb_face, bgr_face, cv::COLOR_RGB2BGR);
    return bgr_face;
}

#endif
