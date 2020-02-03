#include "Downloader.h"
#include <sstream>
#include <curl/curl.h>
#include <gtest/gtest.h>

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
}

std::string Downloader::DownloadData(const std::string& url) const
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

TEST(Downloader, DownloadData)
{
	Downloader downloader;
	EXPECT_EQ(downloader.DownloadData("http://localhost/aaa.txt"), "FileContent");
}