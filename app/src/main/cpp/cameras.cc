//
// Created by untouch on 18-11-30.
//

#include "cameras.h"

CameraOnLine::CameraOnLine(int index) :
    Camera::Camera(){
    if(cam_.isOpened()) cam_.release();
    if(!cam_.open(index)) {
        std::cout << "Open Cam failed!" << std::endl;
        assert(false);
    }
}

bool CameraOnLine::Read(cv::Mat &frame) {
    if(!cam_.read(frame)){
        return false;
    }
    return true;
}

CameraOffLine::CameraOffLine(const std::string &path) :
    Camera::Camera(),index_(0){
    std::vector<std::string> actions = Listdir(path);
    for(auto& action: actions){
        std::string imgPath = action + "/cam";
        auto imgs = Listfile(imgPath);
        files_.insert(files_.end(), imgs.begin(),imgs.end());
    }
    std::cout << "files size : " << files_.size() << std::endl;
}

bool CameraOffLine::Read(cv::Mat &frame) {
    try {
        frame = cv::imread(files_.at(index_++));
    }
    catch (...){
        if(index_ >= files_.size()){
            std::cout << "No files." <<std::endl;
            std::abort();
        }
        std::cerr << "img read failed!" << std::endl;
        return false;
    }
    return true;
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

    std::sort(filenames.begin(), filenames.end(), [&](std::string& s1, std::string& s2){return s1 < s2;});
    return filenames;
}
