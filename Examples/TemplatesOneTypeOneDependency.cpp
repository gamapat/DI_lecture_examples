#include <gmock/gmock.h>

#include "Downloader.h"
#include "FsWrapper.h"

using namespace testing;

namespace
{
    template <typename TDownloader, typename TFsWrapper>
    int DownloadFile(const TDownloader& downloader, const TFsWrapper& fs) {
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
}

TEST(Template, DownloadFileProduceExpectedCalls)
{
    MockDownloader downloader;
    MockFsWrapper fs;
    EXPECT_CALL(downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    DownloadFile(downloader, fs);
}