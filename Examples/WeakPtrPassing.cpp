#include <gmock/gmock.h>
#include <memory>

#include "Downloader.h"
#include "FsWrapper.h"

using namespace testing;

namespace
{
    int DownloadFile(std::weak_ptr<Downloader> weakDownloader, std::weak_ptr<FsWrapper> weakFs) {
        auto downloader = weakDownloader.lock();
        auto fs = weakFs.lock();
        if (downloader && fs)
        {
            const auto& downloadedData = downloader->DownloadData("http://localhost/aaa.txt");
            fs->SaveToFile(downloadedData, "C:\\bbb.txt");
            return 0;
        }
        return (!downloader ? 1 : 0) + (!fs ? 2 : 0);
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
            auto downloader = m_downloader.lock();
            auto fs = m_fs.lock();
            if (downloader && fs)
            {
                const auto& downloadedData = downloader->DownloadData("http://localhost/aaa.txt");
                fs->SaveToFile(downloadedData, "C:\\bbb.txt");
                return 0;
            }
            return (!downloader ? 1 : 0) + (!fs ? 2 : 0);
        }

    private:
        std::weak_ptr<Downloader> m_downloader;
        std::weak_ptr<FsWrapper> m_fs;
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

    class WeakPtrPassingFixture : public Test
    {
    public:
        void SetUp()
        {
            m_downloader = std::make_shared<MockDownloader>();
            m_fs = std::make_shared<MockFsWrapper>();

            auto downloadFile = std::make_unique<ClassToDownloadFile>(m_downloader, m_fs);
        }
    protected:
        std::shared_ptr<MockDownloader> m_downloader;
        std::shared_ptr<MockFsWrapper> m_fs;
        std::unique_ptr<ClassToDownloadFile> m_downloadFile; //unique_ptr to create object with delay
    };
}

TEST(WeakPtrPassing, DownloadFileProduceExpectedCalls)
{
    auto downloader = std::make_shared<MockDownloader>();
    auto fs = std::make_shared<MockFsWrapper>();
    EXPECT_CALL(*downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(*fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    DownloadFile(downloader, fs);
}

TEST(WeakPtrPassing, DownlaodFileReturnsNonZeroStatus)
{
    std::weak_ptr<MockDownloader> weakDownloader;
    std::weak_ptr<MockFsWrapper> weakFs;
    {
        auto downloader = std::make_shared<MockDownloader>();
        weakDownloader = downloader;
        auto fs = std::make_shared<MockFsWrapper>();
        weakFs = fs;
    }
    EXPECT_EQ(DownloadFile(weakDownloader, weakFs), 3);
    {
        auto downloader = std::make_shared<MockDownloader>();
        weakDownloader = downloader;
        {
            auto fs = std::make_shared<MockFsWrapper>();
            weakFs = fs;
        }
        EXPECT_EQ(DownloadFile(weakDownloader, weakFs), 2);
    }
    {
        auto fs = std::make_shared<MockFsWrapper>();
        weakFs = fs;
        {
            auto downloader = std::make_shared<MockDownloader>();
            weakDownloader = downloader;
        }
        EXPECT_EQ(DownloadFile(weakDownloader, weakFs), 1);
    }
}

TEST_F(WeakPtrPassingFixture, DownloadFileProduceExpectedCalls_ClassVersion)
{
    EXPECT_CALL(*m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(*m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    m_downloadFile->DownloadFile();
}

TEST(WeakPtrPassing, DownlaodFileReturnsNonZeroStatus_ClassVersion)
{
    std::unique_ptr<ClassToDownloadFile> downloadFile;
    {
        auto downloader = std::make_shared<MockDownloader>();
        auto fs = std::make_shared<MockFsWrapper>();
        downloadFile = std::make_unique<ClassToDownloadFile>(downloader, fs);
    }
    EXPECT_EQ(downloadFile->DownloadFile(), 3);
    {
        auto downloader = std::make_shared<MockDownloader>();
        {
            auto fs = std::make_shared<MockFsWrapper>();
            downloadFile = std::make_unique<ClassToDownloadFile>(downloader, fs);
        }
        EXPECT_EQ(downloadFile->DownloadFile(), 2);
    }
    {
        auto fs = std::make_shared<MockFsWrapper>();
        {
            auto downloader = std::make_shared<MockDownloader>();
            downloadFile = std::make_unique<ClassToDownloadFile>(downloader, fs);
        }
        EXPECT_EQ(downloadFile->DownloadFile(), 1);
    }
}