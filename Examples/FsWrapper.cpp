#include "FsWrapper.h"
#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>

void FsWrapper::SaveToFile(const std::string& data, const std::string& filePath) const
{
	std::ofstream fs(filePath);
	fs.write(data.data(), data.size());
}


TEST(FsWrapper, SaveToFile)
{
	std::string fileName = "C:\\bbb.txt";
	EXPECT_FALSE(std::filesystem::exists(fileName));
	FsWrapper fs;
	fs.SaveToFile("FileContent", fileName);
	ASSERT_TRUE(std::filesystem::exists(fileName));
	std::ifstream file(fileName);
	std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	EXPECT_EQ(str, "FileContent");
}