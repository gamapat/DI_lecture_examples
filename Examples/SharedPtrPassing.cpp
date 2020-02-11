#include <gmock/gmock.h>
#include <memory>

#include "Downloader.h"
#include "FsWrapper.h"

using namespace testing;

namespace
{
    int DownloadFile(std::shared_ptr<Downloader> downloader, std::shared_ptr<FsWrapper> fs) {
        const auto& downloadedData = downloader->DownloadData("http://localhost/aaa.txt");
        fs->SaveToFile(downloadedData, "C:\\bbb.txt");
        return 0;
    }

    class ClassToDownloadFile
    {
    public:
        ClassToDownloadFile(std::shared_ptr<Downloader> downloader, std::shared_ptr<FsWrapper> fs)
            : m_downloader(downloader)
            , m_fs(fs)
        {}

        int DownloadFile()
        {
            const auto& downloadedData = m_downloader->DownloadData("http://localhost/aaa.txt");
            m_fs->SaveToFile(downloadedData, "C:\\bbb.txt");
            return 0;
        }

    private:
        std::shared_ptr<Downloader> m_downloader;
        std::shared_ptr<FsWrapper> m_fs;
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

    class SharedPtrPassingFixture : public Test
    {
    public:
        void SetUp()
        {
            m_downloader = std::make_shared<MockDownloader>();
            m_fs = std::make_shared<MockFsWrapper>();

            m_downloadFile = std::make_unique<ClassToDownloadFile>(m_downloader, m_fs);
        }
    protected:
        std::shared_ptr<MockDownloader> m_downloader;
        std::shared_ptr<MockFsWrapper> m_fs;
        std::unique_ptr<ClassToDownloadFile> m_downloadFile; //unique_ptr to create object with delay
    };
}

TEST(SharedPtrPassing, DownloadFileProduceExpectedCalls)
{
    auto downloader = std::make_shared<MockDownloader>();
    auto fs = std::make_shared<MockFsWrapper>();
    EXPECT_CALL(*downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(*fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    DownloadFile(downloader, fs);
}

TEST_F(SharedPtrPassingFixture, DownloadFileProduceExpectedCalls_ClassVersion)
{
    EXPECT_CALL(*m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(*m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    m_downloadFile->DownloadFile();
}