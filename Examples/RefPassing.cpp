#include <gmock/gmock.h>
#include <memory>

#include "Downloader.h"
#include "FsWrapper.h"

using namespace testing;

namespace
{
    int DownloadFile(Downloader& downloader, FsWrapper& fs) {
        const auto& downloadedData = downloader.DownloadData("http://localhost/aaa.txt");
        fs.SaveToFile(downloadedData, "C:\\bbb.txt");
        return 0;
    }

    class ClassToDownloadFile
    {
    public:
        ClassToDownloadFile(Downloader& downloader, FsWrapper& fs)
            : m_downloader(downloader)
            , m_fs(fs)
        {}

        int DownloadFile()
        {
            const auto& downloadedData = m_downloader.DownloadData("http://localhost/aaa.txt");
            m_fs.SaveToFile(downloadedData, "C:\\bbb.txt");
            return 0;
        }

    private:
        Downloader& m_downloader;
        FsWrapper& m_fs;
    };

    class MockDownloader : public Downloader
    {
    public:
        MOCK_CONST_METHOD1(DownloadData, std::string(const std::string&));
    };

    class MockFsWrapper : public FsWrapper
    {
    public:
        MOCK_CONST_METHOD2(SaveToFile, void(const std::string&, const std::string&));
    };

    class RefPassingFixture : public Test
    {
    public:
        void SetUp()
        {
            m_downloadFile = std::make_unique<ClassToDownloadFile>(m_downloader, m_fs);
        }
    protected:
        MockDownloader m_downloader;
        MockFsWrapper m_fs;
        std::unique_ptr<ClassToDownloadFile> m_downloadFile; //unique_ptr to create object with delay
    };
}

TEST(RefPassing, DownloadFileProduceExpectedCalls)
{
    MockDownloader downloader;
    MockFsWrapper fs;
    EXPECT_CALL(downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    DownloadFile(downloader, fs);
}

TEST_F(RefPassingFixture, DownloadFileProduceExpectedCalls_ClassVersion)
{
    EXPECT_CALL(m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    m_downloadFile->DownloadFile();
}