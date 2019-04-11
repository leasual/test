#pragma once

#include <memory>

#include <dlpack.h>
#include <opencv2/opencv.hpp>

class TvmData {
public:

    static size_t CalcTotalByteSize(int64_t* shape,int dim);

    explicit TvmData(std::string layer_name,const int64_t* shape,int dim,
            int dtype_code=kDLFloat,int dtype_bits=32,int dtype_lanes=1,int device_type=kDLCPU,int device_id=0);

    ~TvmData();

    void SetImage(cv::Mat& image);

    void SetData(const float* data,size_t size);

private:
    TvmData(const TvmData&) {}
    TvmData& operator=(const TvmData&) {return *this; }

public:
    std::string layer_name;
    DLTensor* dltensor;
    int64_t* shape;

    int dim,dtype_code,dtype_bits,dtype_lanes,device_type,device_id;
};