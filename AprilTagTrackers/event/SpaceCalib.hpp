#pragma once

#include "config/ManualCalib.hpp"
#include "EventDefs.hpp"

namespace evt
{

// clang-format off
struct SpaceCalib : TKEvent<SpaceCalib> { EnableDisable act; };
struct SpaceCalibLockHeight : TKEvent<SpaceCalibLockHeight> { EnableDisable act; };

// user could be editing the gui values, or using the controller and changing from tracking
// both are fighting to save to calib, and update in realtime
// cant use events because they could become unsynced, but only if the user edits both at the same,
// very unlikely that would happen.

// more of an architectural issue...
// tracking thread shouldn't be handling the VRClient

//

struct SpaceCalibUpdate : TKEvent<SpaceCalibUpdate>
{
    cfg::ManualCalib::Real calib;
};
// clang-format on

} // namespace evt
