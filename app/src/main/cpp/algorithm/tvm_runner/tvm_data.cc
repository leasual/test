#include "tvm_data.h"
#include <cassert>

#include <tvm/runtime/module.h>
#include <tvm/runtime/registry.h>
#include <tvm/runtime/packed_func.h>

TvmData::TvmData(std::string layer_name,const int64_t* shape,int dim,
                 int dtype_code,int dtype_bits,int dtype_lanes,int device_type,int device_id)
                 :layer_name(layer_name),dim(dim),dtype_code(dtype_code),dtype_bits(dtype_bits),dtype_lanes
                 (dtype_lanes),device_id(device_id),device_type(device_type),shape(nullptr),dltensor(nullptr) {
    assert(dim != 0 && "dim can't be assigned to zero!");
    this->shape = new int64_t[dim];
    memcpy(this->shape,shape,sizeof(int64_t)*dim);

    TVMArrayAlloc(shape,dim,dtype_code,dtype_bits,dtype_lanes,device_type,device_id,&dltensor);
}

TvmData::~TvmData() {
    delete [] shape;
    shape = nullptr;
    if(dltensor != nullptr) {
        TVMArrayFree(dltensor);
        dltensor = nullptr;
    }
}

size_t TvmData::CalcTotalByteSize(int64_t* shape,int dim){
    size_t result = 1;
    for(int i=0;i<dim;i++) {
        result*=shape[i];
    }
    return result;
}

void TvmData::SetImage(cv::Mat& image) {
    cv::Mat image32nchw = cv::dnn::blobFromImage(image);
    TVMArrayCopyFromBytes(dltensor,image32nchw.data,CalcTotalByteSize(shape,dim)*4);
}

void TvmData::SetData(const float* data,size_t size) {
    float* pointer = static_cast<float*>(dltensor->data);
    for(size_t i=0;i<size;i++) {
        pointer[i] = data[i];
    }
}