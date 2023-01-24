#pragma once

#include "config/ManualCalib.hpp"
#include "EventDefs.hpp"

namespace evt
{

// clang-format off
struct SpaceCalibStart : TKEvent<SpaceCalibStart> {};
struct SpaceCalibStop : TKEvent<SpaceCalibStop> {};
struct SpaceCalibLockHeight : TKEvent<SpaceCalibLockHeight> {};

struct SpaceCalibUpdate : TKEvent<SpaceCalibUpdate>
{
    cfg::ManualCalib::Real calib;
};
// clang-format on

} // namespace evt
