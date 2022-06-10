#pragma once

#include "Debug.h"
#include "Quaternion.h"
#include "Reflectable.h"
#include "ValidatorProxy.h"

#include <opencv2/aruco.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/core/quaternion.hpp>

#include <filesystem>

// clang-format off
#define FILESTORAGE_COMMENT_WITH_ID(a_id, a_commentStr)   \
    REFLECTABLE_FIELD_DATA(const FS::Comment, REFLECTABLE_CONCAT(_comment_, a_id)); \
    static constexpr const FS::Comment REFLECTABLE_CONCAT(_comment_, a_id) { a_commentStr }
// clang-format on

#define FS_COMMENT(a_commentStr) \
    FILESTORAGE_COMMENT_WITH_ID(__LINE__, a_commentStr)

/// OpenCV FileStorage serialization.
namespace FS
{

using Path = std::filesystem::path;
template <typename T>
using Valid = ValidatorProxy<T>;

template <typename ST>
class Serializable
{
public:
    Serializable() {}
    Serializable(Path _filePath) : filePath(std::move(_filePath)) {}

    bool Save() const;
    bool Load();

protected:
    void SetPath(Path path) { filePath = std::move(path); }

private:
    Path filePath;
};

template <typename RT>
inline void WriteEach(cv::FileStorage& fs, const RT& reflType);
template <typename RT>
inline void ReadEach(const cv::FileNode& fn, RT& reflType);

} // namespace FS

template <typename T>
inline std::enable_if_t<Reflect::IsReflectableV<T>> Write(cv::FileStorage& fs, const T& field)
{
    fs << "{";
    FS::WriteEach(fs, field);
    fs << "}";
}
template <typename T>
inline std::enable_if_t<Reflect::IsReflectableV<T>> Read(const cv::FileNode& fn, T& field)
{
    FS::ReadEach(fn, field);
}

template <typename T>
inline std::enable_if_t<!Reflect::IsReflectableV<T>> Write(cv::FileStorage& fs, const T& field)
{
    fs << field;
}
template <typename T>
inline std::enable_if_t<!Reflect::IsReflectableV<T>> Read(const cv::FileNode& fn, T& field)
{
    fn >> field;
}

/*
// Called first to write/read name to node, and optionally call Write/Read
inline void WriteNode(cv::FileStorage& fs, const char* name, const FieldType& field) {}
inline void ReadNode(const cv::FileNode& fn, const char* name, FieldType& field) {}

// Called second, to write to named node
inline void Write(cv::FileStorage&, const FieldType& field) {}
inline void Read(const cv::FileNode&, FieldType&) {}
*/

// -- Overloaded Write and Read without name --

inline void Write(cv::FileStorage& fs, const cv::Ptr<cv::aruco::Board>& board)
{
    fs << "{";
    fs << "ids" << board->ids;
    fs << "objPoints" << board->objPoints;
    fs << "}";
}
inline void Read(const cv::FileNode& fn, cv::Ptr<cv::aruco::Board>& board)
{
    // Resize might grow and initialize null cv::Ptr (alias of std::shared_ptr)
    if (board.empty()) board = cv::makePtr<cv::aruco::Board>();
    // TODO: make our own aruco::Board, as we don't need to store the dictionary
    // Dictionary will be nullptr
    fn["ids"] >> board->ids;
    fn["objPoints"] >> board->objPoints;
}

// TODO: Use cv:Quat
template <typename T>
inline void Write(cv::FileStorage& fs, const Quaternion<T>& q)
{
    fs << "[" << q.w << q.x << q.y << q.z << "]";
}
template <typename T>
inline void Read(const cv::FileNode& fn, Quaternion<T>& q)
{
    auto it = fn.begin();
    Read(*(it++), q.w);
    Read(*(it++), q.x);
    Read(*(it++), q.y);
    Read(*(it++), q.z);
}

template <typename T>
inline void Write(cv::FileStorage& fs, const std::vector<T>& v)
{
    if constexpr (std::is_integral_v<T>)
        fs << "[:"; // condensed flow yaml structure
    else
        fs << "[";
    for (const auto& elem : v)
        Write(fs, elem);
    fs << "]";
}
template <typename T>
inline void Read(const cv::FileNode& fn, std::vector<T>& v)
{
    auto it = fn.begin();
    const size_t length = it.remaining();
    v.resize(length);

    for (size_t i = 0; i < length; i++)
        Read(*(it++), v[i]);
}

namespace FS
{

// -- Overloaded Write and Read with name --

template <typename T>
inline void WriteNode(cv::FileStorage& fs, const char* name, const T& field)
{
    fs << name;
    Write(fs, field);
}
template <typename T>
inline void ReadNode(const cv::FileNode& fn, const char* name, T& field)
{
    const cv::FileNode& elem = fn[name];
    if (elem.empty()) return;
    Read(elem, field);
}

struct Comment
{
    const char* const str = nullptr;
};

inline void WriteNode(cv::FileStorage& fs, const char*, const Comment& field)
{
    fs.writeComment(field.str);
}
inline void ReadNode(const cv::FileNode&, const char*, const Comment&)
{ /*unused*/
}

// OpenCV dosnt have an implementation for storing its own aruco config file, so here it is.
// This will expand the params in-place in the object, instead of under some sub-object.
void WriteNode(cv::FileStorage& fs, const char*, const cv::Ptr<cv::aruco::DetectorParameters>& field);
inline void ReadNode(const cv::FileNode& fn, const char*, cv::Ptr<cv::aruco::DetectorParameters>& field)
{
    cv::aruco::DetectorParameters::readDetectorParameters(fn, field);
}

template <typename ST>
inline bool Serializable<ST>::Save() const
{
    ATASSERT("filePath is not empty.", !filePath.empty());
    cv::FileStorage fs{filePath.generic_string(), cv::FileStorage::WRITE};
    if (!fs.isOpened()) return false;
    WriteEach(fs, Reflect::DerivedThis<ST>(*this));
    fs.release();
    return true;
}

template <typename ST>
inline bool Serializable<ST>::Load()
{
    ATASSERT("filePath is not empty.", !filePath.empty());
    if (!std::filesystem::exists(filePath)) return false;
    if (std::filesystem::is_empty(filePath)) return false;
    cv::FileStorage fs;
    try
    {
        if (!fs.open(filePath.generic_string(), cv::FileStorage::READ))
            return false;
    }
    catch (const std::exception& e)
    {
        ATERROR(e.what());
        return false;
    }
    if (!fs.isOpened()) return false;
    ReadEach(fs.root(), Reflect::DerivedThis<ST>(*this));
    fs.release();
    return true;
}

template <typename RT>
inline void WriteEach(cv::FileStorage& fs, const RT& reflType)
{
    Reflect::ForEach(
        reflType,
        [&](const char* name, const auto& field)
        {
            WriteNode(fs, name, field);
        });
}

template <typename RT>
inline void ReadEach(const cv::FileNode& fn, RT& reflType)
{
    Reflect::ForEach(
        reflType,
        [&](const char* name, auto& field)
        {
            ReadNode(fn, name, field);
        });
}

}; // namespace FS
