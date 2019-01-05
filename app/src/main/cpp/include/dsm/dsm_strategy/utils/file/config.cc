#include "config.h"
#include "file_utils.h"

namespace untouch_utils {
	
Config::Config():
config_file_(""),
is_show_para_(false),
size_(0),
modify_time_(0){
}

Config::Config(const std::string& config_file):
config_file_(config_file),
size_(0),
modify_time_(0),
is_show_para_(false)
{
	loadConfig(config_file);
}

Config::~Config(){
	fSettings.release();
}

bool Config::loadConfig(const std::string &config_file){
	if(!config_file.empty())
		config_file_ = config_file;
	return FileUtils::loadYaml(config_file_, fSettings);
}

void Config::loadData(){
	std::cout << "[warning][Config::loadData]:no config file load" << std::endl;
}

bool Config::isChange(){
	FILESTATE file_state;
	FileUtils::getFileState(config_file_, file_state);
	if(size_ != file_state.size || modify_time_ !=  file_state.modify_time){
		size_ = file_state.size;
		modify_time_ = file_state.modify_time;
		
		std::cout << "size:" << size_ << "," << file_state.size << std::endl;
		std::cout << "time:" << modify_time_ << "," << file_state.modify_time << std::endl;
		return true;
	}
	return false;
	
}

    template<>
    bool Config::param<bool>(const std::string &cpara, const bool &default_var){
        bool var;
        if(fSettings[cpara].empty())
            var = default_var;
        else {
            std::string tmp;
            fSettings[cpara] >> tmp;
            if(tmp == "true")
                var = true;
            else if(tmp == "false")
                var = false;
            else
                std::cout << "format in config file is wrong" << std::endl;

            if(is_show_para_)
                std::cout << cpara << ":" << tmp << std::endl;
        }

        return var;
    }

}