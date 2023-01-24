#pragma once

#include "EventQueue.hpp"

namespace evt
{

struct TKGroup
{};
/// events processed on the tracking thread
template <typename TDerived>
using TKEvent = EventBase<TKGroup, TDerived>;
using TKEventQueue = EventQueue<TKGroup>;
using TKEventQueueView = EventQueueView<TKGroup>;

struct UIGroup
{};
/// events processed on the ui thread
template <typename TDerived>
using UIEvent = EventBase<UIGroup, TDerived>;
using UIEventQueue = EventQueue<UIGroup>;
using UIEventQueueView = EventQueueView<UIGroup>;

template <typename TEvent>
struct UIErrorOf : UIEvent<UIErrorOf<TEvent>>
{
    explicit UIErrorOf(TEvent event) : event(std::move(event)) {}
    TEvent event;
};

// clang-format off

struct StatusVStreamStarted : UIEvent<StatusVStreamStarted> { int idx; };
struct StatusVStreamStopped : UIEvent<StatusVStreamStarted> { int idx; };
struct StatusTrackingStarted : UIEvent<StatusTrackingStarted> {};
struct StatusTrackingStopped : UIEvent<StatusTrackingStopped> {};
struct StatusVRDriverConnected : UIEvent<StatusVRDriverConnected> {};
struct StatusVRDriverDisconnected : UIEvent<StatusVRDriverDisconnected> {};
struct StatusVRClientConnected : UIEvent<StatusVRClientConnected> {};
struct StatusVRClientDisconnected : UIEvent<StatusVRClientDisconnected> {};

struct UnitCalibStart : TKEvent<UnitCalibStart> {};
struct UnitCalibStop : TKEvent<UnitCalibStop> {};

struct VRDriverStart : TKEvent<VRDriverStart> {};
struct VRDriverStop : TKEvent<VRDriverStop> {};

struct VRClientStart : TKEvent<VRClientStart> {};
struct VRClientStop : TKEvent<VRClientStop> {};

struct TrackingStart : TKEvent<TrackingStart> {};
struct TrackingStop : TKEvent<TrackingStop> {};
// clang-format on

} // namespace evt
