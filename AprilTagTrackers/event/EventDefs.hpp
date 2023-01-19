#pragma once

#include "EventQueue.hpp"

namespace evt
{

struct TKGroup
{};
template <typename TDerived>
using TKEvent = EventBase<TKGroup, TDerived>;
using TKEventQueue = EventQueue<TKGroup>;
using TKEventQueueView = EventQueueView<TKGroup>;

struct UIGroup
{};
template <typename TDerived>
using UIEvent = EventBase<UIGroup, TDerived>;
using UIEventQueue = EventQueue<UIGroup>;
using UIEventQueueView = EventQueueView<UIGroup>;

struct VStreamStart : TKEvent<VStreamStart>
{
    constexpr explicit VStreamStart(int index) : index(index) {}
    int index;
};

struct VStreamStop : TKEvent<VStreamStop>
{
    constexpr explicit VStreamStop(int index) : index(index) {}
    int index;
};

struct VStreamError : UIEvent<VStreamError>
{
    constexpr explicit VStreamError(int index) : index(index) {}
    int index;
};

} // namespace evt
