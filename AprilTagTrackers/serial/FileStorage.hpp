#pragma once

#include <opencv2/core/persistence.hpp>

#include <filesystem>

namespace serial
{

namespace fsys = std::filesystem;

class FileStorage
{
public:
    enum class Mode
    {
        Write,
        Read
    };

    ~FileStorage()
    {
        if (storage.isOpened()) storage.release();
    }

    bool Open(const fsys::path& filePath, Mode mode);

    cv::FileStorage& GetForWriting() { return storage; }
    cv::FileNode GetForReading() const { return storage.root(); }

private:
    cv::FileStorage storage{};
};

} // namespace serial
