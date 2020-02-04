#pragma once
#include <string>

class IDownloader
{
public:
    virtual std::string DownloadData(const std::string& url) const = 0;
};

class Downloader : public IDownloader
{
public:
    virtual std::string DownloadData(const std::string& url) const;
};

