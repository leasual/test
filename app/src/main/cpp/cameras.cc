//
// Created by untouch on 18-11-30.
//
#include <exception>
#include "cameras.h"

CameraOnLine::CameraOnLine(int index) :
    Camera::Camera(){
    if(cam_.isOpened()) cam_.release();
    if(!cam_.open(index)) {
        std::cout << "Open Cam failed!" << std::endl;
        assert(false);
    }
}

std::string CameraOnLine::Read(cv::Mat &frame) {
    if(!cam_.read(frame)){
        return "";
    }
    return "";
}

CameraOffLine::CameraOffLine(const std::string &path) :
    Camera::Camera(),index_(0){
#if 0
    std::vector<std::string> actions = Listdir(path);
    for(auto& action: actions){
        std::string imgPath = action + "/cam";
        auto imgs = Listfile(imgPath);
        files_.insert(files_.end(), imgs.begin(),imgs.end());
    }
#endif
    std::vector<std::string> actions = Listdir(path);
    for(size_t i = 1; i != 3; ++i){
//        std::string imgPath = path + std::to_string(i) + "/data/cam0/data";
//        auto imgs = Listfile(imgPath);
//        files_.insert(files_.end(), imgs.begin(),imgs.end());
    }
    std::cout << "files size : " << files_.size() << std::endl;
}

std::string CameraOffLine::Read(cv::Mat &frame) {
    std::string path;
    try {
        path = files_.at(index_++);
        frame = cv::imread(path);
    }
    catch (...){
        if(index_ >= files_.size()){
            throw std::out_of_range("No files");
        }
        std::cerr << "img read failed!" << std::endl;
        return path;
    }
    return path;
}

std::vector<std::string> CameraOffLine::Listdir(const std::string &folder) {
    std::vector<std::string> filenames;
    DIR *dir;
    struct dirent *ptr;
    if((dir = opendir(folder.c_str())) == nullptr){
        std::cout << "Open dir(" << folder << ") error..." << std::endl;
        return filenames;
    }

    while ((ptr = readdir(dir)) != nullptr)
    {
        if(strcmp(ptr->d_name, ".") == 0 |
           strcmp(ptr->d_name, "..") == 0)
            continue;
        if(4 == ptr->d_type) {
            std::string dir_name = ptr->d_name;
            filenames.push_back(folder + "/" + ptr->d_name);
        }
    }
    closedir(dir);

    std::sort(filenames.begin(), filenames.end(), [&](std::string& s1, std::string& s2){return s1 < s2;});
    return filenames;
}

std::vector<std::string> CameraOffLine::Listfile(const std::string &folder) {
    std::vector<std::string> filenames;
    DIR *dir;
    struct dirent *ptr;
    if((dir = opendir(folder.c_str())) == nullptr){
        std::cout << "Open dir(" << folder << ") error..." << std::endl;
        return filenames;
    }

    while ((ptr = readdir(dir)) != nullptr)
    {
        if(strcmp(ptr->d_name, ".") == 0 |
           strcmp(ptr->d_name, "..") == 0)
            continue;
        if(8 == ptr->d_type) {
            std::string dir_name = ptr->d_name;
            filenames.push_back(folder + "/" +ptr->d_name);
        }
    }
    closedir(dir);

    std::sort(filenames.begin(), filenames.end(), [&](std::string& s1, std::string& s2){
        auto a = s1.substr(s1.find_last_of("_")+1,19);
        auto b = s2.substr(s2.find_last_of("_")+1,19);
        return a < b;});
    return filenames;
}
