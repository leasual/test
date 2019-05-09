#include "tvm_runner.h"

#include <tvm/runtime/module.h>
#include <tvm/runtime/registry.h>

#if defined(ANDROID) || defined(__ANDROID__)
#include <android/log.h>
#define LOG_TAG "FORWARD_NATIVE_LIB"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

TvmRunner::TvmRunner(std::string module_path,std::string json_path,std::string param_path)
        :module_path(module_path),json_path(json_path),param_path(param_path){
    // init values
    tvm::runtime::Module mod_dylib =
            tvm::runtime::Module::LoadFromFile(module_path);
    // json graph
    std::ifstream json_in(json_path, std::ios::in);
    std::string json_data((std::istreambuf_iterator<char>(json_in)), std::istreambuf_iterator<char>());
    json_in.close();

    // parameters in binary
    std::ifstream params_in(param_path, std::ios::binary);
    std::string params_data((std::istreambuf_iterator<char>(params_in)), std::istreambuf_iterator<char>());

    TVMByteArray params_arr;
    params_in.close();
    params_arr.data = params_data.c_str();
    params_arr.size = params_data.length();

    int cpu_type = kDLCPU,device_id = 0;
    tvm::runtime::Module mod = (*tvm::runtime::Registry::Get("tvm.graph_runtime.create"))(json_data, mod_dylib, cpu_type,
                                                                                          device_id);
    tvm::runtime::PackedFunc load_params = mod.GetFunction("load_params");
    set_input = mod.GetFunction("set_input");
    get_output = mod.GetFunction("get_output");
    run = mod.GetFunction("run");

    load_params(params_arr);
}

void TvmRunner::operator()(std::vector<std::shared_ptr<TvmData>> &input,
                                std::vector<std::shared_ptr<TvmData>> &output) {
    for(auto& tvmdata_p:input) {
        set_input(tvmdata_p->layer_name,tvmdata_p->dltensor);
    }

#if defined(COUNT_TIME)
    clock_t start,finish;
    double totaltime;
    start = clock();
#endif

    run();

#if defined(COUNT_TIME)
    finish = clock();
    totaltime = (double)(finish-start)/CLOCKS_PER_SEC;;
    std::cout << "total time : " << totaltime << std::endl;
#if defined(ANDROID) || defined(__ANDROID__)
    LOGI("total time %d",totaltime);
#endif
#endif

    for(int i=0;i<output.size();i++) {
        get_output(i,output.at(i)->dltensor);
    }
}

void TvmRunner::operator()(std::shared_ptr<TvmData> &input,
                                std::shared_ptr<TvmData> &output) {
    set_input(input->layer_name,input->dltensor);

#if defined(COUNT_TIME)
    clock_t start,finish;
    double totaltime;
    start = clock();
#endif

    run();

#if defined(COUNT_TIME)
    finish = clock();
    totaltime = (double)(finish-start)/CLOCKS_PER_SEC;;
    std::cout << "total time : " << totaltime << std::endl;
#if defined(ANDROID) || defined(__ANDROID__)
    LOGI("total time %d",totaltime);
#endif
#endif

    get_output(0,output->dltensor);
}