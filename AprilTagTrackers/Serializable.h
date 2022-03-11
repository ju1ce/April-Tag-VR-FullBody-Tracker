#pragma once

#include "Quaternion.h"
#include "Reflectable.h"
#include "Serializable.h"
#include <opencv2/aruco.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/core/quaternion.hpp>
#include <wx/string.h>

#include <iostream>

// Helper for FS_COMMENT to use the special __COUNTER__ macro
#define FILESTORAGE_COMMENT_WITH_UID(a_unique_id, a_comment) \
    REFLECTABLE_FIELD(private, _comment_, private, const FileStorageComment, _comment_##a_unique_id){a_comment}

// Places a comment in the resulting yaml file, at the current line
#define FS_COMMENT(a_comment) \
    FILESTORAGE_COMMENT_WITH_UID(__COUNTER__, a_comment)

/// IFieldVisitor
class IFileStorageField : public Reflect::FieldIdentifier
{
public:
    IFileStorageField(std::string name) : Reflect::FieldIdentifier(std::move(name)) {}
    virtual void Serialize(cv::FileStorage& fs) const = 0;
    virtual void Deserialize(const cv::FileNode& fn) const = 0;
};

/// FieldVisitorImpl
template <typename T>
class FileStorageField
    : public IFileStorageField,
      public Reflect::FieldValidateAccessor<T>
{
public:
    FileStorageField(std::string name, T& field, Reflect::Validator<T> validator)
        : IFileStorageField(std::move(name)),
          Reflect::FieldValidateAccessor<T>(field, validator) {}

    void Serialize(cv::FileStorage& fs) const override;
    void Deserialize(const cv::FileNode& fn) const override;
};

class FileStorageSerializable
    : public Reflect::Reflectable<IFileStorageField, FileStorageField>
{
public:
    explicit FileStorageSerializable(std::string file_path)
        : file_path(std::move(file_path)) {}
    /// Write to file_path
    bool Save() const;
    /// Read from file_path
    bool Load();

protected:
    void SerializeAll(cv::FileStorage& fs) const;
    void DeserializeAll(const cv::FileNode& fn);

    std::string file_path;
};

template <typename FieldType>
inline void FileStorageField<FieldType>::Serialize(cv::FileStorage& fs) const
{
    fs << this->name << this->field;
}
template <typename FieldType>
inline void FileStorageField<FieldType>::Deserialize(const cv::FileNode& fn) const
{
    const cv::FileNode& elem = fn[name];
    if (elem.empty()) return;
    elem >> this->field;

    if (this->validator != nullptr)
        this->validator(this->field);
}

struct FileStorageComment
{
    const std::string str;
};

template <>
inline void FileStorageField<FileStorageComment>::Serialize(cv::FileStorage& fs) const
{
    fs.writeComment(field.str);
}
template <>
inline void FileStorageField<FileStorageComment>::Deserialize(const cv::FileNode& fn) const
{ /* empty */
}

// OpenCV dosnt have an implementation for storing its own aruco config file, so here it is.
// This is a specialization instead of an overload, so that the params can be written
//  at root level instead of inside a named object
template <>
void FileStorageField<cv::Ptr<cv::aruco::DetectorParameters>>::Serialize(cv::FileStorage& fs) const;
template <>
inline void FileStorageField<cv::Ptr<cv::aruco::DetectorParameters>>::Deserialize(const cv::FileNode& fn) const
{
    cv::aruco::DetectorParameters::readDetectorParameters(fn, field);
}

// --- Custom file storage overloads for unimplemented types ---

inline cv::FileStorage& operator<<(cv::FileStorage& fs, const std::vector<cv::Ptr<cv::aruco::Board>>& boards)
{
    fs << "[";
    for (const auto& b : boards)
    {
        fs << "{";
        fs << "ids" << b->ids;
        fs << "objPoints" << b->objPoints;
        fs << "}";
    }
    fs << "]";
    return fs;
}
// This could be implemented as overload of file storage iterator?
inline void operator>>(const cv::FileNode& fn, std::vector<cv::Ptr<cv::aruco::Board>>& boards)
{
    auto it = fn.begin();
    boards.resize(it.remaining());
    for (int i = 0; i < boards.size(); i++, it++)
    {
        assert(it.remaining() > 0);
        // Resize might grow and initialize null cv::Ptr (alias of std::shared_ptr)
        if (boards[i].empty()) boards[i] = cv::makePtr<cv::aruco::Board>();
        // TODO: make our own aruco::Board, as we don't need to store the dictionary
        // Dictionary will be nullptr and should not be accessed.
        (*it)["ids"] >> boards[i]->ids;
        (*it)["objPoints"] >> boards[i]->objPoints;
    }
}

template <typename T>
inline cv::FileStorage& operator<<(cv::FileStorage& fs, const Quaternion<T>& q)
{
    fs << std::vector<T>({q.w, q.x, q.y, q.z});
    return fs;
}
template <typename T>
inline void operator>>(const cv::FileNode& fn, Quaternion<T>& q)
{
    auto it = fn.begin();
    (*(it++)) >> q.w;
    (*(it++)) >> q.x;
    (*(it++)) >> q.y;
    (*(it++)) >> q.z;
}

// On windows this converts from a widestring (utf16) to utf8, but on other platforms its free
inline cv::FileStorage& operator<<(cv::FileStorage& fs, const wxString& s)
{
    fs << s.utf8_str().data();
    return fs;
}
inline void operator>>(const cv::FileNode& fn, wxString& s)
{
    std::string buf;
    fn >> buf;
    // TODO: can the need for a temporary be eliminated? check FileNode::readObj()
    s = wxString::FromUTF8Unchecked(buf);
}
