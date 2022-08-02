#pragma once

#include "FileStorage.hpp"
#include "Reader.hpp"
#include "utils/Assert.hpp"
#include "utils/Reflectable.hpp"
#include "Writer.hpp"

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
        Write(fs.GetForWriting(), Reflect::DerivedThis<TDerived>(*this));
        return true;
    }

    bool Load()
    {
        FileStorage fs{};
        if (!fs.Open(filePath, FileStorage::Mode::Read)) return false;
        Read(fs.GetForReading(), Reflect::DerivedThis<TDerived>(*this));
        return true;
    }

private:
    fsys::path filePath{};
};

} // namespace serial
