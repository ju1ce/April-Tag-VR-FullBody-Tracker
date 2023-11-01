#pragma once

#include "config/List.hpp"
#include "config/TrackerUnit.hpp"
#include "Helpers.hpp"
#include "IPC/IPC.hpp"
#include "utils/Enum.hpp"

#include <memory>
#include <optional>
#include <string_view>

namespace tracker
{

class VRDriver
{
public:
    VRDriver(){};
    explicit VRDriver(const cfg::List<cfg::TrackerUnit>& trackers);

    virtual void UpdateStation(Pose pose) { CmdUpdateStation(0, pose); }

    // pose = x y z qw qx qy qz
    // 'updatepose' id pose time smoothing -> 'updated'
    virtual void UpdateTracker(int id, Pose pose, double frameTime, double smoothing);
    // 'settings' saved factor additional -> 'changed'
    virtual void SetSmoothing(double factor, double additional);

    struct GetTrackerResult
    {
        Pose pose;
        bool isValid = false;
    };

    // 'gettrackerpose' id offset -> 'trackerpose' id pose status
    // status: -1 = invalid, 0 = valid, 1 = late
    virtual GetTrackerResult GetTracker(int id, double timeOffset);

private:
    virtual void AddTracker(int id, cfg::TrackerRole role)
    {
        const std::string name = "ApriltagTracker" + std::to_string(id);
        std::string roleStr = "TrackerRole_";
        roleStr += utils::renum::ToString(role);
        CmdAddTracker(name, roleStr);
    }

    // 'updatestation' id pose -> 'updated'
    virtual void CmdUpdateStation(int id, Pose pose);
    // 'numtrackers' -> 'numtrackers' count version
    virtual int CmdGetTrackerCount();
    // 'addtracker' name role -> 'added'
    virtual void CmdAddTracker(std::string_view name, std::string_view role);
    // 'addstation' -> 'added'
    virtual void CmdAddStation();

    std::unique_ptr<IPC::IClient> mBridge;
};

class DummyDriver : public VRDriver
{
public:
    explicit DummyDriver(const cfg::List<cfg::TrackerUnit>& trackers)
    {}

    void UpdateStation(Pose pose) {}

    // pose = x y z qw qx qy qz
    // 'updatepose' id pose time smoothing -> 'updated'
    void UpdateTracker(int id, Pose pose, double frameTime, double smoothing){};
    // 'settings' saved factor additional -> 'changed'
    void SetSmoothing(double factor, double additional){};

    // 'gettrackerpose' id offset -> 'trackerpose' id pose status
    // status: -1 = invalid, 0 = valid, 1 = late
    GetTrackerResult GetTracker(int id, double timeOffset){ return GetTrackerResult{Pose::Ident(), false}; };

private:
    void AddTracker(int id, cfg::TrackerRole role)
    {}

    // 'updatestation' id pose -> 'updated'
    void CmdUpdateStation(int id, Pose pose){};
    // 'numtrackers' -> 'numtrackers' count version
    int CmdGetTrackerCount() { return 3; };
    // 'addtracker' name role -> 'added'
    void CmdAddTracker(std::string_view name, std::string_view role){};
    // 'addstation' -> 'added'
    void CmdAddStation(){};

    std::unique_ptr<IPC::IClient> mBridge;
};

} // namespace tracker
