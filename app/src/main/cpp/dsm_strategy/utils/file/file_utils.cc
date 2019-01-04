#include "file_utils.h"
#include "file_read_write_utils.h"
#include "file_system.h"
#include <iomanip>
#include <sstream>

bool sortUpCmp(const std::string &file1, const std::string &file2){
    return file1 < file2;
}

bool sortIntUpCmp(const std::string &file1, const std::string &file2){
	FILEINFO file_info1, file_info2;
	FileUtils::getFileInfo(file1, file_info1);
	FileUtils::getFileInfo(file2, file_info2);
	std::stringstream ss1(file_info1.filename_nosuffix), ss2(file_info2.filename_nosuffix);
	int idx1, idx2;
	
	ss1 >> idx1; ss2 >> idx2;
    return idx1 > idx2;
}

bool sortDownCmp(const std::string &file1, const std::string &file2){
    return file1 > file2;
}

bool FileUtils::getAllFileInPathSort(const std::string &path,
                                 std::vector<std::string> &filename,
                                 const std::string &suffix,
                                 SortByFunc sortByFunc){
//     cv::Directory dir;
//     filename = dir.GetListFiles(path, "."+suffix, false);
	filename = FileSystem::GetListFiles(path, "."+suffix);
    if(filename.size() <= 0){
        std::cerr<< "No *."<<suffix<<" file in the path: "<<path<<std::endl;
        return false;
    }

    std::sort(filename.begin(),filename.end(),sortByFunc);
    return true;
}

bool FileUtils::getAllFileInPath(const std::string &path,
                                 std::vector<std::string> &filename,
                                 const std::string &suffix,
                                 bool up_order){
    if(up_order)
		getAllFileInPathSort(path, filename, suffix, &sortUpCmp);
	else 
		getAllFileInPathSort(path, filename, suffix, sortDownCmp);
    return true;
}



bool FileUtils::loadYaml(const std::string &filename, cv::FileStorage &fs){
    if(!FileSystem::isFileExist(filename))
            return false;
    fs.open(filename, cv::FileStorage::READ);
    return true;
}

bool FileUtils::getFileInfo(const std::string &filename, FILEINFO &file_info) {
	int pos = filename.find_last_of('\\');
	if(pos == -1){
		pos = filename.find_last_of('/');
	}
	
	if(pos == -1){
		std::cerr << "[error]:this filename is not existed" << std::endl;
		return false;
	}
	
	file_info.suffix = "";
	file_info.path = filename.substr(0, pos+1);
	file_info.filename = filename.substr(pos+1, filename.length());
	
	
	int pos1 = filename.find_last_of('.');
	file_info.filename_nosuffix = filename.substr(pos+1, pos1);
	if(pos1 != -1 && pos1 > pos){
		file_info.suffix = filename.substr(pos1+1, filename.length());
		
	}
// 	std::cout << "file_info.path:" << file_info.path << std::endl;
// 	std::cout << "file_info.filename:" << file_info.filename << std::endl;
// 	std::cout << "file_info.suffix:" << file_info.suffix << std::endl;
	
	return true;
}

std::string FileUtils::getParentPath(const std::string &file, int n){
	std::string path = file;
	if(FileSystem::isFileExist(file)){
		FILEINFO file_info;
		getFileInfo(file, file_info);
		path = file_info.path;
	}
	
	int k = n;
	while(k > -1){
		int idx = path.find_last_of('/');
		if(idx == -1){
			std::cout << "the number of path is " << n - k << " n is too large" << std::endl;
			break;
		}
		path = path.substr(0, idx);
		k--;
	}
	
	return path+'/';
}

void FileUtils::getFileState(const std::string &filename, FILESTATE &file_state){
	FileSystem::getFileState(filename, file_state);
}


