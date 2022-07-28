///////////////////////////////////////////////////////////////////////////////
// Name:        convertmattowxbmp.cpp
// Source:      https://github.com/PBfordev/wxopencvtest
// Purpose:     Converts OpenCV bitmap (Mat) stored as BGR CVU8 to wxBitmap
// Author:      PB
// Created:     2020-09-16
// Copyright:   (c) 2020 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "MatToBitmap.hpp"

#ifdef __WXMSW__

namespace
{

constexpr int BITMAP_DEPTH = 24;

// Version optimized for Microsoft Windows.
// matBitmap must be continuous and matBitmap.cols % 4 must equal 0
// as SetDIBits() requires the DIB rows to be DWORD-aligned.
// Should not be called directly but only from ConvertMatToBitmap()
// which does all the necessary debug checks.
bool ConvertMatToBitmapMSW(const cv::Mat& matBitmap, wxBitmap& bitmap)
{
    HDC hScreenDC = ::GetDC(nullptr);
    BITMAPINFO bitmapInfo{0};

    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO) - sizeof(RGBQUAD);
    bitmapInfo.bmiHeader.biWidth = bitmap.GetWidth();
    bitmapInfo.bmiHeader.biHeight = 0 - bitmap.GetHeight();
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = BITMAP_DEPTH;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    bool success = ::SetDIBits(hScreenDC, bitmap.GetHBITMAP(), 0, bitmap.GetHeight(),
                       matBitmap.data, &bitmapInfo, DIB_RGB_COLORS) != 0;
    ::ReleaseDC(nullptr, hScreenDC);

    return success;
}

} // unnamed namespace

#endif // #ifndef __WXMSW__

bool ConvertMatToBitmap(const cv::Mat& matBitmap, wxBitmap& bitmap)
{
    ATT_ASSERT(!matBitmap.empty());
    ATT_ASSERT(matBitmap.type() == CV_8UC3);
    ATT_ASSERT(matBitmap.dims == 2);
    ATT_ASSERT(matBitmap.isContinuous());
    ATT_ASSERT(bitmap.IsOk());
    ATT_ASSERT(bitmap.GetWidth() == matBitmap.cols && bitmap.GetHeight() == matBitmap.rows);
    ATT_ASSERT(bitmap.GetDepth() == BITMAP_DEPTH);

#ifdef __WXMSW__
    if (bitmap.IsDIB() && matBitmap.cols % 4 == 0)
    {
        return ConvertMatToBitmapMSW(matBitmap, bitmap);
    }
#endif

    wxNativePixelData pixelData(bitmap);
    wxNativePixelData::Iterator pixelDataIt(pixelData);

    const uchar* bgr = matBitmap.ptr();

    for (int row = 0; row < pixelData.GetHeight(); ++row)
    {
        pixelDataIt.MoveTo(pixelData, 0, row);
        for (int col = 0; col < pixelData.GetWidth(); ++col)
        {
            pixelDataIt.Blue() = *(bgr++);
            pixelDataIt.Green() = *(bgr++);
            pixelDataIt.Red() = *(bgr++);

            ++pixelDataIt;
        }
    }

    return bitmap.IsOk();
}
