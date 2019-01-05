#pragma once
#include <vector>
#include <string>

std::string trim(const std::string &s, char t = 'z');

void split(
	const std::string &line,
	std::vector< std::string > &array,
	char separate = ' ');
bool loadText2String(
	const std::string &filename,
	std::vector< std::string > & lines,
	size_t offset = 0,
	char common = '#');
bool saveString2Text(const std::string &filename,
					 const std::vector< std::string > &lines,
					 bool isappend = false);

bool saveString2Text(
	const std::string &filename,
	const std::string &lines,
	bool isappend = false);

