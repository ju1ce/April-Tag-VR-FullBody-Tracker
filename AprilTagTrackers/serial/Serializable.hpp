#pragma once

#include "FileStorage.hpp"
#include "ReaderWriter.hpp"
#include "utils/Log.hpp"
#include "utils/Reflectable.hpp"

#include <exception>

namespace serial
{

namespace fsys = std::filesystem;

template <typename TDerived>
class Serializable
{
public:
    Serializable() = default;
    explicit Serializable(fsys::path _filePath) : filePath(std::move(_filePath)) {}

    void SetPath(fsys::path _filePath) { filePath = std::move(_filePath); }

    bool Save()
    {
        FileStorage fs{};
        if (!fs.Open(filePath, FileStorage::Mode::Write)) return false;
        try
        {
            WriteReflectable(fs.GetForWriting(), Reflect::DerivedThis<TDerived>(*this));
        }
        catch (const std::exception& e)
        {
            ATT_LOG_ERROR("saving file '", filePath, "' ", e.what());
            return false;
        }
        return true;
    }

    bool Load()
    {
        FileStorage fs{};
        if (!fs.Open(filePath, FileStorage::Mode::Read)) return false;
        try
        {
            ReadReflectable(fs.GetForReading(), Reflect::DerivedThis<TDerived>(*this));
        }
        catch (const std::exception& e)
        {
            ATT_LOG_ERROR("loading file '", filePath, "' ", e.what());
            return false;
        }
        return true;
    }

private:
    fsys::path filePath{};
};

} // namespace serial
