#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include "utils/file/file_utils.h"

namespace untouch_utils {
class Config {
public:
	Config();
	Config(const std::string &config_file);
	virtual ~Config();
	bool loadConfig(const std::string &config_file = "");
	virtual void loadData();
	
	template<typename dataType>
	dataType param(const std::string &cpara,const dataType &default_var);

	bool isChange();
	
	void showPara() { is_show_para_ = true; }
	
	virtual void print() {}
	
private:
	bool is_show_para_;
	uint64_t size_;
	uint64_t modify_time_;
	
	cv::FileStorage fSettings;
	std::string config_file_;
};


template<typename dataType>
dataType Config::param(const std::string &cpara, const dataType &default_var){
	dataType var;
	if(fSettings[cpara].empty())
		var = default_var;
	else 
		fSettings[cpara] >> var;
	
	if(is_show_para_)
		std::cout << cpara << ":" << var << std::endl;
	return var;
}
}
