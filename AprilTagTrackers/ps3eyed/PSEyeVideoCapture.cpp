#define HAVE_PS3EYE

#include "PSEyeVideoCapture.h"
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/videoio/videoio_c.h>
#include "opencv2/imgproc.hpp"
#include <iostream>
#ifdef HAVE_PS3EYE
#include "ps3eye.h"
#endif
#ifdef HAVE_CLEYE
#include "CLEyeMulticam.h"
#include "PlatformDeviceAPIWin32.h"
const uint16_t VENDOR_ID = 0x1415;
const uint16_t PRODUCT_ID = 0x2000;
const char *CLEYE_DRIVER_PROVIDER_NAME = "Code Laboratories, Inc.";
const char *CL_DRIVER_REG_PATH = "Software\\PS3EyeCamera\\Settings";  // [HKCU]
#endif

enum
{
#ifdef HAVE_CLEYE
    PSEYE_CAP_CLMULTI   = 2100,
    PSEYE_CAP_CLEYE     = 2200,
#endif
#ifdef HAVE_PS3EYE
    PSEYE_CAP_PS3EYE    = 2300
#endif
};

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': snprintf
#define snprintf _snprintf
#endif

/**
 cv::IVideoCapture is a convenient abstract base for custom capture devices.
 Unfortunately, it does not have a public interface, so we redefine it here.
 https://github.com/Itseez/opencv/blob/09e6c82190b558e74e2e6a53df09844665443d6d/modules/videoio/src/precomp.hpp#L164-L176
*/
class cv::IVideoCapture
{
public:
    virtual ~IVideoCapture() {}
    virtual double getProperty(int) const { return 0; }
    virtual bool setProperty(int, double) { return false; }
    virtual bool grabFrame() = 0;
    virtual bool retrieveFrame(int, cv::OutputArray) = 0;
    virtual bool isOpened() const = 0;
    virtual int getCaptureDomain() { return CAP_ANY; } // Return the type of the capture object: CAP_VFW, etc...
};

/*
-- Camera-specific implementations of cv::IVideoCapture --
 Examples
 
 HAVE_DSHOW:
 https://github.com/Itseez/opencv/blob/ddf82d0b154873510802ef75c53e628cd7b2cb13/modules/videoio/src/cap_dshow.hpp#L23
 https://github.com/Itseez/opencv/blob/ddf82d0b154873510802ef75c53e628cd7b2cb13/modules/videoio/src/cap_dshow.cpp#L3141
 
 WINRT_VIDEO:
 https://github.com/Itseez/opencv/blob/ddf82d0b154873510802ef75c53e628cd7b2cb13/modules/videoio/src/cap_winrt_capture.hpp#L45
 https://github.com/Itseez/opencv/blob/ddf82d0b154873510802ef75c53e628cd7b2cb13/modules/videoio/src/cap_winrt_capture.cpp#L121
*/

#ifdef HAVE_CLEYE
/// Implementation of cv::IVideoCapture when using CLEyeMulticam.dll
/**
Either uses the DLL that comes with PSMoveService and the user has their camera activated.
Or the user has the CL Eye Platform SDK developer binaries installed and they delete
the DLL that comes with PSMoveService.
*/
class PSEYECaptureCAM_CLMULTI : public cv::IVideoCapture
{
public:
    PSEYECaptureCAM_CLMULTI(int _index)
        : m_index(-1), m_width(-1), m_height(-1),
        m_frame(NULL), m_frame4ch(NULL)
    {
        open(_index);
    }

    ~PSEYECaptureCAM_CLMULTI()
    {
        close();
    }

    double getProperty(int property_id) const
    {
        int _width, _height;
        switch (property_id)
        {
        case CV_CAP_PROP_BRIGHTNESS:
            return (double)(CLEyeGetCameraParameter(m_eye, CLEYE_LENSBRIGHTNESS)); // [-500, 500]
        case CV_CAP_PROP_CONTRAST:
            return false;
        case CV_CAP_PROP_EXPOSURE:
            // [0, 511] -> [0, 255]
            return double(CLEyeGetCameraParameter(m_eye, CLEYE_EXPOSURE))/2.0;
        case CV_CAP_PROP_FPS:
            return (double)(60);
        case CV_CAP_PROP_FRAME_HEIGHT:
            CLEyeCameraGetFrameDimensions(m_eye, _width, _height);
            return (double)(_height);
        case CV_CAP_PROP_FRAME_WIDTH:
            CLEyeCameraGetFrameDimensions(m_eye, _width, _height);
            return (double)(_width);
        case CV_CAP_PROP_GAIN:
            // [0, 79] -> [0, 255]
            return double(CLEyeGetCameraParameter(m_eye, CLEYE_GAIN)) * (256.0/80.0);
        case CV_CAP_PROP_HUE:
            return 0;
        case CV_CAP_PROP_SHARPNESS:
            return 0;
        }
        return 0;
    }

    bool setProperty(int property_id, double value)
    {
        int val;
        if (!m_eye)
        {
            return false;
        }
        switch (property_id)
        {
        case CV_CAP_PROP_BRIGHTNESS:
            // [-500, 500]
            CLEyeSetCameraParameter(m_eye, CLEYE_LENSBRIGHTNESS, (int)value);
        case CV_CAP_PROP_CONTRAST:
            return false;
        case CV_CAP_PROP_EXPOSURE:
            CLEyeSetCameraParameter(m_eye, CLEYE_AUTO_EXPOSURE, value <= 0);
            if (value > 0)
            {
                //[0, 255] -> [0, 511]
                val = (int)(value * 2.0);
                CLEyeSetCameraParameter(m_eye, CLEYE_EXPOSURE, val);
            }
        case CV_CAP_PROP_FPS:
            return false; //TODO: Modifying FPS probably requires resetting the camera
        case CV_CAP_PROP_FRAME_HEIGHT:
            return false; //TODO: Modifying frame size probably requires resetting the camera
        case CV_CAP_PROP_FRAME_WIDTH:
            return false; //TODO: Modifying frame size probably requires resetting the camera
        case CV_CAP_PROP_GAIN:
            CLEyeSetCameraParameter(m_eye, CLEYE_AUTO_GAIN, value <= 0);
            if (value > 0)
            {
                //[0, 255] -> [0, 79]
                val = (int)ceil(value * 80.0 / 256.0);
                CLEyeSetCameraParameter(m_eye, CLEYE_GAIN, val);
            }
        case CV_CAP_PROP_HUE:
            return false;
        case CV_CAP_PROP_SHARPNESS:
            return false; // TODO: Using OpenCV interface, sharpness appears to work
        }
        return true;
    }

    bool grabFrame()
    {
        cvGetRawData(m_frame4ch, &pCapBuffer, 0, 0);
        return true;
    }

    bool retrieveFrame(int channel, cv::OutputArray outArray)
    {
        CLEyeCameraGetFrame(m_eye, pCapBuffer, 33);
        const int from_to[] = { 0, 0, 1, 1, 2, 2 };
        const CvArr** src = (const CvArr**)&m_frame4ch;
        CvArr** dst = (CvArr**)&m_frame;
        cvMixChannels(src, 1, dst, 1, from_to, 3);

        if (m_frame->origin == IPL_ORIGIN_TL)
            cv::cvarrToMat(m_frame).copyTo(outArray);
        else
        {
            cv::Mat temp = cv::cvarrToMat(m_frame);
            flip(temp, outArray, 0);
        }

        return true;
    }

    int getCaptureDomain() { return PSEYE_CAP_CLMULTI; }

    bool isOpened() const
    {
        return (m_index != -1);
    }

    std::string getUniqueIndentifier() const
    {
        std::string identifier= "cleye_";

        if (isOpened())
        {
            GUID guid = CLEyeGetCameraUUID(m_index);
            char guid_string[128];

            snprintf(guid_string, sizeof(guid_string), "cleye_%x-%x-%x-%x%x%x%x%x%x%x%x",
                guid.Data1, guid.Data2, guid.Data3,
                guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
            identifier.append(guid_string);
        }

        return identifier;
    }

protected:
    
    bool open(int _index) {
        close();
        int cams = CLEyeGetCameraCount();
        std::cout << "CLEyeGetCameraCount() found " << cams << " devices." << std::endl;
        if (_index < cams)
        {
            std::cout << "Attempting to open camera " << _index << " of " << cams << "." << std::endl;
            GUID guid = CLEyeGetCameraUUID(_index);
            m_eye = CLEyeCreateCamera(guid, CLEYE_COLOR_PROCESSED, CLEYE_VGA, 75);
            CLEyeCameraGetFrameDimensions(m_eye, m_width, m_height);
            
            m_frame4ch = cvCreateImage(cvSize(m_width, m_height), IPL_DEPTH_8U, 4);
            m_frame = cvCreateImage(cvSize(m_width, m_height), IPL_DEPTH_8U, 3);
            
            CLEyeCameraStart(m_eye);
            CLEyeSetCameraParameter(m_eye, CLEYE_AUTO_EXPOSURE, false);
            CLEyeSetCameraParameter(m_eye, CLEYE_AUTO_GAIN, false);
            m_index = _index;
        }
        return isOpened();
    }
    
    void close()
    {
        if (isOpened())
        {
            CLEyeCameraStop(m_eye);
            CLEyeDestroyCamera(m_eye);
        }
        cvReleaseImage(&m_frame);
        cvReleaseImage(&m_frame4ch);
        m_index = -1;
    }
    
    
    int m_index, m_width, m_height;
    PBYTE pCapBuffer;
    IplImage* m_frame;
    IplImage* m_frame4ch;
    CLEyeCameraInstance m_eye;
};

// We don't need an implementation for CL EYE Driver because
// it uses the native DShow IVideoCapture device.
#endif

#ifdef HAVE_PS3EYE
/// Implementation of PS3EyeCapture when using PS3EYEDriver
class PSEYECaptureCAM_PS3EYE : public cv::IVideoCapture
{
public:
    PSEYECaptureCAM_PS3EYE(int _index)
    : m_index(-1), m_width(-1), m_height(-1), m_widthStep(-1),
    m_size(-1), m_MatBayer(0, 0, CV_8UC1)
    {
        //CoInitialize(NULL);
        open(_index);
    }

    ~PSEYECaptureCAM_PS3EYE()
    {
        close();
    }

    double getProperty(int property_id) const
    {
        switch (property_id)
        {
        case CV_CAP_PROP_BRIGHTNESS:
            return (double)(eye->getBrightness());
        case CV_CAP_PROP_CONTRAST:
            return (double)(eye->getContrast());
        case CV_CAP_PROP_EXPOSURE:
            // Default 120
            return (double)(eye->getExposure());
        case CV_CAP_PROP_FPS:
            return (double)(eye->getFrameRate());
        case CV_CAP_PROP_FRAME_HEIGHT:
            return (double)(eye->getHeight());
        case CV_CAP_PROP_FRAME_WIDTH:
            return (double)(eye->getWidth());
        case CV_CAP_PROP_GAIN:
            // [0, 63] -> [0, 255]
            return (double)(eye->getGain())*256.0/64.0;
        case CV_CAP_PROP_HUE:
            return (double)(eye->getHue());
        case CV_CAP_PROP_SHARPNESS:
            // [0, 63] -> [0, 255]
            return (double)(eye->getSharpness())*256.0 / 64.0;
        }
        return 0;
    }

    bool setProperty(int property_id, double value)
    {
        //set height and set width are disabled since they were added in a branch of ps3driver that we do not have.
        int val;
        if (!eye)
        {
            return false;
        }
        switch (property_id)
        {
        case CV_CAP_PROP_BRIGHTNESS:
            // [0, 255] [20]
            eye->setBrightness((int)round(value));
			break;
        case CV_CAP_PROP_CONTRAST:
            // [0, 255] [37]
            eye->setContrast((int)round(value));
			break;
        case CV_CAP_PROP_EXPOSURE:
            // [0, 255] [120]
            eye->setExposure((int)round(value));
			break;
        case CV_CAP_PROP_FPS:
			// [15, 20, 30, 40 50, 60, 75]
			eye->stop();
			if (!eye->setFrameRate((int)round(value))) return false;
			eye->start();
			break;
        case CV_CAP_PROP_FRAME_HEIGHT:
			eye->stop();
			//if (!eye->setHeight((int)round(value))) return false;
			eye->start();
			break;
            //return false; //TODO: Modifying frame size probably requires resetting the camera
        case CV_CAP_PROP_FRAME_WIDTH:
			eye->stop();
			//if (!eye->setWidth((int)round(value))) return false;
			eye->start();
			break;
            //return false;
        case CV_CAP_PROP_GAIN:
            // [0, 255] -> [0, 63] [20]
            val = (int)(value * 64.0 / 256.0);
            eye->setGain(val);
			break;
        case CV_CAP_PROP_HUE:
            // [0, 255] [143]
            eye->setHue((int)round(value));
			break;
        case CV_CAP_PROP_SHARPNESS:
            // [0, 255] -> [0, 63] [0]
            val = (int)(value * 64.0 / 256.0);
            eye->setSharpness((int)round(value));
        }
        
        refreshDimensions();
        
        return true;
    }

    bool grabFrame()
    {
        return eye->isStreaming();
    }

    bool retrieveFrame(int outputType, cv::OutputArray outArray)
    {
        eye->getFrame(m_MatBayer.data);

        //not sure what BayerBG2BGR is, but it seems to be the original format of data. Probably the source of weird colors?

        //UPDATE: weird colors fixed themselves, somehow?
        cv::cvtColor(m_MatBayer, outArray, cv::COLOR_BayerGB2BGR);

        //flip the image
        cv::flip(outArray, outArray, 1);

        return true;
    }

    int getCaptureDomain() {
        return PSEYE_CAP_PS3EYE;
    }
    
    bool isOpened() const
    {
        return (m_index != -1);
    }

    std::string getUniqueIndentifier() const
    {
        std::string identifier = "ps3eye_";

        if (isOpened())
        {
            char usb_port_path[128];

            if (eye->getUSBPortPath(usb_port_path, sizeof(usb_port_path)))
            {
                identifier.append(usb_port_path);
            }
        }

        return identifier;
    }

protected:
    
    bool open(int _index)
    {
        // Enumerate libusb devices
        std::vector<ps3eye::PS3EYECam::PS3EYERef> devices = ps3eye::PS3EYECam::getDevices();
		std::cout << "ps3eye::PS3EYECam::getDevices() found " << devices.size() << " devices." << std::endl;
        
        if (devices.size() > (unsigned int)_index) {
            
            eye = devices[_index];

            if (eye && eye->init(640, 480, 15, ps3eye::PS3EYECam::EOutputFormat::Bayer))
            {
                // Change any default settings here
                
                eye->start();
                
                eye->setAutogain(false);
                eye->setAutoWhiteBalance(false);

				eye->setFlip(true, false);
                
                m_index = _index;
                refreshDimensions();
                
                return true;
            }
        }
        return false;
    }
    
    void close()
    {
        // eye will close itself when going out of scope.
        m_index = -1;
    }
    
    void refreshDimensions()
    {
        m_width = eye->getWidth();
        m_widthStep = eye->getRowBytes(); // just width * 1 byte per pixel.
        m_height = eye->getHeight();
        m_size = m_widthStep * m_height;
        m_MatBayer.create(cv::Size(m_width, m_height), CV_8UC1);
    }

    int m_index, m_width, m_height, m_widthStep;
    size_t m_size;
    cv::Mat m_MatBayer;
    ps3eye::PS3EYECam::PS3EYERef eye;
};

#endif

static bool usingCLEyeDriver()
{
    bool cleyedriver_found = false;
    // Check if CLEYE_DRIVER
#ifdef HAVE_CLEYE
    PlatformDeviceAPIWin32 platformAPI;
    char provider_name[128];

    if (platformAPI.get_device_property(
			DeviceClass::DeviceClass_Camera,
			VENDOR_ID,
			PRODUCT_ID,
			k_reg_property_provider_name,
			provider_name,
			sizeof(provider_name)))
    {
        if (strcmp(provider_name, CLEYE_DRIVER_PROVIDER_NAME) == 0)
        {
            cleyedriver_found = true;
        }
    }
#endif
    return cleyedriver_found;
}


/*
-- Definitions for PSEyeVideoCapture --
*/
bool PSEyeVideoCapture::open(int index, int apiPreference)
{
    if (isOpened())
    {
        release();
    }

	// Try to open a PS3EYE-specific camera capture
    // Only works for CLEYE_MULTICAM and PS3EYEDRIVER
    icap = pseyeVideoCapture_create(index);
    if (!icap.empty())
    {
        return true;
    }
    
    // Keep track of the camera index. Necessary for CLEyeDriver only.
    // non -1 m_index is used as a CL Eye Driver check elsewhere.
    if (usingCLEyeDriver())
    {
        m_index = index;
        std::cout << "CL Eye Driver being used with native DShow. Setting m_index to " << m_index << std::endl;

        if (!isOpened())
        {
            std::cout << "Attempting cv::VideoCapture::open(index) for CLEye DShow camera." << std::endl;
            return cv::VideoCapture::open(index);
        }
    }

	// PS3EYE-specific camera capture if available, else use base class open()

    //###HipsterSloth $TODO
    // Disabling the OpenCV camera open fallback.
    // We don't officially support anything but the PS3Eye camera at the moment
    // and it's currently confusing debugging other peoples camera issues with 
    // this code path in place (random web cams getting opened)
    //if (!isOpened())
    //{
    //    std::cout << "Attempting cv::VideoCapture::open(index)" << std::endl;
    //    return cv::VideoCapture::open(index);
    //}

    return isOpened();
}

bool PSEyeVideoCapture::set(int propId, double value)
{
#ifdef HAVE_CLEYE
    if (m_index != -1)
    {
        bool param_set = false;
        int val;
        HKEY hKey;
        DWORD l = sizeof(DWORD);
        int err = RegOpenKeyExA(HKEY_CURRENT_USER, CL_DRIVER_REG_PATH, 0, KEY_ALL_ACCESS, &hKey);
        if (err != ERROR_SUCCESS) {
            printf("Error: %d Unable to open reg-key:  [HKCU]\\%s!\n", err, CL_DRIVER_REG_PATH);
            printf("CL-Eye Test must be run at least once for each Windows user.\n");
            return false;
        }

        switch (propId)
        {
        case CV_CAP_PROP_EXPOSURE:
            val = (value == 0);
            RegSetValueExA(hKey, "AutoAEC", 0, REG_DWORD, (CONST BYTE*)&val, l);
            val = (int)(value * 2) % 511;
            RegSetValueExA(hKey, "Exposure", 0, REG_DWORD, (CONST BYTE*)&val, l);
            param_set = true;
            break;
        case CV_CAP_PROP_GAIN:
            val = (value == 0);
            RegSetValueExA(hKey, "AutoAGC", 0, REG_DWORD, (CONST BYTE*)&val, l);
            val = (int)ceil(value * 79/256) % 79;
            RegSetValueExA(hKey, "Gain", 0, REG_DWORD, (CONST BYTE*)&val, l);
            param_set = true;
            break;
        default:
            param_set = cv::VideoCapture::set(propId, value);
        }

        // restart the camera capture
        if (param_set && icap) {
            std::cout << "Parameter changed via registry. Resetting capture device." << std::endl;
            cv::VideoCapture::open(m_index);
        }

        return param_set;
    }
#endif
    return cv::VideoCapture::set(propId, value);

}

double PSEyeVideoCapture::get(int propId) const
{
#ifdef HAVE_CLEYE
    if (m_index != -1)
    {
        HKEY hKey;
        DWORD l = sizeof(DWORD);
        DWORD resultA = 0;
        DWORD resultB = 0;

        int err = RegOpenKeyExA(HKEY_CURRENT_USER, CL_DRIVER_REG_PATH, 0, KEY_ALL_ACCESS, &hKey);
        if (err != ERROR_SUCCESS) {
            printf("Error: %d Unable to open reg-key:  [HKCU]\\%s!\n", err, CL_DRIVER_REG_PATH);
            printf("CL-Eye Test must be run at least once for each Windows user.\n");
            return 0;
        }

        switch (propId)
        {
        case CV_CAP_PROP_EXPOSURE:
            RegQueryValueExA(hKey, "AutoAEC", NULL, NULL, (LPBYTE)&resultA, &l);
            RegQueryValueExA(hKey, "Exposure", NULL, NULL, (LPBYTE)&resultB, &l);
            return (resultA == 1) ? 0 : ((double)(resultB))/2.0;
        case CV_CAP_PROP_GAIN:
            RegQueryValueExA(hKey, "AutoAGC", NULL, NULL, (LPBYTE)&resultA, &l);
            RegQueryValueExA(hKey, "Gain", NULL, NULL, (LPBYTE)&resultB, &l);
            return (resultA == 1) ? 0 : ((double)(resultB))*(256.0/79.0);
        default:
            return cv::VideoCapture::get(propId);
        }
    }
#endif
    return cv::VideoCapture::get(propId);
}

std::string PSEyeVideoCapture::getUniqueIndentifier() const
{
    return m_indentifier;
}

cv::Ptr<cv::IVideoCapture> PSEyeVideoCapture::pseyeVideoCapture_create(int index)
{
    // https://github.com/Itseez/opencv/blob/09e6c82190b558e74e2e6a53df09844665443d6d/modules/videoio/src/cap.cpp#L432
    int  domains[] =
    {
#ifdef HAVE_CLEYE
        PSEYE_CAP_CLMULTI,
        PSEYE_CAP_CLEYE,
#endif
#ifdef HAVE_PS3EYE
        PSEYE_CAP_PS3EYE,
#endif
        -1, -1
    };
    
    // interpret preferred interface (0 = autodetect)
    int pref = (index / 100) * 100;
    if (pref)
    {
        domains[0]=pref;
        index %= 100;
        domains[1]=-1;
    }
    // try every possibly installed camera API
    for (int i = 0; domains[i] >= 0; i++)
    {
#if defined(HAVE_CLEYE) || defined(HAVE_PS3EYE) || (0)
        cv::Ptr<cv::IVideoCapture> capture;
        switch (domains[i])
        {
#ifdef HAVE_CLEYE
            case PSEYE_CAP_CLMULTI:
                {
                    capture = cv::makePtr<PSEYECaptureCAM_CLMULTI>(index);
                    m_indentifier = capture.dynamicCast<PSEYECaptureCAM_CLMULTI>()->getUniqueIndentifier();
                }
                break;

            case PSEYE_CAP_CLEYE:
                // We want to use the native OpenCV icap in this case.
                // So we will return empty capture here.
                if (usingCLEyeDriver())
                {
                    std::cout << "CL Eye Driver detected." << std::endl;
                    capture = cv::Ptr<cv::IVideoCapture>();

                    m_indentifier = "opencv_";
                    m_indentifier.append(std::to_string(index));

                    return capture;
                }
                break;
#endif
#ifdef HAVE_PS3EYE
            case PSEYE_CAP_PS3EYE:
                {
                    capture = cv::makePtr<PSEYECaptureCAM_PS3EYE>(index);
                    m_indentifier = capture.dynamicCast<PSEYECaptureCAM_PS3EYE>()->getUniqueIndentifier();
                }
                break;
#endif
        }
        if (capture && capture->isOpened())
        {
            return capture;
        }
#endif
    }
    
    // failed to open a camera
    return cv::Ptr<cv::IVideoCapture>();
}