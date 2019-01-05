#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define TYPE_NUM 4

class TextData {
public:
    TextData(){
		get_idx = std::vector<char>(TYPE_NUM, -1);
		set_idx = std::vector<char>(TYPE_NUM, -1);
	}

    TextData(int t_count, int s_count, int d_count, int i_count){
		get_idx = std::vector<char>(TYPE_NUM, -1);
		set_idx = std::vector<char>(TYPE_NUM, -1);
		max_sizes.resize(TYPE_NUM);
		max_sizes[0] = t_count; max_sizes[1] = s_count;
		max_sizes[2] = d_count; max_sizes[3] = i_count; 
        int64_data.resize(t_count);
        string_data.resize(s_count);
        double_data.resize(d_count);
		int_data.resize(i_count);
    }
    
    TextData(const std::vector<int> &max_sizev){
		get_idx = std::vector<char>(TYPE_NUM, -1);
		set_idx = std::vector<char>(TYPE_NUM, -1);
		max_sizes.resize(TYPE_NUM);
		max_sizes[0] = max_sizev[0]; max_sizes[1] = max_sizev[1];
		max_sizes[2] = max_sizev[2]; max_sizes[3] = max_sizev[3]; 
        int64_data.resize(max_sizev[0]);
        string_data.resize(max_sizev[1]);
        double_data.resize(max_sizev[2]);
		int_data.resize(max_sizev[3]);
    }
    ~TextData() {
    }
    
    void getIdxClear();
	void setIdxClear();
	int64_t getInt64Value(int idx = -1);
	std::string getStringValue(int idx = -1);
	double getDoubleValue(int idx = -1);
	double getIntValue(int idx = -1);
	void setInt64Value(int64_t value);
	void setStringValue(const std::string &value);
	void setDoubleValue(double value);
	void setIntValue(int value);

private:
    std::vector<int64_t> int64_data;
    std::vector<std::string> string_data;
    std::vector<double> double_data;
	std::vector<int> int_data;
	
	std::vector<char> get_idx, set_idx;
	std::vector<char> max_sizes;
};

class TextTableFile {
public:
	TextTableFile(const std::string format, const char separate = ',', size_t offset_line = 0);
	
	bool loadText(const std::string &filename);
	
	// the seperate must be ','
	bool loadNumericText(const std::string &filename);
	bool saveText(const std::string &filename, int precisions = 2);
	
	std::shared_ptr<TextData> getTextData(size_t idx) const;
	std::string getLine(size_t idx);
	bool getLines(
		size_t begin_idx,
		size_t end_idx,
		std::vector< std::string > &lines);
	size_t getRows() { return text_data_array_.size();}
	size_t getCols() { return cols_;}
	size_t size(){ return text_data_array_.size();}
	
	int getIntValue(size_t nrow, int ncols = -2) const;
	std::string getStringValue(size_t nrow, int ncols = -2) const;
	double getDoubleValue(size_t nrow, int ncols = -2) const;
	int64_t getInt64Value(size_t nrow, int ncols = -2) const;
	
	void setRows(size_t rows);
	void setTitle(const std::string &title);
	void setIntValue(size_t nrow, int value);
	void setStringValue(size_t nrow, std::string &value);
	void setDoubleValue(size_t nrow, double value);
	void setInt64Value(size_t nrow, int64_t value);
	
	void print(const std::string &addition = "");
	
private:
	std::vector< int > formatAnalyse(
		const std::string &format,
		std::vector< std::string > &format_array);
private:
	char separate_;
	std::string format_;
	std::vector< std::string > format_array_;
	std::vector< int > format_count_;
	std::vector< std::shared_ptr<TextData> > text_data_array_;
	
	std::vector< std::string > lines_;
	
	
	size_t offset_line_;
	std::string title_;
	int rows_, cols_;
};