#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <gmock/gmock.h>
#include <filesystem>
#include <sstream>
#include <fstream>

using namespace testing;

namespace
{
	size_t WriteData(void* contents, size_t size, size_t nmemb, void* userp) {

		const size_t realsize = size * nmemb;
		std::ostream* const stream = static_cast<std::ostream*>(userp);
		if (stream != nullptr)
		{
			stream->write(static_cast<const char*>(contents), realsize);
		}
		return realsize;
	}

	class IDownloader
	{
	public:
		virtual std::string DownloadData(const std::string& url) const = 0;
	};

	class IFsSaver
	{
	public:
		virtual void SaveToFile(const std::string& data, const std::string& filePath) const = 0;
	};

	int DownloadFile(const IDownloader& downloader, const IFsSaver& fsSaver) {
		const auto& downloadedData = downloader.DownloadData("http://localhost/aaa.txt");
		fsSaver.SaveToFile(downloadedData, "C:\\bbb.txt");
		return 0;
	}

	class Downloader : public IDownloader
	{
	public:
		std::string DownloadData(const std::string& url) const override
		{
			std::stringstream ss;
			CURL* curl;
			CURLcode res;

			curl = curl_easy_init();
			if (curl) {
				curl_easy_setopt(curl, CURLOPT_URL, url);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);
				res = curl_easy_perform(curl);
				curl_easy_cleanup(curl);
			}
			return ss.str();
		}
	};

	class FsSaver : public IFsSaver
	{
	public:
		void SaveToFile(const std::string& data, const std::string& filePath) const override
		{
			std::ofstream fs(filePath);
			fs.write(data.data(), data.size());
		}
	};

	class MockDownloader : public IDownloader
	{
	public:
		MOCK_CONST_METHOD1(DownloadData, std::string(const std::string&));
	};

	class MockFsSaver : public IFsSaver
	{
	public:
		MOCK_CONST_METHOD2(SaveToFile, void(const std::string&, const std::string&));
	};
}

TEST(InheritFromInterface, Downloader_DownloadData)
{
	Downloader downloader;
	EXPECT_EQ(downloader.DownloadData("http://localhost/aaa.txt"), "FileContent");
}

TEST(InheritFromInterface, FsSaver_SaveToFile)
{
	std::string fileName = "C:\\bbb.txt";
	EXPECT_FALSE(std::filesystem::exists(fileName));
	FsSaver fs;
	fs.SaveToFile("FileContent", fileName);
	ASSERT_TRUE(std::filesystem::exists(fileName));
	std::ifstream file(fileName);
	std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	EXPECT_EQ(str, "FileContent");
}

TEST(InheritFromInterface, DownloadFileProduceExpectedCalls)
{
	MockDownloader downloader;
	MockFsSaver fs;
	EXPECT_CALL(downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
	EXPECT_CALL(fs, SaveToFile("FileContent", "C:\\bbb.txt"));
	DownloadFile(downloader, fs);
}