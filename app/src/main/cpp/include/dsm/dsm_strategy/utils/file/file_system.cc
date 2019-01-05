
#include "file_system.h"
#include <string.h>
#include "system_config.h"

bool FileSystem::isFileExist(const std::string &filename){
	if(access(filename.c_str(), F_OK)){
		std::cout << filename << " file is not exist" << std::endl;
		return false;
	}
	
	return true;
}

bool FileSystem::isFolderExist(const std::string &path){
	if(opendir(path.c_str()) == NULL){
		std::cout << "the folder no exist" << std::endl;
		return false;
	}
	return true;
}

void FileSystem::getFileState(const std::string &filename, FILESTATE &file_state){
	if(!isFileExist(filename))
		return;

	struct stat buf;
	stat(filename.c_str(), &buf);
	
	file_state.size = buf.st_size;
	file_state.modify_time = buf.st_mtime;
	
}

std::vector<std::string> FileSystem::GetListFiles(const std::string &folder, const std::string &suffix){
	
	std::vector<std::string> filenames;
	DIR *dir;
	struct dirent *ptr;
	if((dir = opendir(folder.c_str())) == NULL){
		std::cout << "Open dir(" << folder << ") error..." << std::endl;
		return filenames;
	}
	
	while((ptr = readdir(dir)) != NULL){
		if(strcmp(ptr->d_name, ".") == 0 |
			strcmp(ptr->d_name, "..") == 0)
			continue;
		else if(ptr->d_type == 8){ //file
			std::string file = ptr->d_name;
// 			std::cout << file.substr(file.length()-4, file.length()-1) << std::endl;
			if((file.substr(file.length()-4, file.length()-1)) != suffix){
				continue;
			}
			filenames.push_back(file);
// 			std::cout << filenames.back() << std::endl;
		}
		else if(ptr->d_type == 10) //link file
			continue;
		else if(ptr->d_type == 4){ //dir
			/*
			std::string base = folder + ptr->d_name + "/";
			std::cout << base << std::endl;
			GetListFiles(base);
			*/
		}
// 		std::cout << filenames.back() << std::endl;
	}
	closedir(dir);
	
	std::cout << "GetListFiles over" << std::endl;
	return filenames;
}

double FileSystem::memoryFreeRate(){
	untouch_utils::SystemConfig sc;
	sc.loadSystemFile("/proc/meminfo");
	
	size_t MemTotal, MemAvailable;
	sc.param<size_t>("MemTotal", MemTotal);
	sc.param<size_t>("MemAvailable", MemAvailable);
	return 1.0*MemAvailable/MemTotal;
}

size_t FileSystem::memoryTotalSize(){
	untouch_utils::SystemConfig sc;
	sc.loadSystemFile("/proc/meminfo");
	
	size_t MemTotal;
	sc.param<size_t>("MemTotal", MemTotal);
	return MemTotal;
}

size_t FileSystem::memoryAvailableSize(){
	untouch_utils::SystemConfig sc;
	sc.loadSystemFile("/proc/meminfo");
	
	size_t MemAvailable;
	sc.param<size_t>("MemAvailable", MemAvailable);
	return MemAvailable;
}