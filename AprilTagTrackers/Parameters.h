#pragma once
#include "Quaternion.h"
#include <iostream>
#include <memory>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <unordered_map>
#include <wx/string.h>

//#include "Language_English.h"
#include "Language.h"

// Base class for polymorphism, to store and serialize params in a type agnostic
// way
struct ParameterBase
{
    virtual ~ParameterBase() {}
    virtual void Serialize(cv::FileStorage &fs, const std::string &name = "") const = 0;
    virtual void Deserialize(const cv::FileNode &fn) = 0;
    virtual void Validate() = 0;
};

// Typed version of the parameter, generated for each type
template <typename T>
struct Parameter : public ParameterBase
{
    Parameter(const T &default_value, void (*validate)(T &value))
        : value(default_value), default_value(default_value), validate(validate) {}

    T value;
    const T &default_value;
    void (*validate)(T &value);

    virtual void Serialize(cv::FileStorage &fs, const std::string &name = "") const override
    {
        fs << name << value;
    }
    virtual void Deserialize(const cv::FileNode &fn) override
    {
        if (!fn.empty())
        {
            fn >> value;
            if (validate) validate(value);
        }
        else
            value = default_value;
    }
    virtual void Validate() override
    {
        if (validate) validate(value);
    }

    virtual ~Parameter() override {}
};

class ParamNode
{
public:
    ParamNode() {}
    ParamNode(ParamNode &&node) noexcept
        : params(std::move(node.params)) {}
    ~ParamNode()
    {
        for (auto &p : params)
        {
            delete p.second;
        }
    }

    ParamNode(const ParamNode &) = delete;
    void operator=(const ParamNode &) = delete;

    // T may be able to be deduced via the return type at the callsite
    template <typename T>
    auto &Ref(const std::string &name)
    {
        return static_cast<Parameter<T> *>(params.at(name))->value;
    }

    template <typename T>
    const auto &Get(const std::string &name) const
    {
        return static_cast<const Parameter<T> *>(params.at(name))->value;
    }

    template <typename T>
    void Validate(const std::string &name)
    {   
        auto& val = Ref<T>(name);
        static_cast<Parameter<T> *>(params.at(name))->validate(val);
    }

#define REF_TYPE_ALIAS(name, T) \
    T &name(const std::string &(name)) { return Ref<T>(name); }

    REF_TYPE_ALIAS(Int, int)
    REF_TYPE_ALIAS(Float, float)
    REF_TYPE_ALIAS(Bool, bool)
    REF_TYPE_ALIAS(String, std::string)
    REF_TYPE_ALIAS(WString, wxString)
    REF_TYPE_ALIAS(Mat, cv::Mat)
    REF_TYPE_ALIAS(Node, ParamNode)

#undef REF_TYPE_ALIAS

protected:
    // Add a parameter optionally specifying default and a validation lambda
    template <typename T>
    void Add(std::string &&name, const T &default_value = T(), void (*validate)(T &value) = nullptr)
    {
        auto ptr = new Parameter<T>(default_value, validate);
        params.insert({std::move(name), static_cast<ParameterBase *>(ptr)});
    }

    std::unordered_map<std::string, ParameterBase *> params;

private:
    template <typename>
    friend struct Parameter;
};

// Explicit templates for params that dont have a serialize implementation for
// filestorage already

// Providing the name indicates a nested structure
template <>
void Parameter<ParamNode>::Serialize(cv::FileStorage &fs, const std::string &name) const
{
    if (name.empty()) fs.startWriteStruct(name, cv::FileNode::MAP);
    for (const auto &p : value.params)
        p.second->Serialize(fs, p.first);
    if (name.empty()) fs.endWriteStruct();
}
template <>
void Parameter<ParamNode>::Deserialize(const cv::FileNode &fn)
{
    for (auto &p : value.params)
        p.second->Deserialize(fn[p.first]);
}

template <>
void Parameter<wxString>::Serialize(cv::FileStorage &fs, const std::string &name) const
{

}
template <>
void Parameter<wxString>::Deserialize(const cv::FileNode &fn)
{

}
template <>
void Parameter<cv::aruco::DetectorParameters>::Serialize(cv::FileStorage &fs, const std::string &name) const
{

}
template <>
void Parameter<cv::aruco::DetectorParameters>::Deserialize(const cv::FileNode &fn)
{

}

class ParamStorage : public ParamNode
{
public:
    ParamStorage(const std::string &file_path)
        : file_path(file_path){};

    void Save();
    void Load();

private:
    const std::string file_path;
};

class UserParamsStorage : public ParamStorage
{
public:
    UserParamsStorage();
};

class LocaleStore : public ParamStorage
{
public:
    LocaleStore();
};

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
