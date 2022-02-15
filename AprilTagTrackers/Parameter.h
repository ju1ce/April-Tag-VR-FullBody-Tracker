#pragma once
#include "Quaternion.h"
#include <iostream>
#include <memory>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <unordered_map>
#include <wx/string.h>

#include "Language.h"

// This typedef hopefully makes it more obvious of the purpose of wxString, 
//  and it should only be used for frontend user facing strings.
// On Windows this will be utf-16 and is essentially a typedef for std::wstring,
//  other platforms this is an encoding aware utf-8 string.
// May be useful for windows file paths aswell, although opencv and other
//  libraries don't always support io with widestring paths anyway.
// In general, all unicode string literals should be in the translation yaml files, 
//  and not in a source file. 
typedef wxString UniStr;

// Base class for polymorphism, to store and serialize params in a type agnostic way
struct ParameterBase
{
    virtual ~ParameterBase() {}
    virtual void Validate() = 0;
    virtual void Serialize(cv::FileStorage &fs) const = 0;
    virtual void Deserialize(const cv::FileNode &fn) = 0;
};

// Typed version of the parameter to store value.
template <typename T>
struct Parameter : public ParameterBase
{
    Parameter(void (*validate)(T &value))
        : value(), validate(validate) {}
    Parameter(const T &default_value, void (*validate)(T &value))
        : value(default_value), has_default_value(true), default_value(default_value), validate(validate) {}

    T value;
    bool has_default_value = false;
    const T default_value = T();
    void (*validate)(T &value) = nullptr;
#ifdef _DEBUG
    const size_t tinfo = typeid(T).hash_code();
#endif

    virtual void Validate() override
    {
        if (validate) validate(value);
    }
    virtual void Serialize(cv::FileStorage &fs) const override
    {
        fs << value;
    }
    virtual void Deserialize(const cv::FileNode &fn) override
    {
        if (!fn.empty())
        {
            fn >> value;
            Validate();
        }
        else if (has_default_value)
        {
            value = default_value;
        }
    }
};

// Like the node of a tree
class ParamNode
{
public:
    ~ParamNode()
    {
        for (auto &p : params)
        {
            delete p.second;
        }
    }

    // T may be able to be deduced via the return type at the callsite
    template <typename T>
    auto &Ref(const std::string &name)
    {
        assert(params.count(name) > 0);
        assert(typeid(T).hash_code() == static_cast<Parameter<T> *>(params.at(name))->tinfo);
        return static_cast<Parameter<T> *>(params.at(name))->value;
    }

    template <typename T>
    const auto &Get(const std::string &name) const
    {
        assert(params.count(name) > 0);
        assert(typeid(T).hash_code() == static_cast<Parameter<T> *>(params.at(name))->tinfo);
        return static_cast<const Parameter<T> *>(params.at(name))->value;
    }

    void Validate(const std::string &name)
    {
        assert(params.count(name) > 0);
        params.at(name)->Validate();
    }

    ParamNode &Node(const std::string &n) { return Ref<ParamNode>(n); }

#define GET_TYPE_ALIAS(name, T) \
    const T &name(const std::string &n) const { return Get<T>(n); }

    GET_TYPE_ALIAS(Int, int)
    GET_TYPE_ALIAS(Float, float)
    GET_TYPE_ALIAS(Bool, bool)
    GET_TYPE_ALIAS(Str, std::string)
    GET_TYPE_ALIAS(UStr, UniStr)
    GET_TYPE_ALIAS(Mat, cv::Mat)

#undef GET_TYPE_ALIAS

    friend cv::FileStorage &operator<<(cv::FileStorage &fs, const ParamNode &pn)
    {
        cv::internal::WriteStructContext wsc(fs, cv::String(), cv::FileNode::MAP + cv::FileNode::FLOW);
        for (const auto &p : pn.params)
        {
            fs << p.first;
            p.second->Serialize(fs);
        }
        return fs;
    }
    friend void operator>>(const cv::FileNode &fn, ParamNode &pn)
    {
        for (auto &p : pn.params)
        {
            p.second->Deserialize(fn[p.first]);
        }
    }

    // Add a parameter optionally specifying default and a validation lambda
    template <typename T>
    void Add(std::string &&name, void (*validate)(T &value) = nullptr)
    {
        assert(params.count(name) == 0);
        auto ptr = new Parameter<T>(validate);
        params.insert({std::move(name), static_cast<ParameterBase *>(ptr)});
    }
    template <typename T>
    void Add(std::string &&name, const T &default_value, void (*validate)(T &value) = nullptr)
    {
        assert(params.count(name) == 0);
        auto ptr = new Parameter<T>(default_value, validate);
        params.insert({std::move(name), static_cast<ParameterBase *>(ptr)});
    }

protected:
    // Could be made private, but would need to add protected access to the iterator
    // Requirements: fast random access, 
    // slow insert, slow iteration, sorted iteration, no remove
    std::unordered_map<std::string, ParameterBase *> params;
};

// Implement this for each type that isn't serializable.
// It's fine to make it inline/in the header.
// ```
// friend cv::FileStorage &operator<<(cv::FileStorage &fs, const Type &v) {}
// friend void operator>>(const cv::FileNode &fn, Type &v) {}
// ```
// See below for example of adding serialization to a library's type

// TODO: Is this where extensions for other libraries should go? probably?
static cv::FileStorage &operator<<(cv::FileStorage &fs, const wxString &s)
{
    fs << s.utf8_str().data();
    return fs;
}
static void operator>>(const cv::FileNode &fn, wxString &s)
{
    std::string buf;
    fn >> buf;
    s = wxString::FromUTF8(buf);
}


class Parameters
{
public:
    std::string version = "0.5.5";
    std::string driverversion = "0.5.5";

    Parameters();
    void Load();
    void Save();
    std::string cameraAddr = "0";
    int cameraApiPreference = 0;
    cv::Mat camMat;
    cv::Mat distCoeffs;
    cv::Mat stdDeviationsIntrinsics;
    std::vector<double> perViewErrors;
    std::vector<std::vector<cv::Point2f>> allCharucoCorners;
    std::vector<std::vector<int>> allCharucoIds;
    std::vector<cv::Ptr<cv::aruco::Board>> trackers;
    int trackerNum = 1;
    double markerSize = 0.05;
    int numOfPrevValues = 5;
    double quadDecimate = 1;
    double searchWindow = 0.25;
    bool usePredictive = true;
    int calibrationTracker = 0;
    bool ignoreTracker0 = false;
    bool rotateCl = false;
    bool rotateCounterCl = false;
    bool coloredMarkers = true;
    double calibOffsetX = 0;
    double calibOffsetY = 100;
    double calibOffsetZ = 100;
    double calibOffsetA = 180;
    double calibOffsetB = 0;
    double calibOffsetC = 0;
    bool circularWindow = true;
    double smoothingFactor = 0.5;
    int camFps = 30;
    int camHeight = 0;
    int camWidth = 0;
    cv::Mat wtranslation;
    Quaternion<double> wrotation;
    bool cameraSettings = false;
    bool chessboardCalib = false;
    double camLatency = 0;
    bool circularMarkers = false;
    double trackerCalibDistance = 0.5;
    int cameraCalibSamples = 15;
    bool settingsParameters = false;
    double cameraAutoexposure = 0;
    double cameraExposure = 0;
    double cameraGain = 0;
    bool trackerCalibCenters = false;
    float depthSmoothing = 0;
    float additionalSmoothing = 0;
    int markerLibrary = 0;
    int markersPerTracker = 45;
    int languageSelection = 0;
    double calibScale = 1;

    cv::Ptr<cv::aruco::DetectorParameters> aruco_params = cv::aruco::DetectorParameters::create();

    Lang language;
};
