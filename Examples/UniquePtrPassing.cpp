#include <gmock/gmock.h>
#include <memory>

#include "Downloader.h"
#include "FsWrapper.h"

using namespace testing;

namespace
{
    int DownloadFile(std::unique_ptr<Downloader> downloader, std::unique_ptr<FsWrapper> fs) {
        const auto& downloadedData = downloader->DownloadData("http://localhost/aaa.txt");
        fs->SaveToFile(downloadedData, "C:\\bbb.txt");
        return 0;
    }

    class ClassToDownloadFile
    {
    public:
        ClassToDownloadFile(std::unique_ptr<Downloader> downloader, std::unique_ptr<FsWrapper> fs) 
            : m_downloader(std::move(downloader))
            , m_fs(std::move(fs)) 
        {}

        int DownloadFile()
        {
            const auto& downloadedData = m_downloader->DownloadData("http://localhost/aaa.txt");
            m_fs->SaveToFile(downloadedData, "C:\\bbb.txt");
            return 0;
        }

    private:
        std::unique_ptr<Downloader> m_downloader;
        std::unique_ptr<FsWrapper> m_fs;
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

    class UniquePtrPassingFixture : public Test
    {
    public:
        void SetUp()
        {
            auto downloader = std::make_unique<MockDownloader>();
            m_downloader = downloader.get();
            auto fs = std::make_unique<MockFsWrapper>();
            m_fs = fs.get();

            auto downloadFile = std::make_unique<ClassToDownloadFile>(std::move(downloader), std::move(fs));
        }
    protected:
        MockDownloader* m_downloader;
        MockFsWrapper* m_fs;
        std::unique_ptr<ClassToDownloadFile> m_downloadFile; //unique_ptr to create object with delay
    };
}

TEST(UniquePtrPassing, DownloadFileProduceExpectedCalls)
{
    auto downloader = std::make_unique<MockDownloader>();
    auto fs = std::make_unique<MockFsWrapper>();
    EXPECT_CALL(*downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(*fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    DownloadFile(std::move(downloader), std::move(fs));
}

TEST_F(UniquePtrPassingFixture, DownloadFileProduceExpectedCalls_ClassVersion)
{
    EXPECT_CALL(*m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(*m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    m_downloadFile->DownloadFile();
}