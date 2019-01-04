#include "system_config.h"
#include "text_table_file.h"


namespace untouch_utils {
	
SystemConfig::SystemConfig(char key_separate, char value_separate):
is_show_para_(false),
key_separate_(key_separate),
value_separate_(value_separate){
}

bool SystemConfig::loadSystemFile(const std::string &filename){
	std::vector< std::string > lines;
	if(!loadText2String(filename, lines))
		std::cout << "fail" << std::endl;
	
	for(size_t i = 0; i < lines.size(); i++){
		std::vector< std::string > array;
		split(lines[i], array, key_separate_);
		key_value_[trim(array[0])] = trim(array[1]);
		if(is_show_para_)
 			std::cout << trim(array[0]) << key_separate_ << array[1] << std::endl;
	}
}
}