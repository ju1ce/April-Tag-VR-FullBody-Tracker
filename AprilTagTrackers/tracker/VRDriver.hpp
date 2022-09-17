#pragma once

#include "Helpers.hpp"
#include "IPC/IPC.hpp"

#include <memory>
#include <optional>
#include <string_view>

namespace tracker
{

class VRDriver
{
public:
    explicit VRDriver(std::unique_ptr<IPC::IClient>&& bridge)
        : mBridge(std::move(bridge)) {}

    // pose = x y z qw qx qy qz
    // 'updatepose' id pose time smoothing -> 'updated'
    void UpdateTracker(int id, Pose pose, double frameTime, double smoothing);
    // 'updatestation' id pose -> 'updated'
    void UpdateStation(int id, Pose pose);
    // 'settings' saved factor additional -> 'changed'
    void SetSmoothing(double factor, double additional);
    // 'numtrackers' -> 'numtrackers' count version
    int GetTrackerCount();
    // 'addtracker' name role -> 'added'
    void AddTracker(std::string_view name, std::string_view role);
    // 'addstation' -> 'added'
    void AddStation();
    // 'gettrackerpose' id offset -> 'trackerpose' id pose status
    // status -1 = invalid, 0 = valid, 1 = late
    std::optional<Pose> GetTracker(int id, double timeOffset);

private:
    std::unique_ptr<IPC::IClient> mBridge;
};

} // namespace tracker
