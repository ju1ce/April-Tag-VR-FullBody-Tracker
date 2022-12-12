#include "VRDriver.hpp"

#include "SemVer.h"
#include "utils/Env.hpp"
#include "utils/Error.hpp"
#include "utils/Test.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

inline std::ostream& operator<<(std::ostream& os, const Pose& pose)
{
    return os << pose.position.x << ' '
              << pose.position.y << ' '
              << pose.position.z << ' '
              << pose.rotation.w << ' '
              << pose.rotation.x << ' '
              << pose.rotation.y << ' '
              << pose.rotation.z;
}

inline std::istream& operator>>(std::istream& is, Pose& pose)
{
    return is >> pose.position.x >> pose.position.y >> pose.position.z >>
           pose.rotation.w >> pose.rotation.x >> pose.rotation.y >> pose.rotation.z;
}

namespace
{

template <typename... TArgs>
std::string BuildCommand(std::string_view name, const TArgs&... args)
{
    std::ostringstream ss;
    ss << ' ' << name
       << std::fixed << std::setprecision(6);
    ((ss << ' ' << args), ...);
    return ss.str();
}
TEST_CASE("BuildCommand")
{
    CHECK(BuildCommand("updatepose") == " updatepose");
    CHECK(BuildCommand("foo", 10) == " foo 10");
    CHECK(BuildCommand("foo", 1.1) == " foo 1.100000");
    CHECK(BuildCommand("foo", 1.1F) == " foo 1.100000");
    CHECK(BuildCommand("foo", 10.1234559) == " foo 10.123456");
    CHECK(BuildCommand("foo", "bar", "baz", 10) == " foo bar baz 10");
    CHECK(BuildCommand("foo", Pose(cv::Point3d(0.1, 0.2, 0.3), cv::Quatd(1, 0, 0, 0))) ==
          " foo 0.100000 0.200000 0.300000 1.000000 0.000000 0.000000 0.000000");
}

template <typename... TArgs>
void VerifyAndParseResponse(std::string_view buffer, std::string_view expectName, TArgs&... outArgs)
{
    std::istringstream ss{std::string(buffer)};
    std::string name;
    ss >> name;
    if (name.empty()) throw utils::MakeError("no response from driver");
    if (name != expectName) throw utils::MakeError("command response indicated failure: ", buffer);
    ((ss >> outArgs), ...);
}
TEST_CASE("VerifyAndParseResponse")
{
    DOCTEST_CHECK_NOTHROW(VerifyAndParseResponse(" updated", "updated"));
    DOCTEST_CHECK_THROWS(VerifyAndParseResponse(" updated", "idinvalid"));
    int outId = -1;
    DOCTEST_CHECK_NOTHROW(VerifyAndParseResponse(" foo 12", "foo", outId));
    CHECK(outId == 12);
}

} // namespace

namespace tracker
{

VRDriver::VRDriver(const cfg::List<cfg::TrackerUnit>& trackers)
    : mBridge(IPC::CreateDriverClient())
{
    /// also checks driver version, ensures can connect
    CmdGetTrackerCount();

    int index = 0;
    for (const auto& tracker : trackers.AsRange())
    {
        AddTracker(index++, tracker.role);
    }

    if (trackers.GetSize() != CmdGetTrackerCount()) throw std::runtime_error("some or all trackers were not registered with driver");
    CmdAddStation();
}

void VRDriver::UpdateTracker(int id, Pose pose, double frameTime, double smoothing)
{
    const std::string cmd = BuildCommand("updatepose", id, pose, frameTime, smoothing);
    const std::string_view res = mBridge->SendRecv(cmd);
    VerifyAndParseResponse(res, "updated");
}

void VRDriver::CmdUpdateStation(int id, Pose pose)
{
    const std::string cmd = BuildCommand("updatestation", id, pose);
    const std::string_view res = mBridge->SendRecv(cmd);
    VerifyAndParseResponse(res, "updated");
}

void VRDriver::SetSmoothing(double factor, double additional)
{
    constexpr int saved = 120;
    const std::string cmd = BuildCommand("settings", saved, factor, additional);
    const std::string_view res = mBridge->SendRecv(cmd);
    VerifyAndParseResponse(res, "changed");
}

int VRDriver::CmdGetTrackerCount()
{
    const std::string_view res = mBridge->SendRecv(BuildCommand("numtrackers"));
    int count = -1;
    std::string versionStr;
    VerifyAndParseResponse(res, "numtrackers", count, versionStr);
    if (count < 0) throw std::runtime_error("invalid tracker count: " + std::to_string(count));

    const SemVer expected = utils::GetBridgeDriverVersion();
    if (!SemVer::Compatible(SemVer::Parse(versionStr), expected))
    {
        throw std::runtime_error("incompatible bridge driver: " + versionStr + " expected: " + expected.ToString());
    }

    return count;
}

void VRDriver::CmdAddTracker(std::string_view name, std::string_view role)
{
    const std::string cmd = BuildCommand("addtracker", name, role);
    const std::string_view res = mBridge->SendRecv(cmd);
    VerifyAndParseResponse(res, "added");
}

void VRDriver::CmdAddStation()
{
    const std::string_view res = mBridge->SendRecv(BuildCommand("addstation"));
    VerifyAndParseResponse(res, "added");
}

VRDriver::GetTrackerResult VRDriver::GetTracker(int id, double timeOffset)
{
    const std::string cmd = BuildCommand("gettrackerpose", id, timeOffset);
    const std::string_view res = mBridge->SendRecv(cmd);
    int outId = -1;
    Pose outPose = Pose::Ident();
    int outStatus = -1;
    VerifyAndParseResponse(res, "trackerpose", outId, outPose, outStatus);
    if (id != outId) throw std::runtime_error("unexpected tracker id");
    return GetTrackerResult{outPose, (outStatus == 0)};
}

} // namespace tracker
