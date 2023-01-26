#pragma once

#include "EventDefs.hpp"
#include "tracker/VideoCapture.hpp"

#include <opencv2/core/mat.hpp>

namespace evt
{

// clang-format off
struct VideoStream : TKEvent<VideoStream> { int idx; EnableDisable act; };
struct VideoStreamCalibEnable : TKEvent<VideoStreamCalibEnable> { int idx; };
struct VideoStreamCalibDisable : TKEvent<VideoStreamCalibEnable> { int idx; SubmitCancel act; };

// UI: user action -> UI: send VideoStreamRequestPreview -> TK: send VideoStreamPreview
struct VideoStreamRequestPreview : TKEvent<VideoStreamRequestPreview> { int idx; };
struct VideoStreamPreview : UIEvent<VideoStreamPreview>
{
    int idx;
    tracker::SwapChannel<tracker::CapturedFrame>::Consumer channel;
};
// clang-format on2

} // namespace evt
