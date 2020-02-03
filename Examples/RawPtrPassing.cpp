#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <gmock/gmock.h>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <memory>

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

	class Downloader
	{
	public:
		virtual std::string DownloadData(const std::string& url) const
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

	class FsSaver
	{
	public:
		virtual void SaveToFile(const std::string& data, const std::string& filePath) const
		{
			std::ofstream fs(filePath);
			fs.write(data.data(), data.size());
		}
	};

	int DownloadFile(Downloader* downloader, FsSaver* fsSaver) {
		const auto& downloadedData = downloader->DownloadData("http://localhost/aaa.txt");
		fsSaver->SaveToFile(downloadedData, "C:\\bbb.txt");
		return 0;
	}

	class ClassToDownloadFile
	{
	public:
		ClassToDownloadFile(Downloader* downloader, FsSaver* fsSaver)
			: m_downloader(downloader)
			, m_fsSaver(fsSaver)
		{}

		int DownloadFile()
		{
			const auto& downloadedData = m_downloader->DownloadData("http://localhost/aaa.txt");
			m_fsSaver->SaveToFile(downloadedData, "C:\\bbb.txt");
			return 0;
		}

	private:
		Downloader* m_downloader;
		FsSaver* m_fsSaver;
	};

	class MockDownloader : public Downloader
	{
	public:
		MOCK_CONST_METHOD1(DownloadData, std::string(const std::string&));
	};

	class MockFsSaver : public FsSaver
	{
	public:
		MOCK_CONST_METHOD2(SaveToFile, void(const std::string&, const std::string&));
	};

	class RawPtrPassingFixture : public Test
	{
	public:
		void SetUp()
		{
			auto downloadFile = std::make_unique<ClassToDownloadFile>(&m_downloader, &m_fs);
		}
	protected:
		MockDownloader m_downloader;
		MockFsSaver m_fs;
		std::unique_ptr<ClassToDownloadFile> m_downloadFile; //unique_ptr to create object with delay
	};
}

TEST(RawPtrPassing, Downloader_DownloadData)
{
	Downloader downloader;
	EXPECT_EQ(downloader.DownloadData("http://localhost/aaa.txt"), "FileContent");
}

TEST(RawPtrPassing, FsSaver_SaveToFile)
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

TEST(RawPtrPassing, DownloadFileProduceExpectedCalls)
{
	MockDownloader downloader;
	MockFsSaver fs;
	EXPECT_CALL(downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
	EXPECT_CALL(fs, SaveToFile("FileContent", "C:\\bbb.txt"));
	DownloadFile(&downloader, &fs);
}

TEST_F(RawPtrPassingFixture, DownloadFileProduceExpectedCalls_ClassVersion)
{
	EXPECT_CALL(m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
	EXPECT_CALL(m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
	m_downloadFile->DownloadFile();
}