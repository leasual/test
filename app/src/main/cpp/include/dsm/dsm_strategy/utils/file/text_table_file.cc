#include "text_table_file.h"
#include "file_utils.h"
#include "file_read_write_utils.h"

void TextData::getIdxClear(){
	for(size_t i = 0; i < get_idx.size(); i++){
		get_idx[i] = -1;
	}
}

void TextData::setIdxClear(){
	for(size_t i = 0; i < set_idx.size(); i++){
		set_idx[i] = -1;
	}
}

int64_t TextData::getInt64Value(int idx){
	if(-1 == idx)
		return int64_data[get_idx[0]];
	else if( -2 == idx)
		return int64_data[++get_idx[0]];
	else if( idx >= max_sizes[0]){
		std::cout << "[warning]: idx over!" << std::endl;
		return -999999;
	}
	else
		return int64_data[idx];
}

std::string TextData::getStringValue(int idx){
	if(-1 == idx)
		return string_data[get_idx[1]];
	else if( -2 == idx)
		return string_data[++get_idx[1]];
	else if( idx >= max_sizes[1]){
		std::cout << "[warning]: idx over!" << std::endl;
		return "";
	}
	else
		return string_data[idx];
}

double TextData::getDoubleValue(int idx){
	if(-1 == idx)
		return double_data[get_idx[2]];
	else if( -2 == idx)
		return double_data[++get_idx[2]];
	else if( idx >= max_sizes[2]){
		std::cout << "[warning]: idx over!" << std::endl;
		return -999999.99;
	}
	else
		return double_data[idx];
}

double TextData::getIntValue(int idx){
	if(-1 == idx)
		return int_data[get_idx[3]];
	else if( -2 == idx)
		return int_data[++get_idx[3]];
	else if( idx >= max_sizes[3]){
		std::cout << "[warning]: idx over!" << std::endl;
		return -999999;
	}
	else
		return int_data[idx];
}

void TextData::setInt64Value(int64_t value){
	int64_data[++set_idx[0]] = value;
}

void TextData::setStringValue(const std::string &value){
	string_data[++set_idx[1]] = value;
}

void TextData::setDoubleValue(double value){
	double_data[++set_idx[2]] = value;
}

void TextData::setIntValue(int value){
	int_data[++set_idx[3]] = value;
}
	
	
	
//////////////////////////////////////////////////

TextTableFile::TextTableFile(const std::string format, const char separate, size_t offset_line):
format_(format),
separate_(separate),
offset_line_(offset_line){
	format_count_ = formatAnalyse(format_, format_array_);
	cols_ = format_array_.size();
}

bool TextTableFile::loadText(const std::string &filename) {
	if(!::loadText2String(filename, lines_, offset_line_))
		return false;

	
	std::vector< std::string > array;
	text_data_array_.clear();
	
	int64_t int64_data;
	std::string string_data;
	double double_data;
	int int_data;
	for(int i = 0; i < lines_.size(); i++){
		if(lines_[i][0] == '#')
			continue;

		std::shared_ptr<TextData> text_data = std::make_shared<TextData>(
			format_count_[0], format_count_[1],
			format_count_[2], format_count_[3]);

		split(lines_[i], array, separate_);
		
		std::stringstream ss;
		
		int count[10] ={0};
		for(int j = 0; j < array.size(); ++j){
			std::stringstream ss(array[j]);
			if(format_array_[j] == "d64"){
				ss >> int64_data;
				text_data->setInt64Value(int64_data);
			}
			else if (format_array_[j] == "s"){
				string_data = ss.str();
				text_data->setStringValue(string_data);
			}
			else if (format_array_[j] == "f"){
				ss >> double_data;
				text_data->setDoubleValue(double_data);
			}
			else if(format_array_[j] == "i"){
				ss >> int_data;
				text_data->setIntValue(int_data);
			}
			
// 			std::cout << separate_;
		}
        text_data_array_.push_back(text_data);
		
// 		std::cout << std::endl;
	}
	
	
// 	std::cout << "text_data_array_.size:" << text_data_array_.size() << std::endl;

	return true;
}

bool TextTableFile::loadNumericText(const std::string &filename){
	if(!::loadText2String(filename, lines_, offset_line_))
		return false;
	
	for(size_t i = 0; i < format_array_.size(); i++){
		if(format_array_[i] == "s"){
			std::cout << "[error]: must input numeric text" << std::endl;
			return false;
		}
	}

	text_data_array_.clear();
	int64_t int64_data;
	std::string string_data;
	double double_data;
	int int_data;
	for(size_t i = 0; i < lines_.size(); i++){
		if(lines_[i][0] == '#')
			continue;
		std::shared_ptr<TextData> text_data = std::make_shared<TextData>(
			format_count_[0], format_count_[1],
			format_count_[2], format_count_[3]);
		std::stringstream ss(lines_[i]);
		std::cout << lines_[i] << std::endl;
		
		char delimeter;
		

		for(int j = 0; j < format_array_.size(); ++j){
// 			std::cout << array[j] << ",";
			if(format_array_[j] == "d64"){
				ss >> int64_data >> delimeter;
				text_data->setInt64Value(int64_data);
			}
			else if (format_array_[j] == "s"){
				ss >> string_data >> delimeter;
				text_data->setStringValue(string_data);
			}
			else if (format_array_[j] == "f"){
				ss >> double_data >> delimeter;
				text_data->setDoubleValue(double_data);
			}
			else if(format_array_[j] == "i"){
				ss >> int_data >> delimeter;
				text_data->setIntValue(int_data);
			}
			
// 			std::cout << separate_;
		}
        text_data_array_.push_back(text_data);
// 		std::cout << std::endl;
	}
	
	return true;
}

bool TextTableFile::saveText(const std::string &filename, int precisions){
	std::fstream fs(filename, std::ios::out);
	
	if(!title_.empty())
		fs << title_ << std::endl;
	for(size_t i = 0; i < text_data_array_.size(); i++){
		std::shared_ptr<TextData> &text_data = text_data_array_[i];
		text_data->getIdxClear();
		for(size_t j = 0; j < format_array_.size(); j++){
			if(format_array_[j] == "d64")
				fs << text_data->getInt64Value(-2);
			if(format_array_[j] == "s")
				fs << text_data->getStringValue(-2);
			if(format_array_[j] == "f") 
				fs << text_data->getDoubleValue(-2);
			if(format_array_[j] == "i"){
				fs.precision(precisions);
				fs << text_data->getIntValue(-2);
				fs.unsetf( std::ios::fixed );
			}
			if(j < format_array_.size()-1)
				fs << separate_;
		}
		fs << std::endl;
	}
	
	fs.close();
	return true;
}

std::shared_ptr<TextData> TextTableFile::getTextData(size_t idx) const{
	assert(idx >= 0 && idx < text_data_array_.size());
	return text_data_array_[idx];
}

std::string TextTableFile::getLine(size_t idx){
	assert(idx >= 0 && idx < lines_.size());
	return lines_[idx];
}

bool TextTableFile::getLines(
	size_t begin_idx,
	size_t end_idx,
	std::vector< std::string > &lines){
	assert(begin_idx >= 0 && begin_idx < lines_.size());
	assert(end_idx >= 1 && end_idx <= lines_.size());
	
	lines.clear();
	lines.resize(end_idx - begin_idx);
	for(size_t i = 0; i < end_idx; i++)
		lines[i] = lines_[i];
	
	return true;
}

int TextTableFile::getIntValue(size_t nrow, int ncols) const{
	return text_data_array_[nrow]->getIntValue(ncols);
}

std::string TextTableFile::getStringValue(size_t nrow, int ncols) const{
	return text_data_array_[nrow]->getStringValue(ncols);
}

double TextTableFile::getDoubleValue(size_t nrow, int ncols) const{
	return text_data_array_[nrow]->getDoubleValue(ncols);
}

int64_t TextTableFile::getInt64Value(size_t nrow, int ncols) const{
	return text_data_array_[nrow]->getInt64Value(ncols);
}

void TextTableFile::setRows(size_t rows){
	rows_ = rows;
	text_data_array_.resize(rows);
	for(size_t i = 0; i < rows_; i++){
		text_data_array_[i] = std::make_shared<TextData>(format_count_[0],format_count_[1], format_count_[2], format_count_[3]);
	}
}

void TextTableFile::setTitle(const std::string &title){
	title_ = title;
}

void TextTableFile::setIntValue(size_t nrow, int value){
	text_data_array_[nrow]->setIntValue(value);
}

void TextTableFile::setStringValue(size_t nrow, std::string &value){
	text_data_array_[nrow]->setStringValue(value);
}

void TextTableFile::setDoubleValue(size_t nrow, double value){
	text_data_array_[nrow]->setDoubleValue(value);
}

void TextTableFile::setInt64Value(size_t nrow, int64_t value){
	text_data_array_[nrow]->setInt64Value(value);
}


//format_count：int64_t string double int的顺序输出,分别对应%d64, %s, %f, %i
std::vector< int > TextTableFile::formatAnalyse(
		const std::string &format,
		std::vector< std::string > &format_array) {

	split(format, format_array, '%');

	std::vector<int> format_count(10, 0);
	for(int i = 0; i < format_array.size(); ++i){
		if(format_array[i] == "d64")
			format_count[0]++;
		else if(format_array[i] == "s")
			format_count[1]++;
		else if(format_array[i] == "f")
			format_count[2]++;
		else if(format_array[i] == "i")
			format_count[3]++;
        else
            std::cout << "invaid format char[%" << format_array[i] << "], please input again!" << std::endl;
	}
	
	return format_count;
}

void TextTableFile::print(const std::string &addition){
	std::cout << "--------" << addition << "---------" << std::endl;
	
	for(size_t i = 0; i < text_data_array_.size(); i++){
		int format_count[10] = {0};
		for(size_t j = 0; j < format_array_.size(); j++){
// 			std::cout << "format_array_[j]:" << format_array_[j] << std::endl;
			if(format_array_[j] == "d64")
				std::cout << text_data_array_[i]->getInt64Value(format_count[0]++);
			if(format_array_[j] == "s")
				std::cout << text_data_array_[i]->getStringValue(format_count[1]++);
			if(format_array_[j] == "f") 
				std::cout << text_data_array_[i]->getDoubleValue(format_count[2]++);
			if(format_array_[j] == "i")
				std::cout << text_data_array_[i]->getIntValue(format_count[3]++);
			if(j < format_array_.size()-1)
				std::cout << format_array_[j] << separate_;
		}
		std::cout << format_array_[format_array_.size()-1] << std::endl;
	}
	std::cout << "++++++++" << addition << "++++++++" << std::endl;
}
