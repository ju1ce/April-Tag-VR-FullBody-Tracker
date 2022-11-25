#pragma once

#include "config/TrackerUnit.hpp"
#include "Helpers.hpp"
#include "math/CVHelpers.hpp"
#include "math/CVTypes.hpp"
#include "utils/Error.hpp"
#include "utils/Types.hpp"

#include <vector>

namespace tracker
{

class TrackerUnit
{
    using MarkersList = std::vector<MarkerCorners3f>;
    using IdsList = std::vector<int>;

    /// move corners to the center of all markers in the list
    static void RecenterCornersList(MarkersList& markers)
    {
        cv::Point3f trackerCenter{};
        for (const auto& corners : markers)
        {
            for (const auto& corner : corners)
            {
                trackerCenter += corner;
            }
        }
        constexpr std::size_t numOfCorners = 4;
        trackerCenter /= static_cast<float>(markers.size() * numOfCorners);
        for (auto& corners : markers)
        {
            for (auto& corner : corners)
            {
                corner -= trackerCenter;
            }
        }
    }

    static void EnsureCorners(const MarkerCorners3f& corners)
    {
        if (corners.size() != 4) throw utils::MakeError("expected 4 corners, got ", corners.size());
    }
    static void EnsureMarkers(const IdsList& ids, const MarkersList& cornersList)
    {
        if (ids.size() != cornersList.size()) throw utils::MakeError("ids size ", ids.size(), " != cornersList size", cornersList.size());
        for (const auto& corners : cornersList)
        {
            EnsureCorners(corners);
        }
    }

public:
    static MarkerCorners3f CreateModelMarker(double markerSizeM)
    {
        const auto halfSize = static_cast<float>(markerSizeM / 2.0);
        return {
            cv::Point3f(-halfSize, halfSize, 0),   // top right
            cv::Point3f(halfSize, halfSize, 0),    // top left
            cv::Point3f(halfSize, -halfSize, 0),   // bottom left
            cv::Point3f(-halfSize, -halfSize, 0)}; // bottom right
    }

    void RecenterMarkers()
    {
        RecenterCornersList(mArucoBoard->objPoints);
    }

    void SetMarkers(IdsList ids, MarkersList cornersList)
    {
        EnsureMarkers(ids, cornersList);
        mArucoBoard->ids = std::move(ids);
        mArucoBoard->objPoints = std::move(cornersList);
    }
    void AddMarker(int id, MarkerCorners3f corners)
    {
        EnsureCorners(corners);
        mArucoBoard->ids.push_back(id);
        mArucoBoard->objPoints.push_back(std::move(corners));
    }

    void SetRole(cfg::TrackerRole role) { mRole = role; }

    bool HasMarkerId(int id) const { return std::find(GetIds().begin(), GetIds().end(), id) != GetIds().end(); }
    Index GetMarkerCount() { return mArucoBoard->ids.size(); }
    /// best tell for calibration is whether the corner offsets of markers have been set
    /// so if they are all zero this could return a false positive
    bool IsCalibrated() const { return !mArucoBoard->objPoints.empty(); }
    /// more accurate and expensive check
    bool EnsureIsCalibrated() const
    {
        // if (!IsCalibrated()) return false;
        // TODO: check ids in range, and marker corners have some meaning, maybe as simple as not zero
        utils::Unreachable();
    }

    void SetWasVisibleLastFrame(bool isFound) { mIsFound = isFound; }
    void SetWasVisibleToDriverLastFrame(bool isFound) { mIsDriverFound = isFound; }
    bool WasVisibleLastFrame() const { return mIsFound; }
    bool WasVisibleToDriverLastFrame() const { return mIsDriverFound; }

    void SetMaskCenter(const cv::Point2d& center) { mMaskCenter = center; }
    const cv::Point2d& GetMaskCenter() const { return mMaskCenter; }

    void SetEstimatedPose(const RodrPose& pose) { mPose = pose; }
    const RodrPose& GetEstimatedPose() const { return mPose; }
    void SetPoseFromDriver(const RodrPose& pose) { mDriverPose = pose; }
    const RodrPose& GetPoseFromDriver() const { return mDriverPose; }

    const ArucoBoardSharedPtr& GetArucoBoard() const { return mArucoBoard; }
    const MarkersList& GetMarkers() const { return mArucoBoard->objPoints; }
    const IdsList& GetIds() const { return mArucoBoard->ids; }

private:
    /// stores ids and corners of markers
    ArucoBoardSharedPtr mArucoBoard = cv::aruco::Board::create(
        MarkersList{},
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50),
        IdsList{});

    RodrPose mPose{};
    cv::Point2d mMaskCenter{};
    bool mIsFound = false;

    RodrPose mDriverPose{};
    bool mIsDriverFound = false;

    cfg::TrackerRole mRole = cfg::TrackerRole::Disabled;
};

} // namespace tracker
