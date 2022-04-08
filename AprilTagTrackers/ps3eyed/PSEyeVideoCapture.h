#ifndef PSEYE_VIDEO_CAPTURE_H
#define PSEYE_VIDEO_CAPTURE_H

#include <opencv2/videoio.hpp>
#

/// Video capture class that prioritizes PS3 Eye devices.
/**
Device opening priority:
-CL Eye MultiCam (Win x86 only; user must have activated camera OR CL Eye Platform SDK Developer binaries)
-CL Eye Driver (Win x86 only)
-PS3EYEDriver (Win and OSX)
-OpenCV native
To prioritize custom PS3 Eye devices, we must override open().
For CL Eye MultiCam and PS3EYEDriver, we set the parent 
Ptr< cv::IVideoCapture >icap member variable
to a custom cv::IVideoCapture object.
For CL Eye Driver, we use the native open to get the native icap member variable,
but we use custom cv::VideoCapture::set() and \ref get() to change parameters via registry settings.
 
 See base class here:
 https://github.com/Itseez/opencv/blob/ddf82d0b154873510802ef75c53e628cd7b2cb13/modules/videoio/include/opencv2/videoio.hpp#L387
 https://github.com/Itseez/opencv/blob/ddf82d0b154873510802ef75c53e628cd7b2cb13/modules/videoio/src/cap.cpp#L550
*/
class PSEyeVideoCapture : public cv::VideoCapture {
public:

    /**
    \param camindex The index of the camera (0-based). To specify an API
    pass in the index + API. e.g., PSEyeVideoCapture(0 + PSEYE_CAP_CLMULTI)
    */
    PSEyeVideoCapture(int camindex)
        : m_index(-1) {open(camindex);} // Constructor is same as base class
    
    /// Attempts to open a device using different drivers in the order outlined above.
    /*
    If a PS3 Eye device is not found, fall back to base class cv::VideoCapture::open(index)
    */
    bool open(int index, int apiPreference = 0) override;

    /// Use cv::VideoCapture::set() unless \ref eyeType == PSEYE_CLEYE_DRIVER
    bool set(int propId, double value) override;

    /// Use cv::VideoCapture::get() unless \ref eyeType == PSEYE_CLEYE_DRIVER
    double get(int propId) const override;

    /// Get the unique identifier for the camera
    std::string getUniqueIndentifier() const;
    
protected:
    int m_index; /**< Keep track of index. Necessary for PSEYE_CLEYE_DRIVER */
    std::string m_indentifier; /**< Filled in when the tracker is opened */

private:
    /// Get the camera capture. If successful, we will have a functional cv::Ptr<CvCapture> \ref cap member variable.
    cv::Ptr<cv::IVideoCapture> pseyeVideoCapture_create(int index);
};

#endif //PSEYE_VIDEO_CAPTURE_H