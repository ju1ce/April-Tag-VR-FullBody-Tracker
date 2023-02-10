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

enum class SubmitCancel : bool { Submit = true, Cancel = false };
enum class EnableDisable : bool { Enable = true, Disable = false };
using enum SubmitCancel;
using enum EnableDisable;

struct StatusVideoStream : UIEvent<StatusVideoStream> { EnableDisable act; int idx; };
struct StatusTracking : UIEvent<StatusTracking> { EnableDisable act; };
struct StatusVRDriver : UIEvent<StatusVRDriver> { EnableDisable act; };
struct StatusVRClient : UIEvent<StatusVRClient> { EnableDisable act; };

struct TrackerUnitCalib : TKEvent<TrackerUnitCalib> { EnableDisable act; };
struct VRClient : TKEvent<VRClient> { EnableDisable act; };
struct VRDriver : TKEvent<VRDriver> { EnableDisable act; };
struct Tracking : TKEvent<Tracking> { EnableDisable act; };
// clang-format on

} // namespace evt
