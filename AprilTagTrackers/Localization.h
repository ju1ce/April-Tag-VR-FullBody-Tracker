#pragma once

#include "Reflectable.h"
#include "Serializable.h"
#include <wx/string.h>

// This typedef hopefully makes it more obvious of the purpose of wxString,
//  and it should only be used for frontend user facing strings.
// On Windows this will be utf-16 and is essentially a typedef for std::wstring,
//  other platforms this is an encoding aware utf-8 string.
// May be useful for windows file paths aswell, although opencv and other
//  libraries don't always support io with widestring paths anyway.
// In general, all unicode string literals should be in the translation yaml files,
//  and not in a source file.
using UniStr = wxString;

// temporary alias, undefined at end of file,
// Makes the visitor fields private, essentially const fields except when Load is called
#define T(key) \
    REFLECTABLE_FIELD(private, _unused_, public, const UniStr, key)

class Localization : protected FileStorageSerializable
{
public:
    explicit Localization(const std::string& lang_code)
        : FileStorageSerializable("locales/lang_" + lang_code + ".yaml")
    {
        Load();
    }

    // TODO:
    void FindAvailableLangs()
    {
    }

    FS_COMMENT("Put all strings in \"double quotes\".");

    T(CODE) = "en";
    T(NAME_ENGLISH) = "English";
    T(NAME_NATIVE) = "English";

    T(APP_TITLE) = "Juices VR Marker Tracking";

    T(TAB_CAMERA) = "Camera";
    T(TAB_PARAMS) = "Params";
    T(TAB_LICENSE) = "License";

    T(CAMERA_START_CAMERA) = "1. Start/Stop camera";
    T(CAMERA_CALIBRATE_CAMERA) = "2. Calibrate camera";
    T(CAMERA_CALIBRATE_TRACKERS) = "3. Calibrate trackers";
    T(CAMERA_START_STEAMVR) = "4. Start up SteamVR!";
    T(CAMERA_CONNECT) = "5. Connect to SteamVR";
    T(CAMERA_START_DETECTION) = "6. Start/Stop";
    T(CAMERA_PREVIEW_CAMERA) = "Preview camera";
    T(CAMERA_PREVIEW_CALIBRATION) = "Preview calibration";
    T(CAMERA_CALIBRATION_MODE) = "Calibration mode";
    T(CAMERA_MULTICAM_CALIB) = "Refine calibration using second camera";
    T(CAMERA_LOCK_HEIGHT) = "Lock camera height";
    T(CAMERA_CALIBRATION_INSTRUCTION) =
        R"(Disable SteamVR home to see the camera.
Use your left trigger to grab the camera and move it into position, then use grip to grab trackers and move those into position.

Uncheck Calibration mode when done!)";
    T(CAMERA_DISABLE_OUT) = "Disable out window";
    T(CAMERA_DISABLE_OPENVR_API) = "Disable OpenVR API use";

    T(PARAMS_CAMERA_TOOLTIP_API_1) = "0: No preference\n\nCamera backends:";
    T(PARAMS_CAMERA_TOOLTIP_API_2) = "Stream backends:";

    T(PARAMS_LANGUAGE) = "Language";
    T(PARAMS_CAMERA) = "CAMERA PARAMETERS";
    T(PARAMS_CAMERA_NAME_ID) = "Ip or ID of camera";
    T(PARAMS_CAMERA_TOOLTIP_ID) = "Will be a number 0-10 for USB cameras and\nhttp://<ip here>:8080/video for IP webcam)";
    T(PARAMS_CAMERA_NAME_API) = "Camera API preference";
    T(PARAMS_CAMERA_NAME_ROT_CLOCKWISE) = "Rotate camera clockwise";
    T(PARAMS_CAMERA_TOOLTIP_ROT_CLOCKWISE) = "Rotate the camera 90°. Use both to rotate image 180°";
    T(PARAMS_CAMERA_NAME_ROT_CCLOCKWISE) = "Rotate camera counterclockwise";
    T(PARAMS_CAMERA_TOOLTIP_ROT_CCLOCKWISE) = "Rotate the camera 90°. Use both to rotate image 180°";
    T(PARAMS_CAMERA_NAME_WIDTH) = "Camera width in pixels";
    T(PARAMS_CAMERA_TOOLTIP_WIDTH) = "Width and height should be fine on 0, but change it to the camera resolution in case camera doesn't work correctly.";
    T(PARAMS_CAMERA_NAME_HEIGHT) = "Camera height in pixels";
    T(PARAMS_CAMERA_TOOLTIP_HEIGHT) = "Width and height should be fine on 0, but change it to the camera resolution in case camera doesn't work correctly.";
    T(PARAMS_CAMERA_NAME_FPS) = "Camera FPS";
    T(PARAMS_CAMERA_TOOLTIP_FPS) = "Set the fps of the camera";
    T(PARAMS_CAMERA_NAME_SETTINGS) = "Open camera settings";
    T(PARAMS_CAMERA_TOOLTIP_SETTINGS) = "Should open settings of your camera. Only works with Camera API preference DirectShow (700)";
    T(PARAMS_CAMERA_NAME_3_OPTIONS) = "Enable last 3 camera options";
    T(PARAMS_CAMERA_TOOLTIP_3_OPTIONS) = "Experimental. Checking this will enable the bottom three options, which will otherwise not work. Will also try to disable autofocus.";
    T(PARAMS_CAMERA_NAME_AUTOEXP) = "Camera autoexposure";
    T(PARAMS_CAMERA_TOOLTIP_AUTOEXP) = "Experimental. Will try to set camera autoexposure. Usualy 1 for enable and 0 for disable, but can be something dumb as 0.75 and 0.25,";
    T(PARAMS_CAMERA_NAME_EXP) = "Camera exposure";
    T(PARAMS_CAMERA_TOOLTIP_EXP) = "Experimental. Will try to set camera expousre. Can be on a scale of 0-255, or in exponentials of 2 ( -8 for 4ms exposure)";
    T(PARAMS_CAMERA_NAME_GAIN) = "Camera gain";
    T(PARAMS_CAMERA_TOOLTIP_GAIN) = "Experimental. Will try to set gain. Probably on a scale of 0-255, but could be diffrent based on the camera.";

    T(PARAMS_TRACKER) = "TRACKER PARAMETERS";
    T(PARAMS_TRACKER_NAME_NUM_TRACKERS) = "Number of trackers";
    T(PARAMS_TRACKER_TOOLTIP_NUM_TRACKERS) = "Set to 3 for full body. 2 will not work in vrchat!";
    T(PARAMS_TRACKER_NAME_MARKER_SIZE) = "Size of markers in cm";
    T(PARAMS_TRACKER_TOOLTIP_MARKER_SIZE) = "Measure the white square on markers and input it here";
    T(PARAMS_TRACKER_NAME_QUAD_DECIMATE) = "Quad decimate";
    T(PARAMS_TRACKER_TOOLTIP_QUAD_DECIMATE) = "Can be 1, 1.5, 2, 3, 4. Higher values will increase FPS, but reduce maximum range of detections";
    T(PARAMS_TRACKER_NAME_SEARCH_WINDOW) = "Search window";
    T(PARAMS_TRACKER_TOOLTIP_SEARCH_WINDOW) = "Size of the search window. Smaller window will speed up detection, but having it too small will cause detection to fail if tracker moves too far in one frame.";
    T(PARAMS_TRACKER_NAME_MARKER_LIBRARY) = "Marker library";
    T(PARAMS_TRACKER_TOOLTIP_MARKER_LIBRARY) = "Marker library to use. Leave at ApriltagStandard unless you know what you are doing.";
    T(PARAMS_TRACKER_NAME_USE_CENTERS) = "Use centers of trackers";
    T(PARAMS_TRACKER_TOOLTIP_USE_CENTERS) = "Experimental. Instead of having position of tracker detected at the main marker, it will be the center of all markers in the tracker.";
    T(PARAMS_TRACKER_NAME_IGNORE_0) = "Ignore tracker 0";
    T(PARAMS_TRACKER_TOOLTIP_IGNORE_0) = "If you want to replace the hip tracker with a vive tracker/owotrack, check this option. Keep number of trackers on 3.";

    T(PARAMS_SMOOTHING) = "SMOOTHING PARAMETERS";
    T(PARAMS_SMOOTHING_NAME_WINDOW) = "Smoothing time window";
    T(PARAMS_SMOOTHING_TOOLTIP_WINDOW) = "Values in this time window will be used for interpolation. The higher it is, the less shaking there will be, but it will increase delay. 0.2-0.5 are usualy good values";
    T(PARAMS_SMOOTHING_NAME_ADDITIONAL) = "Additional smoothing";
    T(PARAMS_SMOOTHING_TOOLTIP_ADDITIONAL) = "Extra smoothing applied to tracker position. Should mimic the old style of smoothing in previous versions. 0 for no smoothing, 1 for very slow movement.";
    T(PARAMS_SMOOTHING_NAME_DEPTH) = "Depth smoothing";
    T(PARAMS_SMOOTHING_TOOLTIP_DEPTH) = "Experimental. Additional smoothing applied to the depth estimation, as it has higher error. Cam help remove shaking with multiple cameras.";
    T(PARAMS_SMOOTHING_NAME_CAM_LATENCY) = "Camera latency";
    T(PARAMS_SMOOTHING_TOOLTIP_CAM_LATENCY) = "Represents camera latency in seconds. Should counter any delay when using an IP camera. Usualy lower than 0.1.";

    T(PARAMS_HOVER_HELP) = "Hover over text for help!";
    T(PARAMS_SAVE) = "Save";

    T(PARAMS_NOTE_LOW_SMOOTHING) = "NOTE: Smoothing time window is extremely low, which may cause problems. \n\nIf you get any problems with tracking, try to increase it.";
    T(PARAMS_NOTE_QUAD_NONSTANDARD) = "NOTE: Quad Decimate is not a standard value. \n\nKeep it at 1, 1.5, 2, 3 or 4, or else detection may not work.";
    T(PARAMS_NOTE_NO_DSHOW_CAMSETTINGS) = "NOTE: Camera settings parameter is on, but camera API preference is not 700 \n\nOpening camera parameters only work when camera API is set to DirectShow, or 700.";
    T(PARAMS_NOTE_LATENCY_GREATER_SMOOTHING) = "NOTE: Camera latency should never be higher than smoothing time window or tracking isnt going to work. \n\nYou probably dont want it any higher than 0.1, and smoothing window probably shouldnt be under 0.2 unless you use high speed cameras.";
    T(PARAMS_NOTE_HIGH_SMOOTHING) = "NOTE: Smoothing time window is over 1 second, which will cause very slow movement! \n\nYou probably want to update it to something like 0.5.";
    T(PARAMS_NOTE_2TRACKERS_IGNORE0) = "Number of trackers is 2 and ignore tracker 0 is on. This will result in only 1 tracker spawning in SteamVR. \nIf you wish to use both feet trackers, keep number of trackers at 3.";
    T(PARAMS_NOTE_LANGUAGECHANGE) = "Language has been changed! Please restart application to apply.";

    T(PARAMS_SAVED_MSG) = "Parameters saved!";

    T(PARAMS_WRONG_VALUES) = "Please enter appropriate values. Parameters were not saved.";

    T(TRACKER_CAMERA_START_ERROR) =
        R"(Could not start camera.Make sure you entered the correct ID or IP of your camera in the params.
"For USB cameras, it will be a number, usually 0,1,2... try a few until it works.
"For IP webcam, the address will be in the format http://<ip here>:8080/video)";
    T(TRACKER_CAMERA_ERROR) = "Camera error";
    T(TRACKER_CAMERA_PREVIEW) = "Preview";

    T(TRACKER_CAMERA_NOTRUNNING) = "Camera not running";

    T(TRACKER_CAMERA_CALIBRATION_INSTRUCTIONS) =
        R"(Camera calibration started!

Place the printed Charuco calibration board on a flat surface, or display it on a (non-curved) monitor. The camera will take a picture every second - take pictures of the board from as many different angles and distances as you can.

Make sure that the board is seen nicely and isn't blurred. Move the camera around slowly.

Purple dots = great
Green dots = okay
Yellow dots = bad

The grid should be fairly flat, fairly stable (can still shake a couple of pixels) and take over the whole image.
The dots should be spread across the whole image

Press OK to save calibration when done.)";

    T(TRACKER_CAMERA_CALIBRATION_COMPLETE) = "Calibration complete.";
    T(TRACKER_CAMERA_CALIBRATION_NOTDONE) = "Calibration has not been completed as too few images have been taken.";

    T(TRACKER_CAMERA_NOTCALIBRATED) = "Camera not calibrated";

    T(TRACKER_TRACKER_CALIBRATION_INSTRUCTIONS) =
        R"(Tracker calibration started!

Before calibrating, set the number of trackers and marker size parameters (measure the white square). Make sure the trackers are completely rigid and cannot bend,
neither the markers or at the connections between markers - use images on github for reference. Wear your trackers, then calibrate them by moving them to the camera closer than 30cm.

Green: This marker is calibrated and can be used to calibrate other markers.
Blue: This marker is not part of any used trackers. You probably have to increase number of trackers in params.
Purple: This marker is too far from the camera to be calibrated. Move it closer than 30cm.
Red: This marker cannot be calibrated as no green markers are seen. Rotate the tracker until a green marker is seen along this one.
Yellow: The marker is being calibrated. Hold it still for a second.

When all the markers on all trackers are shown as green, press OK to finish calibration.)";

    T(TRACKER_TRACKER_NOTCALIBRATED) = "Trackers not calibrated";

    T(TRACKER_STEAMVR_NOTCONNECTED) = "Not connected to SteamVR";

    T(TRACKER_CALIBRATION_SOMETHINGWRONG) = "Something went wrong. Try again.";
    T(CONNECT_SOMETHINGWRONG) = "Something went wrong. Try again.";
    T(TRACKER_DETECTION_SOMETHINGWRONG) = "Something went wrong when estimating tracker pose. Try again! \n"
                                          "If the problem persists, try to recalibrate camera and trackers.";

    T(CONNECT_ALREADYCONNECTED) = "Already connected. Restart connection?";
    T(CONNECT_CLIENT_ERROR) = "Error when connecting to SteamVR as a client! Make sure your HMD is connected. \nError code: ";
    T(CONNECT_BINDINGS_ERROR) = "Could not find bindings file att_actions.json.";
    T(CONNECT_DRIVER_ERROR) =
        R"(Could not connect to SteamVR driver. If error code is 2, make sure SteamVR is running and the apriltagtrackers driver is installed and enabled in settings.
You may also have to run bin/ApriltagTrackers.exe as administrator, if error code is not 2.
Error code: )";

    T(CONNECT_DRIVER_MISSMATCH_1) = "Driver version and ATT version do not match! \n\nDriver version: ";
    T(CONNECT_DRIVER_MISSMATCH_2) = "\nExpected driver version: ";
};

#undef T
