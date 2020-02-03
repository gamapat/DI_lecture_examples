#include <gmock/gmock.h>
#include <memory>

#include "Downloader.h"
#include "FsWrapper.h"

using namespace testing;

namespace
{
	class Factory
	{
	public:
		virtual std::shared_ptr<Downloader> GetDownloader() const
		{
			return std::make_shared<Downloader>();
		}

		virtual std::shared_ptr<FsWrapper> GetFsWrapper() const
		{
			return std::make_shared<FsWrapper>();
		}
	};

	int DownloadFile(const Factory& factory) {
		const auto& downloadedData = factory.GetDownloader()->DownloadData("http://localhost/aaa.txt");
		factory.GetFsWrapper()->SaveToFile(downloadedData, "C:\\bbb.txt");
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

	class MockFactory : public Factory
	{
	public:
		MOCK_CONST_METHOD0(GetDownloader, std::shared_ptr<Downloader>());
		MOCK_CONST_METHOD0(GetFsWrapper, std::shared_ptr<FsWrapper>());
	};

	class PassingFactoryFixture : public Test
	{
	public:
		void SetUp()
		{
			m_downloader = std::make_shared<MockDownloader>();
			m_fs = std::make_shared<MockFsWrapper>();
			ON_CALL(m_factory, GetDownloader()).WillByDefault(Return(m_downloader));
			ON_CALL(m_factory, GetFsWrapper()).WillByDefault(Return(m_fs));
		}
	protected:
		std::shared_ptr<MockDownloader> m_downloader;
		std::shared_ptr<MockFsWrapper> m_fs;
		MockFactory m_factory;
	};
}

TEST_F(PassingFactoryFixture, DownloadFileProduceExpectedCalls)
{
	EXPECT_CALL(*m_downloader, DownloadData("http://localhost/aaa.txt")).WillOnce(Return("FileContent"));
	EXPECT_CALL(*m_fs, SaveToFile("FileContent", "C:\\bbb.txt"));
	DownloadFile(m_factory);
}