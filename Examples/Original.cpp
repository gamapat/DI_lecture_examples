#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

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

int DownloadFile(void) {
    CURL* curl;
    CURLcode res;
    char url[] = "http://localhost/aaa.txt";
    std::ofstream fs("C:\\bbb.txt");
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fs);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return 0;
}
}

TEST(OriginalDownloadFile, DownloadFileExpected)
{
    std::string fileName = "C:\\bbb.txt";
    EXPECT_FALSE(std::filesystem::exists(fileName));
    DownloadFile();
    ASSERT_TRUE(std::filesystem::exists(fileName));
    std::ifstream file(fileName);
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_EQ(str, "FileContent");
}