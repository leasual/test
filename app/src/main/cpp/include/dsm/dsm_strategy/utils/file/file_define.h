#pragma once
#include <string>

typedef bool (*SortByFunc)(const std::string &file1, const std::string &file2);

struct FILEINFO {
	std::string path;//末尾带/
	std::string filename;
	std::string filename_nosuffix;
	std::string suffix;
};