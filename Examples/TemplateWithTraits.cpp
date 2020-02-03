#include <gmock/gmock.h>

#include "Downloader.h"
#include "FsWrapper.h"

using namespace testing;

namespace
{
	class ProdTraits
	{
	public:
		using Downloader = Downloader;
		using Fs = FsWrapper;
	};

	template <class T>
	int DownloadFile(const typename T::Downloader& downloader, const typename T::Fs& fs) {
		const auto& downloadedData = downloader.DownloadData("http://localhost/aaa.txt");
		fs.SaveToFile(downloadedData, "C:\\bbb.txt");
		return 0;
	}

	class MockDownloader
	{
	public:
		MOCK_CONST_METHOD1(DownloadData, std::string(const std::string&));
	};

	class MockFsWrapper
	{
	public:
		MOCK_CONST_METHOD2(SaveToFile, void(const std::string&, const std::string&));
	};

	struct TestTraits
	{
		using Downloader = MockDownloader;
		using Fs = MockFsWrapper;
	};
}

TEST(TemplateWithTraits, DownloadFileProduceExpectedCalls)
{
	TestTraits::Downloader downloader;
	TestTraits::Fs fs;
	EXPECT_CALL(downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
	EXPECT_CALL(fs, SaveToFile("FileContent", "C:\\bbb.txt"));
	DownloadFile<TestTraits>(downloader, fs);
}