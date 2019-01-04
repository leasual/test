#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include "file_read_write_utils.h"

namespace untouch_utils {

class SystemConfig{
public:
	SystemConfig(char key_separate = ':', char value_separate = ' ');
	bool loadSystemFile(const std::string &filename);
	
	template<typename dataType>
	bool param(const std::string &key, dataType &value);
	
	void showPara() { is_show_para_ = true; }
	
private:
	size_t getIntValue(const std::string &line);
	double getDoubleValue(const std::string &line);
	std::string getStringValue(const std::string &line);
	
	
	
private:
	bool is_show_para_;
	char key_separate_;
	char value_separate_;
	std::map<std::string, std::string> key_value_;
};

template<typename dataType>
bool SystemConfig::param(const std::string &key, dataType &value){
	if(key_value_.find(key) == key_value_.end())
		return false;
	
	std::vector< std::string > array;
	split(key_value_[key], array, value_separate_);
	std::stringstream ss(array[0]);
	ss >> value;
	if(is_show_para_)
		std::cout << key << ":" << value << std::endl;
}

}