///////////////////////////////////////////////////////////////////////////////
// Name:        convertmattowxbmp.h
// Source:      https://github.com/PBfordev/wxopencvtest
// Purpose:     Converts OpenCV bitmap (Mat) stored as BGR CVU8 to wxBitmap
// Author:      PB
// Created:     2020-09-16
// Copyright:   (c) 2020 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "utils/Assert.hpp"

#include <opencv2/core/mat.hpp>
#include <wx/rawbmp.h>
#include <wx/wx.h>

/**
    @param matBitmap
        Its data must be encoded as BGR CV_8UC3, which is
        probably the most common format for OpenCV images.
    @param bitmap
        It must be initialized to the same width and height as matBitmap
        and its depth must be 24.
    @return @true if the conversion succeeded, @false otherwise.
    On MS Windows, a MSW-optimized version is used if possible,
    the portable one otherwise. In my testing on MSW with
    3840x2160 image in the Release build, the optimized version
    was about 25% faster then the portable one. MSW-optimized version
    is used when bitmap is a DIB and its width modulo 4 is 0.
    In my testing on MSW with MSVS using 3840x2160 image, the portable
    version of conversion function in the Debug build was more then
    60 times slower than in the Release build.
    At least on MSW, initializing a wxBitmap takes quite some
    time. If you are processing images of the same size in a loop
    (e.g., frames of a video file), I recommend initializing the
    wxBitmap outside the loop and reusing it in the loop instead
    of creating it every time inside the loop.
*/
bool ConvertMatToBitmap(const cv::Mat& matBitmap, wxBitmap& bitmap);

/// allocate the bitmap if it isn't the correct size and depth
inline void ReserveBitmapForMat(const cv::Mat& matBitmap, wxBitmap& bitmap)
{
    ATT_ASSERT(matBitmap.depth() == CV_8U);
    if (bitmap.IsOk() &&
        bitmap.GetWidth() == matBitmap.cols &&
        bitmap.GetHeight() == matBitmap.rows &&
        bitmap.GetDepth() == matBitmap.channels() * 8) return;
    bitmap.Create(matBitmap.cols, matBitmap.rows, matBitmap.channels() * 8);
}
