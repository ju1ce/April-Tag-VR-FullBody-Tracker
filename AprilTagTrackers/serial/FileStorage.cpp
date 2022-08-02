#include "FileStorage.hpp"

namespace serial
{

bool FileStorage::Open(const fsys::path& filePath, Mode mode)
{
    ATT_ASSERT(!filePath.empty());
    ATT_ASSERT(filePath.has_parent_path());
    cv::FileStorage::Mode storageMode = cv::FileStorage::Mode::READ;
    if (mode == Mode::Read)
    {
        if (!fsys::exists(filePath)) return false;
        if (fsys::is_empty(filePath)) return false;
        storageMode = cv::FileStorage::Mode::READ;
    }
    else if (mode == Mode::Write)
    {
        if (!fsys::exists(filePath.parent_path()))
            fsys::create_directories(filePath.parent_path());
        storageMode = cv::FileStorage::Mode::WRITE;
    }
    else ATT_ASSERT(false, "unhandled FileStorage::Mode ", mode);

    try
    {
        if (!storage.open(filePath.generic_string(), storageMode)) return false;
    }
    catch (const std::exception& e)
    {
        ATT_LOG_ERROR(e.what());
        return false;
    }
    return true;
}

} // namespace serial
