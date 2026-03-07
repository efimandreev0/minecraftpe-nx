#ifndef NET_MINECRAFT_CLIENT__OptionsFile_H__
#define NET_MINECRAFT_CLIENT__OptionsFile_H__


#include <string>
#include <vector>
typedef std::vector<std::string> StringVector;
class OptionsFile
{
public:
	OptionsFile();
    void save(const StringVector& settings);
	StringVector getOptionStrings();
	void setSettingsPath(const std::string& path) { settingsPath = path; }
	
private:
	std::string settingsPath;
};

#endif 
