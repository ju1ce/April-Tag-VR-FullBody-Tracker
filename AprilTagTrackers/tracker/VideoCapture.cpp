#include "VideoCapture.hpp"

#include "utils/Assert.hpp"
#include "utils/Log.hpp"
#include "utils/Test.hpp"

#include <ps3eye/PSEyeVideoCapture.h>

#include <charconv>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <bitset>

namespace
{

std::optional<int> AddressToIndex(std::string_view address) noexcept
{
    constexpr int base = 10;
    int outIndex = 0;
    const char* const endPtr = address.data() + address.length();
    std::from_chars_result result = std::from_chars(address.data(), endPtr, outIndex, base);
    if (result.ec != std::errc()) return std::nullopt;
    if (result.ptr != endPtr) return std::nullopt; // require entire string to be parsed
    if (outIndex < 0) return std::nullopt;
    return outIndex;
}
TEST_CASE("AddressToIndex")
{
    CHECK(AddressToIndex("0") == 0);
    CHECK(AddressToIndex("1") == 1);
    CHECK(AddressToIndex("015") == 15);
    CHECK_NOT(AddressToIndex(""));
    CHECK_NOT(AddressToIndex("-1"));
    CHECK_NOT(AddressToIndex("1two"));
}

// On Linux cv::VideoCapture does not work when GStreamer backend is used and
// camera is set to MJPG pixel format. As a work around we manually setup the
// GStreamer pipeline with suitable decoding before feeding the stream into
// application.
[[maybe_unused]] std::string CreateGStreamerPipeline(int index, cv::Size2i resolution, int fps)
{
    std::stringstream ss;
    ss << "v4l2src device=/dev/video" << index << " ! image/jpeg";
    if (resolution.width > 0) ss << ",width=" << resolution.width;
    if (resolution.height > 0) ss << ",height=" << resolution.height;
    if (fps > 0) ss << ",framerate=" << fps << "/1";
    ss << " ! jpegdec ! video/x-raw,format=I420 ! videoconvert ! appsink";
    return ss.str();
}
TEST_CASE("CreateGStreamerPipeline")
{
    CHECK(CreateGStreamerPipeline(3, {15, 0}, 30) ==
          "v4l2src device=/dev/video3 ! image/jpeg,width=15,framerate=30/1 ! jpegdec ! video/x-raw,format=I420 ! videoconvert ! appsink");
}

} // namespace

namespace tracker
{

bool VideoCapture::TryOpen()
{
    try
    {
        if (!TryOpenCapture()) return false;
        SetCaptureOptions();
        LogCaptureOptions();
        return true;
    }
    catch (const std::exception& e)
    {
        Close();
        ATT_LOG_ERROR(e.what());
        return false;
    }
}

void VideoCapture::Close()
{
    mCapture.reset();
}

bool VideoCapture::TryReadFrame(cv::Mat& outImage)
{
    return mCapture && mCapture->read(outImage) && !outImage.empty();
}

bool VideoCapture::TryOpenCapture()
{
    const int api = mCameraInfo->api;
    const std::optional<int> hwIndex = AddressToIndex(mCameraInfo->address);
    if (api == CAP_PS3EYE)
    {
        if (!hwIndex) return false;
        mCapture = std::make_unique<PSEyeVideoCapture>(*hwIndex);
        return mCapture->isOpened();
    }

    mCapture = std::make_unique<cv::VideoCapture>();
    if (hwIndex)
    {
        if (api == cv::CAP_ANY || api == cv::CAP_GSTREAMER)
        {
            if (TryOpenGStreamerCapture(*hwIndex)) return true;
        }
        return mCapture->open(*hwIndex, api);
    }
    return mCapture->open(mCameraInfo->address, api);
}

bool VideoCapture::TryOpenGStreamerCapture(int index) // NOLINT: linux only
{
#if ATT_OS_LINUX
    ATT_REQUIRE(mCapture);
    std::string pipeline = CreateGStreamerPipeline(index, mCameraInfo->resolution, mCameraInfo->fps);
    return mCapture->open(pipeline, cv::CAP_GSTREAMER);
#else
    return false;
#endif
}

void VideoCapture::SetCaptureOptions()
{
    ATT_REQUIRE(mCapture);
    // On Linux and when GStreamer backend is used we already setup the camera pixel format,
    // width, height and FPS above when the GStreamer pipeline was created.
#ifdef ATT_OS_LINUX
    if ((mCameraInfo->api != cv::CAP_ANY) && (mCameraInfo->api != cv::CAP_GSTREAMER))
#endif
    {
        if (mCameraInfo->resolution.width != 0) mCapture->set(cv::CAP_PROP_FRAME_WIDTH, mCameraInfo->resolution.width);
        if (mCameraInfo->resolution.height != 0) mCapture->set(cv::CAP_PROP_FRAME_HEIGHT, mCameraInfo->resolution.height);
        mCapture->set(cv::CAP_PROP_FPS, mCameraInfo->fps);
    }
    if (mCameraInfo->openDirectShowSettings) mCapture->set(cv::CAP_PROP_SETTINGS, 1);
    if (mCameraInfo->extraSettings.enabled)
    {
        mCapture->set(cv::CAP_PROP_AUTOFOCUS, 0);
        mCapture->set(cv::CAP_PROP_AUTO_EXPOSURE, mCameraInfo->extraSettings.autoExposure);
        mCapture->set(cv::CAP_PROP_EXPOSURE, mCameraInfo->extraSettings.exposure);
        mCapture->set(cv::CAP_PROP_GAIN, mCameraInfo->extraSettings.gain);
    }
    // same as cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    constexpr double codec = 0x47504A4D; // code by FPaul. Should use MJPEG codec to enable fast framerates.
    mCapture->set(cv::CAP_PROP_FOURCC, codec);
}

void VideoCapture::LogCaptureOptions()
{
    ATT_REQUIRE(mCapture);
    ATT_LOG_INFO("OpenCV VideoCapture opened with options:", '\n',
                 "address = ", mCameraInfo->address, '\n',
                 "api = ", mCapture->getBackendName(), '\n',
                 "resolution = ", static_cast<int>(mCapture->get(cv::CAP_PROP_FRAME_WIDTH)), 'x',
                 static_cast<int>(mCapture->get(cv::CAP_PROP_FRAME_HEIGHT)), '\n',
                 "fps = ", static_cast<int>(mCapture->get(cv::CAP_PROP_FPS)));
}

} // namespace tracker
