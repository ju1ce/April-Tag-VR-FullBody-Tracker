#pragma once

#include "MessageQueue.hpp"
#include "RefPtr.hpp"
#include "utils/Assert.hpp"
#include "utils/Concepts.hpp"
#include "utils/TypeName.hpp"

#include <new>
#include <string_view>
#include <unordered_map>

// events have a statically generated identifier
// uppercase is Id, e.g. EventId, eventId, GetIdFor
// lowercase is id

namespace evt
{

template <typename>
class EventQueue;

namespace detail
{

inline int COUNTER = 0; // NOLINT
template <typename T>
inline int ID = COUNTER++; // NOLINT

// NOLINTBEGIN(*reinterpret-cast)
/// type erased pointer to event, uses SBO if event size is <= pointer size
class EventPtr
{
    static constexpr int SBO_SIZE = sizeof(void*);

public:
    constexpr EventPtr() noexcept = default;
    constexpr EventPtr(EventPtr&&) noexcept = default;

    EventPtr(const EventPtr&) = delete;
    EventPtr& operator=(const EventPtr&) = delete;

    template <typename TEvent, typename... Args>
    void Emplace(Args&&... args)
    {
#ifdef ATT_DEBUG
        mDebugName = utils::TypeName<TEvent>();
#endif
        mId = TEvent::Id();
        if constexpr (sizeof(TEvent) <= SBO_SIZE)
        {
            ::new (&mData) TEvent(std::forward<Args>(args)...);
        }
        else
        {
            ::new (&mData) TEvent*(new TEvent(std::forward<Args>(args)...));
        }
    }
    template <typename TEvent>
    TEvent& Get()
    {
        ATT_ASSERT(TEvent::Id() == mId);
        if constexpr (sizeof(TEvent) <= SBO_SIZE)
        {
            return *std::launder(reinterpret_cast<TEvent*>(&mData));
        }
        else
        {
            return **std::launder(reinterpret_cast<TEvent**>(&mData));
        }
    }
    template <typename TEvent>
    void Destroy()
    {
        ATT_ASSERT(TEvent::Id() == mId);
        if constexpr (sizeof(TEvent) <= SBO_SIZE)
        {
            std::destroy_at(std::launder(reinterpret_cast<TEvent*>(&mData)));
        }
        else
        {
            delete *std::launder(reinterpret_cast<TEvent**>(&mData));
        }
    }

    int GetId() const { return mId; }

#ifdef ATT_DEBUG
    std::string_view mDebugName;
#endif

private:
    // void* or event in place
    std::aligned_storage_t<sizeof(void*), alignof(void*)> mData{};
    int mId = -1;
};
// NOLINTEND(*reinterpret-cast)

} // namespace detail

template <typename TGroupTag, typename TDerived>
class EventBase
{
public:
    // type checking of EventQueue group tag
    friend class EventQueue<TGroupTag>;
    friend class detail::EventPtr;

private:
    static int Id() { return detail::ID<EventBase>; }
};

namespace detail
{

class HandlerTypeErasure
{
public:
    virtual ~HandlerTypeErasure() noexcept = default;

    /// Do some operation on event, then destroys and frees memory
    virtual void Handle(EventPtr&) const = 0;
    bool operator==(const HandlerTypeErasure& other) const { return &other == this; }
};

template <typename TEvent, typename T>
using MemberFuncPtr = void (T::*)(const TEvent&);

template <typename TEvent, typename T>
class MemberFuncHandler final : public HandlerTypeErasure
{
public:
    using MemberFuncPtr = MemberFuncPtr<TEvent, T>;

    constexpr MemberFuncHandler(T* const instance, const MemberFuncPtr memberFunc)
        : mInstance(instance), mMemberFunc(memberFunc) {}

    void Handle(detail::EventPtr& event) const final
    {
        (mInstance->*mMemberFunc)(event.Get<TEvent>());
        event.Destroy<TEvent>();
    }

private:
    T* const mInstance;
    const MemberFuncPtr mMemberFunc;
};

template <typename TEvent, typename Fn>
class FunctorHandler final : public HandlerTypeErasure
{
public:
    constexpr explicit FunctorHandler(Fn&& func)
        : mFunc(std::move(func)) {}

    void Handle(detail::EventPtr& event) const final
    {
        mFunc(event.Get<TEvent>());
        event.Destroy<TEvent>();
    }

private:
    const Fn mFunc;
};

} // namespace detail

template <typename Event, typename GroupTag>
concept EventInGroup = std::derived_from<Event, EventBase<GroupTag, Event>>;

/// Single consumer, internally-synchronized, event queue.
/// Cannot be statically constructed. IDs are generated.
template <typename TGroupTag>
class EventQueue
{
    using AnyHandlerPtr = std::unique_ptr<const detail::HandlerTypeErasure>;
    // TODO: IDs are sequential, use a vector
    using HandlerMap = std::unordered_map<int, AnyHandlerPtr>;

public:
    template <typename Event>
        requires(EventInGroup<std::remove_reference_t<Event>, TGroupTag>)
    void Push(Event&& event)
    {
        using EventType = std::remove_reference_t<Event>;
        detail::EventPtr e{};
        e.Emplace<EventType>(std::forward<Event>(event));
        mEvents.Enqueue(std::move(e));
    }

    template <EventInGroup<TGroupTag> Event, typename... Args>
    void Emplace(Args&&... args)
    {
        detail::EventPtr e{};
        e.Emplace<Event>(std::forward<Args>(args)...);
        mEvents.Enqueue(std::move(e));
    }

    template <EventInGroup<TGroupTag> TEvent, typename T>
    void Bind(T* const instance, const detail::MemberFuncPtr<TEvent, T> memberFunc)
    {
        mHandlers.try_emplace(
            TEvent::Id(),
            std::make_unique<detail::MemberFuncHandler<TEvent, T>>(instance, memberFunc));
    }

    template <EventInGroup<TGroupTag> TEvent,
              utils::Callable<const TEvent&> Func>
    void Bind(Func&& func)
    {
        mHandlers.try_emplace(
            TEvent::Id(),
            std::make_unique<detail::FunctorHandler<TEvent, Func>>(std::forward<Func>(func)));
    }

    template <EventInGroup<TGroupTag> TEvent>
    void Unbind()
    {
        mHandlers.erase(TEvent::Id());
    }

    void ProcessEvents()
    {
        mEvents.Process([this](detail::EventPtr& event) {
            const HandlerMap::const_iterator iter = mHandlers.find(event.GetId());
            ATT_ASSERT(iter != mHandlers.cend(), "unhandled event: ", event.mDebugName);
            const AnyHandlerPtr& handler = iter->second;
            if (handler) handler->Handle(event);
        });
    }

private:
    MessageQueue<detail::EventPtr> mEvents;
    HandlerMap mHandlers;
};

template <typename TGroupTag>
class EventQueueView
{
public:
    EventQueueView() = default;
    explicit EventQueueView(RefPtr<EventQueue<TGroupTag>> queue) : mQueue(queue) {}

    template <typename TEvent>
    void Push(TEvent&& event)
    {
        mQueue->Push(std::forward<TEvent>(event));
    }

    template <typename TEvent, typename... Args>
    void Emplace(Args&&... args)
    {
        mQueue->template Emplace<TEvent>(std::forward<Args>(args)...);
    }

private:
    RefPtr<EventQueue<TGroupTag>> mQueue;
};

}; // namespace evt
