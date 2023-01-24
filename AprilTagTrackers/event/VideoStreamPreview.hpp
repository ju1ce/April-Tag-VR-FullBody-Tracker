#pragma once

#include "EventDefs.hpp"
#include "tracker/VideoCapture.hpp"

#include <opencv2/core/mat.hpp>

namespace evt
{

// clang-format off
struct VideoStreamStart : TKEvent<VideoStreamStart> { int idx; };
struct VideoStreamStop : TKEvent<VideoStreamStop> { int idx; };
struct VideoStreamCalibStart : TKEvent<VideoStreamCalibStart> { int idx; };
struct VideoStreamCalibSave : TKEvent<VideoStreamCalibSave> { int idx; };
struct VideoStreamCalibCancel : TKEvent<VideoStreamCalibCancel> { int idx; };

// UI: user action -> UI: send VideoStreamRequestPreview -> TK: send VideoStreamPreview
struct VideoStreamRequestPreview : TKEvent<VideoStreamRequestPreview> { int idx; };
struct VideoStreamPreview : UIEvent<VideoStreamPreview>
{
    int idx;
    tracker::SwapChannel<tracker::CapturedFrame>::Consumer channel;
};
// clang-format on2

} // namespace evt
