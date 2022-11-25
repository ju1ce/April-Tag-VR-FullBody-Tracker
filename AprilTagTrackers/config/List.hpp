#pragma once

#include "RefPtr.hpp"
#include "serial/Serial.hpp"
#include "utils/Types.hpp"

#include <memory>
#include <ranges>
#include <vector>

namespace cfg
{

/// use instead of a vector when it will be individually referenced,
/// rather than written in bulk and saved.
template <typename T>
class List
{
public:
    friend struct serial::Serial<List<T>>;

    explicit List(int count) { Resize(count); }

    void Resize(Index newSize)
    {
        mList.resize(newSize);
        for (auto& ptr : mList)
        {
            if (!ptr) ptr.reset(new T());
        }
    }

    RefPtr<T> operator[](Index idx) const { return mList.at(idx); }
    RefPtr<T> UnsafeAt(Index idx) const { return mList[idx]; }

    Index GetSize() const noexcept { return mList.size(); }

    constexpr std::ranges::random_access_range auto AsRange() const
    {
        return mList | std::views::transform([](const std::unique_ptr<T>& ptr)
                                             { return *ptr; });
    }

private:
    std::vector<std::unique_ptr<T>> mList{};
};

} // namespace cfg

template <typename T>
struct serial::Serial<cfg::List<T>>
{
    void Parse(auto ctx, cfg::List<T>& outValue)
    {
        ctx.Read(outValue.mList);
    }
    void Format(auto ctx, const cfg::List<T>& value)
    {
        ctx.Write(value.mList);
    }
};
