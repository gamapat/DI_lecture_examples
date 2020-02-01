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

	std::shared_ptr<Downloader> g_downloader = std::make_shared<Downloader>();
	std::shared_ptr<Downloader> GetDownloaderPtr() { return g_downloader; }
	const Downloader& GetDownloader() { return *g_downloader; }
	void SetDownloader(std::shared_ptr<Downloader> downloader) { g_downloader = downloader; }

	std::shared_ptr<FsSaver> g_fs = std::make_shared<FsSaver>();
	std::shared_ptr<FsSaver> GetFsPtr() { return g_fs; }
	const FsSaver& GetFs() { return *g_fs; }
	void SetFs(std::shared_ptr<FsSaver> fs) { g_fs = fs; }

	int DownloadFile() {
		const auto& downloadedData = GetDownloader().DownloadData("http://localhost/aaa.txt");
		GetFs().SaveToFile(downloadedData, "C:\\bbb.txt");
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

	class UseSingletonFixture : public Test
	{
	public:
		void SetUp()
		{
			m_downloader = std::make_shared<MockDownloader>();
			m_fs = std::make_shared<MockFsSaver>();
			SetDownloader(m_downloader);
			SetFs(m_fs);
		}

		void TearDown()
		{
			SetDownloader(std::make_shared<Downloader>());
			SetFs(std::make_shared<FsSaver>());
		}

	protected:
		std::shared_ptr<MockDownloader> m_downloader;
		std::shared_ptr<MockFsSaver> m_fs;		
	};
}

TEST(UseSingleton, Downloader_DownloadData)
{
	Downloader downloader;
	EXPECT_EQ(downloader.DownloadData("http://localhost/aaa.txt"), "FileContent");
}

TEST(UseSingleton, FsSaver_SaveToFile)
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

TEST_F(UseSingletonFixture, DownloadFileProduceExpectedCalls)
{
	EXPECT_CALL(*m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
	EXPECT_CALL(*m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
	DownloadFile();
}