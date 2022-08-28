#pragma once

#include "RefPtr.hpp"
#include "serial/ReaderWriter.hpp"

#include <memory>
#include <vector>

namespace cfg
{

/// use instead of a vector when it will be individually referenced,
/// rather than written in bulk and saved.
template <typename T>
class List
{
public:
    explicit List(int count) { Resize(count); }

    void Resize(int newSize)
    {
        list.resize(newSize);
        for (auto& ptr : list)
        {
            if (!ptr) ptr.reset(new T());
        }
    }

    RefPtr<T> operator[](int idx) const { return list[idx]; }
    int Size() const noexcept { return static_cast<int>(list.size()); }

    void ReadSelf(serial::Reader& reader) { serial::Read(reader, list); }
    void WriteSelf(serial::Writer& writer) const { serial::Write(writer, list); }

private:
    std::vector<std::unique_ptr<T>> list{};
};

} // namespace cfg
