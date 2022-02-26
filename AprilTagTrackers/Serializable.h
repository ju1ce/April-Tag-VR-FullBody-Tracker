#pragma once

#include "Quaternion.h"
#include "Reflectable.h"
#include "Serializable.h"
#include <opencv2/aruco.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/core/quaternion.hpp>
#include <wx/string.h>

#include <iostream>

// Define a reflectable field with optional validator,
// within a class that derives from FileStorageSerializable
#define S_FIELD(arg_type, arg_name, ...)                                \
public:                                                                 \
    REFLECTABLE_VISITOR(interface_, arg_type, arg_name, ##__VA_ARGS__); \
    arg_type arg_name

// Helper for FS_COMMENT to use the special __COUNTER__ macro
#define FILESTORAGE_COMMENT_WITH_UID(arg_comment, arg_uid)                 \
private:                                                                   \
    REFLECTABLE_VISITOR(_comment_, FileStorageComment, comment_##arg_uid); \
    FileStorageComment comment_##arg_uid { arg_comment }

// Places a comment in the resulting yaml file, at the current line
#define FS_COMMENT(arg_comment) \
    FILESTORAGE_COMMENT_WITH_UID(arg_comment, __COUNTER__)

// Useful for defining a yaml object with a custom type marked within the file, mainly for documentation purpose
// Basically this uses the constructor to output "{" and the destructor to output "}"
#define FILESTORAGE_MAP_HELPER(arg_type) \
    cv::internal::WriteStructContext _wsc(fs, cv::String(), cv::FileNode::MAP + cv::FileNode::FLOW, arg_type)
#define FILESTORAGE_SEQ_HELPER(arg_type) \
    cv::internal::WriteStructContext _wsc(fs, cv::String(), cv::FileNode::SEQ + cv::FileNode::FLOW, arg_type)

class IFileStorageField : public Reflect::FieldIdentifier
{
public:
    IFileStorageField(std::string name) : Reflect::FieldIdentifier(std::move(name)) {}
    virtual void Serialize(cv::FileStorage& fs) const = 0;
    virtual void Deserialize(const cv::FileNode& fn) const = 0;
};

template <typename T>
class FileStorageField
    : public IFileStorageField,
      public Reflect::FieldReferenceWithValidatingAccessor<T>
{
public:
    FileStorageField(std::string name, T& field, Reflect::Validator<T> validator)
        : IFileStorageField(std::move(name)),
          Reflect::FieldReferenceWithValidatingAccessor<T>(field, validator) {}

    void Serialize(cv::FileStorage& fs) const override
    {
        fs << name << this->field;
    }
    void Deserialize(const cv::FileNode& fn) const override
    {
        const cv::FileNode& elem = fn[name];
        if (elem.empty()) return;
        elem >> this->field;
        if (this->validator != nullptr)
            this->validator(this->field);
    }
};

class FileStorageSerializable
    : public Reflect::Reflectable<IFileStorageField, FileStorageField>
{
public:
    explicit FileStorageSerializable(std::string file_path)
        : file_path(std::move(file_path)) {}

    bool Save() const;
    bool Load();

protected:
    void SerializeAll(cv::FileStorage& fs) const;
    void DeserializeAll(const cv::FileNode& fn);

    std::string file_path;
};

struct FileStorageComment
{
    const std::string str;
};

// Specialization for comments to not create a key
template <>
inline void FileStorageField<FileStorageComment>::Serialize(cv::FileStorage& fs) const
{
    fs.writeComment(field.str);
}
template <>
inline void FileStorageField<FileStorageComment>::Deserialize(const cv::FileNode& fn) const
{ /* empty */
}

// --- Custom file storage overloads for unimplemented types ---

inline cv::FileStorage& operator<<(cv::FileStorage& fs, const std::vector<cv::Ptr<cv::aruco::Board>>& boards)
{
    fs << "[";
    for (const auto& b : boards)
    {
        FILESTORAGE_MAP_HELPER("aruco::Board");
        fs << "ids" << b->ids;
        fs << "objPoints" << b->objPoints;
    }
    fs << "]";
    return fs;
}
inline void operator>>(const cv::FileNode& fn, std::vector<cv::Ptr<cv::aruco::Board>>& boards)
{
    auto it = fn.begin();
    boards.resize(it.remaining());

    for (int i = 0; i < boards.size(); i++, it++)
    {
        assert(it.remaining() > 0);
        if (boards[i].empty()) boards[i] = cv::makePtr<cv::aruco::Board>();

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
    s = wxString::FromUTF8(buf);
}

// OpenCV dosnt have an implementation for storing its own aruco config file, so here it is.
// This is a specialization instead of an overload, so that the params can be written
//  at root level instead of inside a named object
template <>
inline void FileStorageField<cv::Ptr<cv::aruco::DetectorParameters>>::Serialize(cv::FileStorage& fs) const
{
    fs << "adaptiveThreshWinSizeMin" << field->adaptiveThreshWinSizeMin;
    fs << "adaptiveThreshWinSizeMax" << field->adaptiveThreshWinSizeMax;
    fs << "adaptiveThreshWinSizeStep" << field->adaptiveThreshWinSizeStep;
    fs << "adaptiveThreshConstant" << field->adaptiveThreshConstant;
    fs << "minMarkerPerimeterRate" << field->minMarkerPerimeterRate;
    fs << "maxMarkerPerimeterRate" << field->maxMarkerPerimeterRate;
    fs << "polygonalApproxAccuracyRate" << field->polygonalApproxAccuracyRate;
    fs << "minCornerDistanceRate" << field->minCornerDistanceRate;
    fs << "minDistanceToBorder" << field->minDistanceToBorder;
    fs << "minMarkerDistanceRate" << field->minMarkerDistanceRate;
    fs << "cornerRefinementMethod" << field->cornerRefinementMethod;
    fs << "cornerRefinementWinSize" << field->cornerRefinementWinSize;
    fs << "cornerRefinementMaxIterations" << field->cornerRefinementMaxIterations;
    fs << "cornerRefinementMinAccuracy" << field->cornerRefinementMinAccuracy;
    fs << "markerBorderBits" << field->markerBorderBits;
    fs << "perspectiveRemovePixelPerCell" << field->perspectiveRemovePixelPerCell;
    fs << "perspectiveRemoveIgnoredMarginPerCell" << field->perspectiveRemoveIgnoredMarginPerCell;
    fs << "maxErroneousBitsInBorderRate" << field->maxErroneousBitsInBorderRate;
    fs << "minOtsuStdDev" << field->minOtsuStdDev;
    fs << "errorCorrectionRate" << field->errorCorrectionRate;

    // April :: User-configurable parameters.
    fs << "aprilTagQuadDecimate" << field->aprilTagQuadDecimate;
    fs << "aprilTagQuadSigma" << field->aprilTagQuadSigma;

    // April :: Internal variables
    fs << "aprilTagMinClusterPixels" << field->aprilTagMinClusterPixels;
    fs << "aprilTagMaxNmaxima" << field->aprilTagMaxNmaxima;
    fs << "aprilTagCriticalRad" << field->aprilTagCriticalRad;
    fs << "aprilTagMaxLineFitMse" << field->aprilTagMaxLineFitMse;
    fs << "aprilTagMinWhiteBlackDiff" << field->aprilTagMinWhiteBlackDiff;
    fs << "aprilTagDeglitch" << field->aprilTagDeglitch;

    // to detect white (inverted) markers
    fs << "detectInvertedMarker" << field->detectInvertedMarker;
}
template<>
inline void FileStorageField<cv::Ptr<cv::aruco::DetectorParameters>>::Deserialize(const cv::FileNode& fn) const
{
    cv::aruco::DetectorParameters::readDetectorParameters(fn, field);
}