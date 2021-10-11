#pragma once

#include <string>
#include <opencv2/videoio/registry.hpp>

#include "Language.h"

Lang get_lang_chinese()
{
	Lang language;

	language.APP_TITLE = L"Juices VR Marker Tracking, but chinese   简简";		//needs the L prefix or chinese characters wont work

	language.TAB_CAMERA = "Hello";
	language.TAB_PARAMS = "There";
	language.TAB_LICENSE = "Nana";

	language.CAMERA_START_CAMERA = "1. Start/Stop camera";
	language.CAMERA_CALIBRATE_CAMERA = "2. Calibrate camera";
	language.CAMERA_CALIBRATE_TRACKERS = "3. Calibrate trackers";
	language.CAMERA_START_STEAMVR = "4. Start up SteamVR!";
	language.CAMERA_CONNECT = "5. Connect to SteamVR";
	language.CAMERA_START_DETECTION = "6. Start/Stop";
	language.CAMERA_PREVIEW_CAMERA = "Preview camera";
	language.CAMERA_PREVIEW_CALIBRATION = "Preview calibration";
	language.CAMERA_CALIBRATION_MODE = "Calibration mode";
	language.CAMERA_MULTICAM_CALIB = "Refine calibration using second camera";
	language.CAMERA_LOCK_HEIGHT = "Lock camera height";
	language.CAMERA_CALIBRATION_INSTRUCTION = "Disable SteamVR home to see the camera.\nUse your left trigger to grab the camera and move it into position, then use grip to grab trackersand move those into position.\nUncheck Calibration mode when done!\n\n\n";

	language.cameraApiDescriptions = []()
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

	language.PARAMS_LANGUAGE = L"简体中文";

	language.PARAMS_CAMERA = "CAMERA PARAMTERS";
	language.PARAMS_CAMERA_NAME_ID = "Ip or ID of camera";
	language.PARAMS_CAMERA_TOOLTIP_ID = "Will be a number 0-10 for USB cameras and \nhttp://'ip - here':8080/video for IP webcam";
	language.PARAMS_CAMERA_NAME_API = "Camera API preference";
	language.PARAMS_CAMERA_TOOLTIP_API = language.cameraApiDescriptions;
	language.PARAMS_CAMERA_NAME_ROT_CLOCKWISE = "Rotate camera clockwise";
	language.PARAMS_CAMERA_TOOLTIP_ROT_CLOCKWISE = "Rotate the camera 90°. Use both to rotate image 180°";
	language.PARAMS_CAMERA_NAME_ROT_CCLOCKWISE = "Rotate camera counterclockwise";
	language.PARAMS_CAMERA_TOOLTIP_ROT_CCLOCKWISE = "Rotate the camera 90°. Use both to rotate image 180°";
	language.PARAMS_CAMERA_NAME_WIDTH = "Camera width in pixels";
	language.PARAMS_CAMERA_TOOLTIP_WIDTH = "Width and height should be fine on 0, but change it to the camera resolution in case camera doesn't work correctly.";
	language.PARAMS_CAMERA_NAME_HEIGHT = "Camera height in pixels";
	language.PARAMS_CAMERA_TOOLTIP_HEIGHT = "Width and height should be fine on 0, but change it to the camera resolution in case camera doesn't work correctly.";
	language.PARAMS_CAMERA_NAME_FPS = "Camera FPS";
	language.PARAMS_CAMERA_TOOLTIP_FPS = "Set the fps of the camera";
	language.PARAMS_CAMERA_NAME_SETTINGS = "Open camera settings";
	language.PARAMS_CAMERA_TOOLTIP_SETTINGS = "Should open settings of your camera. Only works with Camera API preference DirectShow (700)";
	language.PARAMS_CAMERA_NAME_3_OPTIONS = "Enable last 3 camera options";
	language.PARAMS_CAMERA_TOOLTIP_3_OPTIONS = "Experimental. Checking this will enable the bottom three options, which will otherwise not work. Will also try to disable autofocus.";
	language.PARAMS_CAMERA_NAME_AUTOEXP = "Camera autoexposure";
	language.PARAMS_CAMERA_TOOLTIP_AUTOEXP = "Experimental. Will try to set camera autoexposure. Usualy 1 for enable and 0 for disable, but can be something dumb as 0.75 and 0.25,";
	language.PARAMS_CAMERA_NAME_EXP = "Camera exposure";
	language.PARAMS_CAMERA_TOOLTIP_EXP = "Experimental. Will try to set camera expousre. Can be on a scale of 0-255, or in exponentials of 2 ( -8 for 4ms exposure)";
	language.PARAMS_CAMERA_NAME_GAIN = "Camera gain";
	language.PARAMS_CAMERA_TOOLTIP_GAIN = "Experimental. Will try to set gain. Probably on a scale of 0-255, but could be diffrent based on the camera.";

	language.PARAMS_TRACKER = "TRACKER PARAMETERS";
	language.PARAMS_TRACKER_NAME_NUM_TRACKERS = "Number of trackers";
	language.PARAMS_TRACKER_TOOLTIP_NUM_TRACKERS = "Set to 3 for full body. 2 will not work in vrchat!";
	language.PARAMS_TRACKER_NAME_MARKER_SIZE = "Size of markers in cm";
	language.PARAMS_TRACKER_TOOLTIP_MARKER_SIZE = "Measure the white square on markers and input it here";
	language.PARAMS_TRACKER_NAME_QUAD_DECIMATE = "Quad decimate";
	language.PARAMS_TRACKER_TOOLTIP_QUAD_DECIMATE = "Can be 1, 1.5, 2, 3, 4. Higher values will increase FPS, but reduce maximum range of detections";
	language.PARAMS_TRACKER_NAME_SEARCH_WINDOW = "Search window";
	language.PARAMS_TRACKER_TOOLTIP_SEARCH_WINDOW = "Size of the search window. Smaller window will speed up detection, but having it too small will cause detection to fail if tracker moves too far in one frame.";
	language.PARAMS_TRACKER_NAME_MARKER_LIBRARY = "Marker library";
	language.PARAMS_TRACKER_TOOLTIP_MARKER_LIBRARY = "Marker library to use. Leave at ApriltagStandard unless you know what you are doing.";
	language.PARAMS_TRACKER_NAME_USE_CENTERS = "Use centers of trackers";
	language.PARAMS_TRACKER_TOOLTIP_USE_CENTERS = "Experimental. Instead of having position of tracker detected at the main marker, it will be the center of all markers in the tracker.";
	language.PARAMS_TRACKER_NAME_IGNORE_0 = "Ignore tracker 0";
	language.PARAMS_TRACKER_TOOLTIP_IGNORE_0 = "If you want to replace the hip tracker with a vive tracker/owotrack, check this option. Keep number of trackers on 3.";

	language.PARAMS_SMOOTHING = "SMOOTHING PARAMETERS";
	language.PARAMS_SMOOTHING_NAME_WINDOW = "Smoothing time window";
	language.PARAMS_SMOOTHING_TOOLTIP_WINDOW = "Values in this time window will be used for interpolation. The higher it is, the less shaking there will be, but it will increase delay. 0.2-0.5 are usualy good values";
	language.PARAMS_SMOOTHING_NAME_ADDITIONAL = "Additional smoothing";
	language.PARAMS_SMOOTHING_TOOLTIP_ADDITIONAL = "Extra smoothing applied to tracker position. Should mimic the old style of smoothing in previous versions. 0 for no smoothing, 1 for very slow movement.";
	language.PARAMS_SMOOTHING_NAME_DEPTH = "Depth smoothing";
	language.PARAMS_SMOOTHING_TOOLTIP_DEPTH = "Experimental. Additional smoothing applied to the depth estimation, as it has higher error. Cam help remove shaking with multiple cameras.";
	language.PARAMS_SMOOTHING_NAME_CAM_LATENCY = "Camera latency";
	language.PARAMS_SMOOTHING_TOOLTIP_CAM_LATENCY = "Represents camera latency in seconds. Should counter any delay when using an IP camera. Usualy lower than 0.1.";

	language.PARAMS_HOVER_HELP = "Hover over text for help!";
	language.PARAMS_SAVE = "Save";

	language.PARAMS_NOTE_LOW_SMOOTHING = "NOTE: Smoothing time window is extremely low, which may cause problems. \n\nIf you get any problems with tracking, try to increase it.";
	language.PARAMS_NOTE_QUAD_NONSTANDARD = "NOTE: Quad Decimate is not a standard value. \n\nKeep it at 1, 1.5, 2, 3 or 4, or else detection may not work.";
	language.PARAMS_NOTE_NO_DSHOW_CAMSETTINGS = "NOTE: Camera settings parameter is on, but camera API preference is not 700 \n\nOpening camera parameters only work when camera API is set to DirectShow, or 700.";
	language.PARAMS_NOTE_LATENCY_GREATER_SMOOTHING = "NOTE: Camera latency should never be higher than additional smoothing or tracking isnt going to work. \n\nYou probably dont want it any higher than 0.1.";
	language.PARAMS_NOTE_HIGH_SMOOTHING = "NOTE: Additional smoothing is over 1 second, which will cause very slow movement! \n\nYou probably want to update it to something like 0.5.";
	language.PARAMS_NOTE_2TRACKERS_IGNORE0 = "Number of trackers is 2 and ignore tracker 0 is on. This will result in only 1 tracker spawning in SteamVR. \nIf you wish to use both feet trackers, keep number of trackers at 3.";
	language.PARAMS_NOTE_LANGUAGECHANGE = L"Language has been changed! Please restart application to apply. But in 简体中文";

	language.PARAMS_SAVED_MSG = "Parameters saved!";

	language.PARAMS_WRONG_VALUES = "Please enter appropriate values.Parameters were not saved.";

	language.TRACKER_CAMERA_START_ERROR = "Could not start camera.Make sure you entered the correct ID or IP of your camera in the params.\n"
		"For USB cameras, it will be a number, usually 0,1,2... try a few until it works.\n"
		"For IP webcam, the address will be in the format http://'ip - here':8080/video";
	language.TRACKER_CAMERA_ERROR = "Camera error";
	language.TRACKER_CAMERA_PREVIEW = "Preview";

	language.TRACKER_CAMERA_NOTRUNNING = "Camera not running";

	language.TRACKER_CAMERA_CALIBRATION_INSTRUCTIONS = "Camera calibration started! \n\n"
		"Place the printed Charuco calibration board on a flat surface, or display it on a (non-curved) monitor. The camera will take a picture every second - take pictures of the board from as many diffrent angles and distances as you can. \n\n"
		"Make sure that the board is seen nicely and isn't blurred. Move the camera around slowly. \n\n"
		"Purple dots = great\nGreen dots = okay\nYellow dots = bad\n\n"
		"The grid should be fairly flat, fairly stable (can still shake a couple of pixels) and take over the whole image.\n"
		"The dots should be spread across the whole image\n\n"
		"Press OK to save calibration when done.";

	language.TRACKER_CAMERA_CALIBRATION_COMPLETE = "Calibration complete.";
	language.TRACKER_CAMERA_CALIBRATION_NOTDONE = "Calibration has not been completed as too few images have been taken.";

	language.TRACKER_CAMERA_NOTCALIBRATED = "Camera not calibrated";

	language.TRACKER_TRACKER_CALIBRATION_INSTRUCTIONS = "Tracker calibration started! \n\n"
		"Before calibrating, set the number of trackers and marker size parameters (measure the white square). Make sure the trackers are completely rigid and cannot bend,"
		"neither the markers or at the connections between markers - use images on github for reference. Wear your trackers, then calibrate them by moving them to the camera closer than 30cm \n\n"
		"Green: This marker is calibrated and can be used to calibrate other markers.\n"
		"Blue: This marker is not part of any used trackers. You probably have to increase number of trackers in params.\n"
		"Purple: This marker is too far from the camera to be calibrated. Move it closer than 30cm.\n"
		"Red: This marker cannot be calibrated as no green markers are seen. Rotate the tracker until a green marker is seen along this one.\n"
		"Yellow: The marker is being calibrated. Hold it still for a second.\n\n"
		"When all the markers on all trackers are shown as green, press OK to finish calibration.";

	language.TRACKER_TRACKER_NOTCALIBRATED = "Trackers not calibrated";

	language.TRACKER_STEAMVR_NOTCONNECTED = "Not connected to SteamVR";

	language.TRACKER_CALIBRATION_SOMETHINGWRONG = "Something went wrong. Try again.";
	language.CONNECT_SOMETHINGWRONG = "Something went wrong. Try again.";
	language.TRACKER_DETECTION_SOMETHINGWRONG = "Something went wrong when estimating tracker pose. Try again! \n"
		"If the problem persists, try to recalibrate camera and trackers.";

	language.CONNECT_ALREADYCONNECTED = "Already connected. Restart connection?";
	language.CONNECT_CLIENT_ERROR = "Error when connecting to SteamVR as a client! Make sure your HMD is connected. \nError code: ";
	language.CONNECT_DRIVER_ERROR = "Could not connect to SteamVR driver. If error code is 2, make sure SteamVR is running and the apriltagtrackers driver is installed and enabled in settings. \n"
		"You may also have to run bin/ApriltagTrackers.exe as administrator, if error code is not 2. \nError code: ";

	language.CONNECT_DRIVER_MISSMATCH1 = "Driver version and ATT version do not match! \n\nDriver version: ";
	language.CONNECT_DRIVER_MISSMATCH2 = "\nExpected driver version: ";

	return language;
};

