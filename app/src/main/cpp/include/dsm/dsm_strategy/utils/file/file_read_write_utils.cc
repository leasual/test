#include "file_read_write_utils.h"
#include "file_system.h"
#include <fstream>

std::string trim(const std::string &s, char t) {
    if(s.empty())
        return s;
    
    int begin_idx = 0, end_idx = s.length() - 1;
    if(t != 'z'){
        while(begin_idx <= end_idx && s[begin_idx++] == t);
        while(end_idx >= begin_idx && s[end_idx--] == t);
    }
    else {
        while(begin_idx <= end_idx
            && (s[begin_idx] == '\r' 
            || s[begin_idx] == '\n' 
            || s[begin_idx] == '\t'
            || s[begin_idx] == ' ')){
			begin_idx++;
            }
        while(end_idx >= begin_idx
            && (s[end_idx] == '\r' 
            || s[end_idx] == '\n' 
            || s[end_idx] == '\t'
            || s[end_idx] == ' ')){
			end_idx--;
            }
    }
    
    return s.substr(begin_idx, end_idx + 1);
    
}

void split(const std::string &line, std::vector< std::string > &array, char separate){
    if(line.empty()){
        std::cerr << "the line is empty, please check it!" << std::endl;
        return;
    }

    int start = 0;
    int index;
    std::string subString;

    array.clear();
    index = line.find_first_of(separate, start);

    while(index != std::string::npos){
        subString = line.substr(start, index - start);
        if(subString.length())
            array.push_back(subString);
        start = index+1;
        index = line.find_first_of(separate,start);
    }

    if(start != line.length()){
        subString = line.substr(start);
        if(subString.length())
            array.push_back(subString);
    }
}

bool loadText2String(
	const std::string &filename,
	std::vector< std::string > & lines,
	size_t offset,
	char common){

	if(!FileSystem::isFileExist(filename))
		return false;

    std::fstream file_reader(filename, std::ios::in);

    lines.clear();
    std::string data_line;
    size_t i_counter = 0;
    while (getline(file_reader, data_line)) {
        if (data_line[0] == common)
            continue;
        if (++i_counter < offset)
            continue;
        lines.push_back(data_line);
    }

    return true;
}

bool saveString2Text(const std::string &filename, const std::vector< std::string > &lines, bool isappend){
	std::fstream file_writer;

	if(isappend)
		file_writer.open(filename, std::ios::out | std::ios::app);
	else
		file_writer.open(filename, std::ios::out);

	for(int i = 0; i < lines.size(); ++i)
		file_writer << lines[i] << std::endl;

	file_writer.close();

}

bool saveString2Text(const std::string &filename, const std::string &lines, bool isappend){
	std::fstream file_writer;

	if(isappend)
		file_writer.open(filename, std::ios::out | std::ios::app);
	else
		file_writer.open(filename, std::ios::out);

	file_writer << lines;

	file_writer.close();

}