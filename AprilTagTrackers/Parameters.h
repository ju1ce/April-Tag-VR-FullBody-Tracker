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

    // ParamNode(const ParamNode &) = delete;
    void operator=(const ParamNode &pn) { assert(false); }

    // T may be able to be deduced via the return type at the callsite
    template <typename T>
    T &Ref(const std::string &name)
    {
#ifdef _DEBUG
        assert(typeid(T).hash_code() == static_cast<Parameter<T> *>(params.at(name))->tinfo);
#endif
        return static_cast<Parameter<T> *>(params.at(name))->value;
    }

    template <typename T>
    const T &Get(const std::string &name) const
    {
#ifdef _DEBUG
        assert(typeid(T).hash_code() == static_cast<Parameter<T> *>(params.at(name))->tinfo);
#endif
        return static_cast<const Parameter<T> *>(params.at(name))->value;
    }

    void Validate(const std::string &name)
    {
        params.at(name)->Validate();
    }

    ParamNode &Node(const std::string &n) { return Ref<ParamNode>(n); }

#define REF_TYPE_ALIAS(name, T) \
    const T &name(const std::string &n) const { return Get<T>(n); }

    REF_TYPE_ALIAS(Int, int)
    REF_TYPE_ALIAS(Float, float)
    REF_TYPE_ALIAS(Bool, bool)
    REF_TYPE_ALIAS(String, std::string)
    REF_TYPE_ALIAS(WString, wxString)
    REF_TYPE_ALIAS(Mat, cv::Mat)

#undef REF_TYPE_ALIAS

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

protected:
    // Add a parameter optionally specifying default and a validation lambda
    template <typename T>
    void Add(std::string &&name, void (*validate)(T &value) = nullptr)
    {
        auto ptr = new Parameter<T>(validate);
        params.insert({std::move(name), static_cast<ParameterBase *>(ptr)});
    }
    template <typename T>
    void Add(std::string &&name, const T &default_value, void (*validate)(T &value) = nullptr)
    {
        auto ptr = new Parameter<T>(default_value, validate);
        params.insert({std::move(name), static_cast<ParameterBase *>(ptr)});
    }

    std::unordered_map<std::string, ParameterBase *> params;
};

// Implement this for each type that isn't serializable.
// It's fine to make it inline/in the header.
// friend cv::FileStorage &operator<<(cv::FileStorage &fs, const Type &v) {}
// friend void operator>>(const cv::FileNode &fn, Type &v) {}

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
