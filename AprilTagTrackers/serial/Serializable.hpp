#pragma once

#include "CustomSerial.hpp"
#include "FileStorage.hpp"
#include "utils/Log.hpp"
#include "utils/Reflectable.hpp"

#include <exception>

namespace serial
{

namespace fsys = std::filesystem;

template <typename T>
inline bool TryWriteFile(const T& value, const fsys::path& filePath)
{
    FileStorage fs{};
    if (!fs.Open(filePath, FileStorage::Mode::Write)) return false;
    try
    {
        FileStorageWriter ctx{fs.GetForWriting()};
        ctx.Write(value);
    }
    catch (const std::exception& e)
    {
        ATT_LOG_ERROR("saving file '", filePath, "' ", e.what());
        return false;
    }
    return true;
}

template <typename T>
inline bool TryReadFile(T& outValue, const fsys::path& filePath)
{
    FileStorage fs{};
    if (!fs.Open(filePath, FileStorage::Mode::Read)) return false;
    try
    {
        const FileStorageReader ctx{fs.GetForReading()};
        ctx.Read(outValue);
    }
    catch (const std::exception& e)
    {
        ATT_LOG_ERROR("loading file '", filePath, "' ", e.what());
        return false;
    }
    return true;
}

template <typename TDerived>
class Serializable
{
public:
    Serializable() = default;
    explicit Serializable(fsys::path filePath) : mFilePath(std::move(filePath)) {}

    void SetPath(fsys::path filePath) { mFilePath = std::move(filePath); }

    bool Save()
    {
        FileStorage fs{};
        if (!fs.Open(mFilePath, FileStorage::Mode::Write)) return false;
        try
        {
            FileStorageWriter ctx{fs.GetForWriting()};
            detail::WriteReflectable(ctx, Reflect::DerivedThis<TDerived>(*this));
        }
        catch (const std::exception& e)
        {
            ATT_LOG_ERROR("saving file '", mFilePath, "' ", e.what());
            return false;
        }
        return true;
    }

    bool Load()
    {
        FileStorage fs{};
        if (!fs.Open(mFilePath, FileStorage::Mode::Read)) return false;
        try
        {
            FileStorageReader ctx{fs.GetForReading()};
            detail::ReadReflectable(ctx, Reflect::DerivedThis<TDerived>(*this));
        }
        catch (const std::exception& e)
        {
            ATT_LOG_ERROR("loading file '", mFilePath, "' ", e.what());
            return false;
        }
        return true;
    }

private:
    fsys::path mFilePath{};
};

} // namespace serial
