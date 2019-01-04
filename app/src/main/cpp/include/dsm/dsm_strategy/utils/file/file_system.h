#pragma once
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

struct FILESTATE {
	uint64_t size;
	uint64_t modify_time;
};

class FileSystem {
public:
	static bool isFileExist(const std::string &filename);
	static bool isFolderExist(const std::string &path);
	static void getFileState(const std::string &filename, FILESTATE &file_state);
	static std::vector<std::string> GetListFiles(
		const std::string &folder,
		const std::string &suffix);
	static double memoryFreeRate();
	static size_t memoryTotalSize();
	static size_t memoryAvailableSize();
};