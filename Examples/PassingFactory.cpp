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

	class Factory
	{
	public:
		virtual std::shared_ptr<Downloader> GetDownloader() const
		{
			return std::make_shared<Downloader>();
		}

		virtual std::shared_ptr<FsSaver> GetFsSaver() const
		{
			return std::make_shared<FsSaver>();
		}
	};

	int DownloadFile(const Factory& factory) {
		const auto& downloadedData = factory.GetDownloader()->DownloadData("http://localhost/aaa.txt");
		factory.GetFsSaver()->SaveToFile(downloadedData, "C:\\bbb.txt");
		return 0;
	}

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

	class MockFactory : public Factory
	{
	public:
		MOCK_CONST_METHOD0(GetDownloader, std::shared_ptr<Downloader>());
		MOCK_CONST_METHOD0(GetFsSaver, std::shared_ptr<FsSaver>());
	};

	class PassingFactoryFixture : public Test
	{
	public:
		void SetUp()
		{
			m_downloader = std::make_shared<MockDownloader>();
			m_fs = std::make_shared<MockFsSaver>();
			ON_CALL(m_factory, GetDownloader()).WillByDefault(Return(m_downloader));
			ON_CALL(m_factory, GetFsSaver()).WillByDefault(Return(m_fs));
		}
	protected:
		std::shared_ptr<MockDownloader> m_downloader;
		std::shared_ptr<MockFsSaver> m_fs;
		MockFactory m_factory;
	};
}

TEST(PassingFactory, Downloader_DownloadData)
{
	Downloader downloader;
	EXPECT_EQ(downloader.DownloadData("http://localhost/aaa.txt"), "FileContent");
}

TEST(PassingFactory, FsSaver_SaveToFile)
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

TEST_F(PassingFactoryFixture, DownloadFileProduceExpectedCalls)
{
	EXPECT_CALL(*m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
	EXPECT_CALL(*m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
	DownloadFile(m_factory);
}