#pragma once

#include <wx/string.h>

#include <string>
#include <opencv2/videoio/registry.hpp>

class Lang
{
public:
	wxString APP_TITLE = "Juices VR Marker Tracking";

	wxString TAB_CAMERA = "Camera";
	wxString TAB_PARAMS = "Params";
	wxString TAB_LICENSE = "License";

	wxString CAMERA_START_CAMERA = "1. Start/Stop camera";
	wxString CAMERA_CALIBRATE_CAMERA = "2. Calibrate camera";
	wxString CAMERA_CALIBRATE_TRACKERS = "3. Calibrate trackers";
	wxString CAMERA_START_STEAMVR = "4. Start up SteamVR!";
	wxString CAMERA_CONNECT = "5. Connect to SteamVR";
	wxString CAMERA_START_DETECTION = "6. Start/Stop";
	wxString CAMERA_PREVIEW_CAMERA = "Preview camera";
	wxString CAMERA_PREVIEW_CALIBRATION = "Preview calibration";
	wxString CAMERA_CALIBRATION_MODE = "Calibration mode";
	wxString CAMERA_MULTICAM_CALIB = "Refine calibration using second camera";
	wxString CAMERA_LOCK_HEIGHT = "Lock camera height";
	wxString CAMERA_CALIBRATION_INSTRUCTION = "Disable SteamVR home to see the camera.\nUse your left trigger to grab the camera and move it into position, then use grip to grab trackersand move those into position.\nUncheck Calibration mode when done!\n\n\n";
	wxString CAMERA_DISABLE_OUT = "Disable out window";
	wxString CAMERA_DISABLE_OPENVR_API = "Disable OpenVR API use";

	wxString cameraApiDescriptions = []()
	{
		std::stringstream description;
		description << "0: No preference\n\nCamera backends:";
		for (const auto backend : cv::videoio_registry::getCameraBackends())
		{
			description << "\n" << int(backend) << ": " << cv::videoio_registry::getBackendName(backend);
		}
		description << "\n\nStream backends:";
		for (const auto backend : cv::videoio_registry::getStreamBackends())
		{
			description << "\n" << int(backend) << ": " << cv::videoio_registry::getBackendName(backend);
		}
		return description.str();
	}();

	wxString PARAMS_LANGUAGE = "Language";

	wxString PARAMS_CAMERA = "CAMERA PARAMTERS";
	wxString PARAMS_CAMERA_NAME_ID = "Ip or ID of camera";
	wxString PARAMS_CAMERA_TOOLTIP_ID = "Will be a number 0-10 for USB cameras and \nhttp://'ip - here':8080/video for IP webcam";
	wxString PARAMS_CAMERA_NAME_API = "Camera API preference";
	wxString PARAMS_CAMERA_TOOLTIP_API = cameraApiDescriptions;
	wxString PARAMS_CAMERA_NAME_ROT_CLOCKWISE = "Rotate camera clockwise";
	wxString PARAMS_CAMERA_TOOLTIP_ROT_CLOCKWISE = "Rotate the camera 90°. Use both to rotate image 180°";
	wxString PARAMS_CAMERA_NAME_ROT_CCLOCKWISE = "Rotate camera counterclockwise";
	wxString PARAMS_CAMERA_TOOLTIP_ROT_CCLOCKWISE = "Rotate the camera 90°. Use both to rotate image 180°";
	wxString PARAMS_CAMERA_NAME_WIDTH = "Camera width in pixels";
	wxString PARAMS_CAMERA_TOOLTIP_WIDTH = "Width and height should be fine on 0, but change it to the camera resolution in case camera doesn't work correctly.";
	wxString PARAMS_CAMERA_NAME_HEIGHT = "Camera height in pixels";
	wxString PARAMS_CAMERA_TOOLTIP_HEIGHT = "Width and height should be fine on 0, but change it to the camera resolution in case camera doesn't work correctly.";
	wxString PARAMS_CAMERA_NAME_FPS = "Camera FPS";
	wxString PARAMS_CAMERA_TOOLTIP_FPS = "Set the fps of the camera";
	wxString PARAMS_CAMERA_NAME_SETTINGS = "Open camera settings";
	wxString PARAMS_CAMERA_TOOLTIP_SETTINGS = "Should open settings of your camera. Only works with Camera API preference DirectShow (700)";
	wxString PARAMS_CAMERA_NAME_3_OPTIONS = "Enable last 3 camera options";
	wxString PARAMS_CAMERA_TOOLTIP_3_OPTIONS = "Experimental. Checking this will enable the bottom three options, which will otherwise not work. Will also try to disable autofocus.";
	wxString PARAMS_CAMERA_NAME_AUTOEXP = "Camera autoexposure";
	wxString PARAMS_CAMERA_TOOLTIP_AUTOEXP = "Experimental. Will try to set camera autoexposure. Usualy 1 for enable and 0 for disable, but can be something dumb as 0.75 and 0.25,";
	wxString PARAMS_CAMERA_NAME_EXP = "Camera exposure";
	wxString PARAMS_CAMERA_TOOLTIP_EXP = "Experimental. Will try to set camera expousre. Can be on a scale of 0-255, or in exponentials of 2 ( -8 for 4ms exposure)";
	wxString PARAMS_CAMERA_NAME_GAIN = "Camera gain";
	wxString PARAMS_CAMERA_TOOLTIP_GAIN = "Experimental. Will try to set gain. Probably on a scale of 0-255, but could be diffrent based on the camera.";

	wxString PARAMS_TRACKER = "TRACKER PARAMETERS";
	wxString PARAMS_TRACKER_NAME_NUM_TRACKERS = "Number of trackers";
	wxString PARAMS_TRACKER_TOOLTIP_NUM_TRACKERS = "Set to 3 for full body. 2 will not work in vrchat!";
	wxString PARAMS_TRACKER_NAME_MARKER_SIZE = "Size of markers in cm";
	wxString PARAMS_TRACKER_TOOLTIP_MARKER_SIZE = "Measure the white square on markers and input it here";
	wxString PARAMS_TRACKER_NAME_QUAD_DECIMATE = "Quad decimate";
	wxString PARAMS_TRACKER_TOOLTIP_QUAD_DECIMATE = "Can be 1, 1.5, 2, 3, 4. Higher values will increase FPS, but reduce maximum range of detections";
	wxString PARAMS_TRACKER_NAME_SEARCH_WINDOW = "Search window";
	wxString PARAMS_TRACKER_TOOLTIP_SEARCH_WINDOW = "Size of the search window. Smaller window will speed up detection, but having it too small will cause detection to fail if tracker moves too far in one frame.";
	wxString PARAMS_TRACKER_NAME_MARKER_LIBRARY = "Marker library";
	wxString PARAMS_TRACKER_TOOLTIP_MARKER_LIBRARY = "Marker library to use. Leave at ApriltagStandard unless you know what you are doing.";
	wxString PARAMS_TRACKER_NAME_USE_CENTERS = "Use centers of trackers";
	wxString PARAMS_TRACKER_TOOLTIP_USE_CENTERS = "Experimental. Instead of having position of tracker detected at the main marker, it will be the center of all markers in the tracker.";
	wxString PARAMS_TRACKER_NAME_IGNORE_0 = "Ignore tracker 0";
	wxString PARAMS_TRACKER_TOOLTIP_IGNORE_0 = "If you want to replace the hip tracker with a vive tracker/owotrack, check this option. Keep number of trackers on 3.";

	wxString PARAMS_SMOOTHING = "SMOOTHING PARAMETERS";
	wxString PARAMS_SMOOTHING_NAME_WINDOW = "Smoothing time window";
	wxString PARAMS_SMOOTHING_TOOLTIP_WINDOW = "Values in this time window will be used for interpolation. The higher it is, the less shaking there will be, but it will increase delay. 0.2-0.5 are usualy good values";
	wxString PARAMS_SMOOTHING_NAME_ADDITIONAL = "Additional smoothing";
	wxString PARAMS_SMOOTHING_TOOLTIP_ADDITIONAL = "Extra smoothing applied to tracker position. Should mimic the old style of smoothing in previous versions. 0 for no smoothing, 1 for very slow movement.";
	wxString PARAMS_SMOOTHING_NAME_DEPTH = "Depth smoothing";
	wxString PARAMS_SMOOTHING_TOOLTIP_DEPTH = "Experimental. Additional smoothing applied to the depth estimation, as it has higher error. Cam help remove shaking with multiple cameras.";
	wxString PARAMS_SMOOTHING_NAME_CAM_LATENCY = "Camera latency";
	wxString PARAMS_SMOOTHING_TOOLTIP_CAM_LATENCY = "Represents camera latency in seconds. Should counter any delay when using an IP camera. Usualy lower than 0.1.";

	wxString PARAMS_HOVER_HELP = "Hover over text for help!";
	wxString PARAMS_SAVE = "Save";

	wxString PARAMS_NOTE_LOW_SMOOTHING = "NOTE: Smoothing time window is extremely low, which may cause problems. \n\nIf you get any problems with tracking, try to increase it.";
	wxString PARAMS_NOTE_QUAD_NONSTANDARD = "NOTE: Quad Decimate is not a standard value. \n\nKeep it at 1, 1.5, 2, 3 or 4, or else detection may not work.";
	wxString PARAMS_NOTE_NO_DSHOW_CAMSETTINGS = "NOTE: Camera settings parameter is on, but camera API preference is not 700 \n\nOpening camera parameters only work when camera API is set to DirectShow, or 700.";
	wxString PARAMS_NOTE_LATENCY_GREATER_SMOOTHING = "NOTE: Camera latency should never be higher than smoothing time window or tracking isnt going to work. \n\nYou probably dont want it any higher than 0.1, and smoothing window probably shouldnt be under 0.2 unless you use high speed cameras.";
	wxString PARAMS_NOTE_HIGH_SMOOTHING = "NOTE: Smoothing time window is over 1 second, which will cause very slow movement! \n\nYou probably want to update it to something like 0.5.";
	wxString PARAMS_NOTE_2TRACKERS_IGNORE0 = "Number of trackers is 2 and ignore tracker 0 is on. This will result in only 1 tracker spawning in SteamVR. \nIf you wish to use both feet trackers, keep number of trackers at 3.";
	wxString PARAMS_NOTE_LANGUAGECHANGE = "Language has been changed! Please restart application to apply.";


	wxString PARAMS_SAVED_MSG = "Parameters saved!";

	wxString PARAMS_WRONG_VALUES = "Please enter appropriate values.Parameters were not saved.";

	wxString TRACKER_CAMERA_START_ERROR = "Could not start camera.Make sure you entered the correct ID or IP of your camera in the params.\n"
		"For USB cameras, it will be a number, usually 0,1,2... try a few until it works.\n"
		"For IP webcam, the address will be in the format http://'ip - here':8080/video";
	wxString TRACKER_CAMERA_ERROR = "Camera error";
	wxString TRACKER_CAMERA_PREVIEW = "Preview";

	wxString TRACKER_CAMERA_NOTRUNNING = "Camera not running";

	wxString TRACKER_CAMERA_CALIBRATION_INSTRUCTIONS = "Camera calibration started! \n\n"
		"Place the printed Charuco calibration board on a flat surface, or display it on a (non-curved) monitor. The camera will take a picture every second - take pictures of the board from as many diffrent angles and distances as you can. \n\n"
		"Make sure that the board is seen nicely and isn't blurred. Move the camera around slowly. \n\n"
		"Purple dots = great\nGreen dots = okay\nYellow dots = bad\n\n"
		"The grid should be fairly flat, fairly stable (can still shake a couple of pixels) and take over the whole image.\n"
		"The dots should be spread across the whole image\n\n"
		"Press OK to save calibration when done.";

	wxString TRACKER_CAMERA_CALIBRATION_COMPLETE = "Calibration complete.";
	wxString TRACKER_CAMERA_CALIBRATION_NOTDONE = "Calibration has not been completed as too few images have been taken.";

	wxString TRACKER_CAMERA_NOTCALIBRATED = "Camera not calibrated";

	wxString TRACKER_TRACKER_CALIBRATION_INSTRUCTIONS = "Tracker calibration started! \n\n"
		"Before calibrating, set the number of trackers and marker size parameters (measure the white square). Make sure the trackers are completely rigid and cannot bend,"
		"neither the markers or at the connections between markers - use images on github for reference. Wear your trackers, then calibrate them by moving them to the camera closer than 30cm \n\n"
		"Green: This marker is calibrated and can be used to calibrate other markers.\n"
		"Blue: This marker is not part of any used trackers. You probably have to increase number of trackers in params.\n"
		"Purple: This marker is too far from the camera to be calibrated. Move it closer than 30cm.\n"
		"Red: This marker cannot be calibrated as no green markers are seen. Rotate the tracker until a green marker is seen along this one.\n"
		"Yellow: The marker is being calibrated. Hold it still for a second.\n\n"
		"When all the markers on all trackers are shown as green, press OK to finish calibration.";

	wxString TRACKER_TRACKER_NOTCALIBRATED = "Trackers not calibrated";

	wxString TRACKER_STEAMVR_NOTCONNECTED = "Not connected to SteamVR";

	wxString TRACKER_CALIBRATION_SOMETHINGWRONG = "Something went wrong. Try again.";
	wxString CONNECT_SOMETHINGWRONG = "Something went wrong. Try again.";
	wxString TRACKER_DETECTION_SOMETHINGWRONG = "Something went wrong when estimating tracker pose. Try again! \n"
		"If the problem persists, try to recalibrate camera and trackers.";

	wxString CONNECT_ALREADYCONNECTED = "Already connected. Restart connection?";
	wxString CONNECT_CLIENT_ERROR = "Error when connecting to SteamVR as a client! Make sure your HMD is connected. \nError code: ";
	wxString CONNECT_BINDINGS_ERROR = "Could not find bindings file att_actions.json."; 		//NOT TRANSLATED YET
	wxString CONNECT_DRIVER_ERROR = "Could not connect to SteamVR driver. If error code is 2, make sure SteamVR is running and the apriltagtrackers driver is installed and enabled in settings. \n"
		"You may also have to run bin/ApriltagTrackers.exe as administrator, if error code is not 2. \nError code: ";

	wxString CONNECT_DRIVER_MISSMATCH1 = "Driver version and ATT version do not match! \n\nDriver version: ";
	wxString CONNECT_DRIVER_MISSMATCH2 = "\nExpected driver version: ";


	//Other strings, not for translation

	wxString LANGUAGE_ENGLISH = "English";
	wxString LANGUAGE_CHINESE = L"简体中文";

};

Lang get_lang_chinese();

