#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>
#include <memory>
#include "file_system.h"
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "file_define.h"

bool sortUpCmp(const std::string &file1, const std::string &file2);
bool sortIntUpCmp(const std::string &file1, const std::string &file2);
bool sortDownCmp(const std::string &file1, const std::string &file2);
	
class FileUtils
{
public:
	FileUtils(){};
	~FileUtils(){};

	static bool getAllFileInPath(
			const std::string &path,
			std::vector<std::string> &filename,
			const std::string &suffix,
			bool up_order = true);

	template<typename T>
    static std::string number2string(T num, int len = -1, char full = '0');

    static bool loadYaml(const std::string &filename, cv::FileStorage &fs);
    
	static bool getFileInfo(const std::string &filename, FILEINFO &file_info);
	static void getFileState(const std::string &filename, FILESTATE &file_state);
	static std::string getParentPath(const std::string &file, int n);
	
// private:
	
	static bool getAllFileInPathSort(
            const std::string &path,
            std::vector<std::string> &filename,
            const std::string &suffix,
            SortByFunc sortByFunc = sortUpCmp);
private:
	
};

template<typename T>
std::string FileUtils::number2string(T num, int len, char full){
	std::stringstream ss;
	if(len > 0)
		ss << std::setw(len) << std::setfill(full);

	ss << num;
	return ss.str();
}

#endif // FILEUTILS_H
