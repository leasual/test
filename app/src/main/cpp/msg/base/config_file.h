#ifndef CONFIGFILEREADER_H_
#define CONFIGFILEREADER_H_

#include "public_def.h"
#include "singleton.h"
#include <string>
#include <map>

class CConfigFileReader:public Singleton<CConfigFileReader>
{
public:
	//CConfigFileReader(const char* filename);
	CConfigFileReader();
	~CConfigFileReader();
	void LoadFromFile(const char* filename);

    char* GetConfigName(const char* name);
    int SetConfigValue(const char* name, const char*  value);
private:
    void _LoadFile(const char* filename);
    int _WriteFIle(const char*filename = NULL);
    void _ParseLine(char* line);
    char* _TrimSpace(char* name);

    bool m_load_ok;
    std::map<std::string, std::string> m_config_map;
    std::string m_config_file;
};



#endif /* CONFIGFILEREADER_H_ */
