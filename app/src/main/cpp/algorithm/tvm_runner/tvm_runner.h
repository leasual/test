#pragma once

#include <string>
#include <memory>
#include <vector>
#include <dlpack/dlpack.h>
#include <tvm/runtime/packed_func.h>
#include "tvm_data.h"

class TvmRunner{
public:
    explicit TvmRunner(std::string module_path,std::string json_path,std::string param_path);
    void operator()(std::vector<std::shared_ptr<TvmData>>& input,std::vector<std::shared_ptr<TvmData>>& ouput);
    void operator()(std::shared_ptr<TvmData> &input,std::shared_ptr<TvmData> &ouput);

private:
    TvmRunner(const TvmRunner&) {};
    TvmRunner& operator=(const TvmRunner&){return *this;}

private:
    std::string module_path, json_path, param_path;
    tvm::runtime::PackedFunc set_input;
    tvm::runtime::PackedFunc get_output;
    tvm::runtime::PackedFunc run;
};