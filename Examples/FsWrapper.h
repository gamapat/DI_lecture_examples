#pragma once
#include <string>

class IFsWrapper
{
public:
	virtual void SaveToFile(const std::string& data, const std::string& filePath) const = 0;
};

class FsWrapper
{
public:
	virtual void SaveToFile(const std::string& data, const std::string& filePath) const;
};

