#include <gmock/gmock.h>
#include <memory>

#include "Downloader.h"
#include "FsWrapper.h"

using namespace testing;

namespace
{
    std::shared_ptr<Downloader> g_downloader = std::make_shared<Downloader>();
    std::shared_ptr<Downloader> GetDownloaderPtr() { return g_downloader; }
    const Downloader& GetDownloader() { return *g_downloader; }
    void SetDownloader(std::shared_ptr<Downloader> downloader) { g_downloader = downloader; }

    std::shared_ptr<FsWrapper> g_fs = std::make_shared<FsWrapper>();
    std::shared_ptr<FsWrapper> GetFsPtr() { return g_fs; }
    const FsWrapper& GetFs() { return *g_fs; }
    void SetFs(std::shared_ptr<FsWrapper> fs) { g_fs = fs; }

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

    class MockFsWrapper : public FsWrapper
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
            m_fs = std::make_shared<MockFsWrapper>();
            SetDownloader(m_downloader);
            SetFs(m_fs);
        }

        void TearDown()
        {
            SetDownloader(std::make_shared<Downloader>());
            SetFs(std::make_shared<FsWrapper>());
        }

    protected:
        std::shared_ptr<MockDownloader> m_downloader;
        std::shared_ptr<MockFsWrapper> m_fs;        
    };
}

TEST_F(UseSingletonFixture, DownloadFileProduceExpectedCalls)
{
    EXPECT_CALL(*m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
    EXPECT_CALL(*m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
    DownloadFile();
}