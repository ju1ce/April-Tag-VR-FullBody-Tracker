#pragma once

#include "Serial.hpp"
#include "utils/Assert.hpp"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/core/types.hpp>

#include <filesystem>
#include <iterator>
#include <ranges>

template <>
struct std::iterator_traits<cv::FileNodeIterator>
{
    using difference_type = std::ptrdiff_t;
    using value_type = cv::FileNode;
    using element_type = cv::FileNode;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
};

namespace serial
{

namespace fsys = std::filesystem;

namespace detail
{
// clang-format off
template <typename T> inline constexpr bool IsFileStorable = false;
template <> inline constexpr bool IsFileStorable<bool> = true;
template <> inline constexpr bool IsFileStorable<int> = true;
template <> inline constexpr bool IsFileStorable<float> = true;
template <> inline constexpr bool IsFileStorable<double> = true;
template <> inline constexpr bool IsFileStorable<std::string> = true;
template <> inline constexpr bool IsFileStorable<cv::Mat> = true;
template <typename T, int N> inline constexpr bool IsFileStorable<cv::Vec<T, N>> = true;
template <typename T, int M, int N> inline constexpr bool IsFileStorable<cv::Matx<T, M, N>> = true;
template <typename T> inline constexpr bool IsFileStorable<cv::Point_<T>> = true;
template <typename T> inline constexpr bool IsFileStorable<cv::Point3_<T>> = true;
template <typename T> inline constexpr bool IsFileStorable<cv::Size_<T>> = true;
// clang-format on
} // namespace detail

template <typename T>
concept CVFileStorable = detail::IsFileStorable<T>;

class FileStorage
{
public:
    enum class Mode
    {
        Write,
        Read
    };

    ~FileStorage()
    {
        if (storage.isOpened()) storage.release();
    }

    bool Open(const fsys::path& filePath, Mode mode);

    cv::FileStorage& GetForWriting() { return storage; }
    cv::FileNode GetForReading() const { return storage.root(); }

private:
    cv::FileStorage storage{};
};

class FileNodeIteratorView : public std::ranges::view_interface<FileNodeIteratorView>
{
public:
    FileNodeIteratorView() = default;
    explicit FileNodeIteratorView(const cv::FileNodeIterator& node)
        : mIter(node, node.remaining()) {}
    // NOLINTBEGIN(readability-identifier-naming): std naming
    auto begin() const { return mIter; }
    static auto end() { return std::default_sentinel; }
    auto size() const { return mIter.count(); }
    // NOLINTEND(readability-identifier-naming)

private:
    std::counted_iterator<cv::FileNodeIterator> mIter{};
};

class FileStorageWriter
{
public:
    explicit FileStorageWriter(cv::FileStorage& writer) : mWriter(writer) {}

    void Write(const CVFileStorable auto& value)
    {
        mWriter << value;
    }
    template <Formattable<FileStorageWriter> T>
    void Write(const T& value)
    {
        Serial<T>{}.Format(*this, value);
    }
    void Write(std::string_view value)
    {
        mWriter << std::string(value);
    }
    void Write(const auto&)
    {
        utils::Unreachable();
    }
    template <FormattableKey<FileStorageWriter> T>
    void WriteKey(std::string_view key, const T& value)
    {
        Serial<T>{}.FormatKey(*this, key, value);
    }
    void WriteKey(std::string_view key, const auto& value)
    {
        mWriter << std::string(key);
        Write(value);
    }

    void BeginMap(const Nest nest = Nest::Expand)
    {
        if (nest == Nest::Expand)
        {
            mWriter << "{";
        }
        else
        {
            mWriter << "{:";
        }
    }

    void EndMap()
    {
        mWriter << "}";
    }

    void BeginSeq(const Nest nest = Nest::Expand)
    {
        if (nest == Nest::Expand)
        {
            mWriter << "[";
        }
        else
        {
            mWriter << "[:";
        }
    }

    void EndSeq()
    {
        mWriter << "]";
    }

    void WriteComment(std::string_view text)
    {
        mWriter.writeComment(std::string(text));
    }

private:
    cv::FileStorage& mWriter;
};
static_assert(WritableContext<FileStorageWriter, int>);

class FileStorageReader
{
public:
    explicit FileStorageReader(const cv::FileNode& reader) : mReader(reader) {}

    void Read(CVFileStorable auto& outValue)
    {
        mReader >> outValue;
    }
    template <Parsable<FileStorageReader> T>
    void Read(T& outValue)
    {
        Serial<T>{}.Parse(*this, outValue);
    }
    void Read(auto&)
    {
        utils::Unreachable();
    }
    template <ParsableKey<FileStorageReader> T>
    bool ReadKey(std::string_view key, T& outValue)
    {
        return Serial<T>{}.ParseKey(*this, key, outValue);
    }
    bool ReadKey(std::string_view key, auto& outValue)
    {
        const cv::FileNode elem = mReader[std::string(key)];
        if (elem.empty()) return false;
        FileStorageReader(elem).Read(outValue);
        return true;
    }

    SizedRangeOf<FileStorageReader> auto ReadSeq()
    {
        return FileNodeIteratorView(mReader.begin()) |
               std::views::transform([](const cv::FileNode& node)
                                     { return FileStorageReader(node); });
    }

    cv::FileNode GetReader() const { return mReader; }

private:
    cv::FileNode mReader;
};
static_assert(ReadableContext<FileStorageReader, int>);

} // namespace serial
