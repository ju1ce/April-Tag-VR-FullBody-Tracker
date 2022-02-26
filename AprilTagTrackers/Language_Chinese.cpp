// #pragma once

// #include <string>
// #include <opencv2/videoio/registry.hpp>

// #include "Language.h"

// Lang get_lang_chinese()
// {
// 	Lang language;

// 	language.APP_TITLE = L"果汁的二维码全身追踪（简体中文）";		//needs the L prefix or chinese characters wont work

// 	language.TAB_CAMERA = L"主页";
// 	language.TAB_PARAMS = L"参数";
// 	language.TAB_LICENSE = L"关于";

// 	language.CAMERA_START_CAMERA = L"1. 启动或停止摄像头";
// 	language.CAMERA_CALIBRATE_CAMERA = L"2. 校准摄像头";
// 	language.CAMERA_CALIBRATE_TRACKERS = L"3. 校准追踪器";
// 	language.CAMERA_START_STEAMVR = L"4. 启动 SteamVR！";
// 	language.CAMERA_CONNECT = L"5. 连接到 SteamVR";
// 	language.CAMERA_START_DETECTION = L"6. 启动或停止追踪器";
// 	language.CAMERA_PREVIEW_CAMERA = L"查看摄像头画面";
// 	language.CAMERA_PREVIEW_CALIBRATION = L"查看校准数据";
// 	language.CAMERA_CALIBRATION_MODE = L"位置校准模式";
// 	language.CAMERA_MULTICAM_CALIB = L"将当前实例的白色摄像头与其它实例的白色摄像头对齐位置";
// 	language.CAMERA_LOCK_HEIGHT = L"在使用手柄校准时锁定Y轴";
// 	language.CAMERA_CALIBRATION_INSTRUCTION = L"禁用 SteamVR 家并退出游戏以看到白色摄像头。\n使用左手柄的扳机抓住白色摄像头，使其与现实生活中的摄像头处于相同位置，然后使用左手柄的抓握键旋转白色摄像头，使其与现实生活中的摄像头处于相同朝向。\n完成校准后取消勾选 \"位置校准模式\" 以保存校准数据！\n\n\n";
// 	language.CAMERA_DISABLE_OUT = L"关闭输出窗口";

// 	language.cameraApiDescriptions = []()
// 	{
// 		std::stringstream description;
// 		description << "0: No preference\n\nCamera backends:";
// 		for (const auto backend : cv::videoio_registry::getCameraBackends())
// 		{
// 			description << "\n" << int(backend) << ": " << cv::videoio_registry::getBackendName(backend);
// 		}
// 		description << "\n\nStream backends:";
// 		for (const auto backend : cv::videoio_registry::getStreamBackends())
// 		{
// 			description << "\n" << int(backend) << ": " << cv::videoio_registry::getBackendName(backend);
// 		}
// 		return description.str();
// 	}();

// 	language.PARAMS_LANGUAGE = L"简体中文";

// 	language.PARAMS_CAMERA = L"摄像头";
// 	language.PARAMS_CAMERA_NAME_ID = L"视频流地址或摄像头 ID";
// 	language.PARAMS_CAMERA_TOOLTIP_ID = L"USB摄像头需要填写数字 ID， 比如 0，1，2，3 这样的数字\nIP-Webcam 需要填写视频流地址，通常是手机的IP加上 \":8080/video\" 比如 \"http://192.168.1.101:8080/video\" ";
// 	language.PARAMS_CAMERA_NAME_API = L"视频捕获 API";
// 	language.PARAMS_CAMERA_TOOLTIP_API = language.cameraApiDescriptions;
// 	language.PARAMS_CAMERA_NAME_ROT_CLOCKWISE = L"顺时针旋转图像";
// 	language.PARAMS_CAMERA_TOOLTIP_ROT_CLOCKWISE = L"将图像顺时针旋转90°，与旁边的选项一起使用会将图像旋转180°";
// 	language.PARAMS_CAMERA_NAME_ROT_CCLOCKWISE = L"逆时针旋转图像";
// 	language.PARAMS_CAMERA_TOOLTIP_ROT_CCLOCKWISE = L"将图像逆时针旋转90°，与旁边的选项一起使用会将图像旋转180°";
// 	language.PARAMS_CAMERA_NAME_WIDTH = L"图像宽度";
// 	language.PARAMS_CAMERA_TOOLTIP_WIDTH = L"图像宽度和图像高度通常会自动进行调整，但如果图像不符合实际的分辨率，请手动填写这两个参数的值。";
// 	language.PARAMS_CAMERA_NAME_HEIGHT = L"图像高度";
// 	language.PARAMS_CAMERA_TOOLTIP_HEIGHT = L"图像宽度和图像高度通常会自动进行调整，但如果图像不符合实际的分辨率，请手动填写这两个参数的值。";
// 	language.PARAMS_CAMERA_NAME_FPS = L"摄像头 FPS";
// 	language.PARAMS_CAMERA_TOOLTIP_FPS = L"尝试设置摄像头的 FPS";
// 	language.PARAMS_CAMERA_NAME_SETTINGS = L"尝试打开摄像头菜单";
// 	language.PARAMS_CAMERA_TOOLTIP_SETTINGS = L"有些摄像头可以勾选此项以打开摄像头菜单。这个选项仅在视频捕获 API 为 DirectShow（700）时才会生效";
// 	language.PARAMS_CAMERA_NAME_3_OPTIONS = L"尝试配置后面的三个参数";
// 	language.PARAMS_CAMERA_TOOLTIP_3_OPTIONS = L"实验性功能。勾选此项将禁用摄像头的自动对焦，并尝试为摄像头配置后面的三个参数。";
// 	language.PARAMS_CAMERA_NAME_AUTOEXP = L"摄像头自动曝光";
// 	language.PARAMS_CAMERA_TOOLTIP_AUTOEXP = L"实验性功能。将尝试设置摄像头的自动曝光功能。大部分 USB 摄像头填写 1 将启用自动曝光， 填写 0 将禁用自动曝光， 但也有些摄像头是 0.75 与 0.25";
// 	language.PARAMS_CAMERA_NAME_EXP = L"摄像头快门速度";
// 	language.PARAMS_CAMERA_TOOLTIP_EXP = L"实验性功能。 将尝试设置摄像头的快门速度。可以 0-255 之间的一个刻度，或者是 2 的指数 ( -8 是 4 毫秒的快门速度)";
// 	language.PARAMS_CAMERA_NAME_GAIN = L"摄像头增益";
// 	language.PARAMS_CAMERA_TOOLTIP_GAIN = L"实验性功能。将尝试设置摄像头的增益。大概是在 0-255 之间的一个刻度，在不同摄像头是不同的值。";

// 	language.PARAMS_TRACKER = L"追踪器";
// 	language.PARAMS_TRACKER_NAME_NUM_TRACKERS = L"追踪器的数量";
// 	language.PARAMS_TRACKER_TOOLTIP_NUM_TRACKERS = L"设置为 3 才能在 VRchat 得到全身追踪！";
// 	language.PARAMS_TRACKER_NAME_MARKER_SIZE = L"边界长度（厘米）";
// 	language.PARAMS_TRACKER_TOOLTIP_MARKER_SIZE = L"\"边界\" 是一个正方形在白色方框与黑块方框的相邻处，以毫米级精度（精确到小数点后一位）测量它的边长并填写到这里。";
// 	language.PARAMS_TRACKER_NAME_QUAD_DECIMATE = L"欠采样倍率";
// 	language.PARAMS_TRACKER_TOOLTIP_QUAD_DECIMATE = L"它可以是 1、1.5、2、3、4 这样的数字。较大的数字将减少每帧所需要的 CPU 时间并优化性能，但会减少二维码被检测到的概率。";
// 	language.PARAMS_TRACKER_NAME_SEARCH_WINDOW = L"搜索范围";
// 	language.PARAMS_TRACKER_TOOLTIP_SEARCH_WINDOW = L"搜索范围的大小。较小的搜索范围将减少每帧所需要的 CPU 时间并优化性能，但会在运动模糊产生时丢失追踪并难以再次找回追踪。";
// 	language.PARAMS_TRACKER_NAME_MARKER_LIBRARY = L"二维码类型";
// 	language.PARAMS_TRACKER_TOOLTIP_MARKER_LIBRARY = L"切换要使用的二维码类型。仅限开发者使用，默认选项为 ApriltagStandard。";
// 	language.PARAMS_TRACKER_NAME_USE_CENTERS = L"追踪器居中生成";
// 	language.PARAMS_TRACKER_TOOLTIP_USE_CENTERS = L"实验性功能。启用此选项以在每组二维码的中央生成追踪器，而不是在主二维码上生成追踪器。";
// 	language.PARAMS_TRACKER_NAME_IGNORE_0 = L"忽略第一个追踪器";
// 	language.PARAMS_TRACKER_TOOLTIP_IGNORE_0 = L"如果想用 vive tracker 或 owotracker 替换掉腰部追踪器，勾选它并保持追踪器的数量为 3。";

// 	language.PARAMS_SMOOTHING = L"平滑算法";
// 	language.PARAMS_SMOOTHING_NAME_WINDOW = L"时间窗口";
// 	language.PARAMS_SMOOTHING_TOOLTIP_WINDOW = L"此时间窗口中的样本将用于线性插值的计算。以秒为单位，越大抖动越少，但会增加延迟并造成更多的粘滞感。最佳值通常处于 0.2 至 0.5 之间。";
// 	language.PARAMS_SMOOTHING_NAME_ADDITIONAL = L"额外平滑";
// 	language.PARAMS_SMOOTHING_TOOLTIP_ADDITIONAL = L"对追踪器的定位结果应用额外的平滑算法。将模仿 0.4 版本中的平滑算法。0 的意思是禁用，1 是最大值。";
// 	language.PARAMS_SMOOTHING_NAME_DEPTH = L"深度平滑";
// 	language.PARAMS_SMOOTHING_TOOLTIP_DEPTH = L"实验性功能。对深度估算的结果应用额外的平滑处理算法，之所以要这样做是因为深度信息拥有更高的误差。以极少的延迟作为代价将不规则抖动稀释为匀速运动。（ 0 表示禁用，建议值为 0 至 1 之间 ）";
// 	language.PARAMS_SMOOTHING_NAME_CAM_LATENCY = L"摄像头延迟（latency）";
// 	language.PARAMS_SMOOTHING_TOOLTIP_CAM_LATENCY = L"以秒为单位的摄像头延迟（ latency ）。配置此参数应该可以应对在使用 IP-Webcam 时的任何延迟。 该参数的值通常应低于 0.1。";

// 	language.PARAMS_HOVER_HELP = L"将鼠标指针移动到参数上查看提示！";
// 	language.PARAMS_SAVE = L"保存";

// 	language.PARAMS_NOTE_LOW_SMOOTHING = L"提示：平滑器的时间窗口太小，这可能会导致问题。\n\n如果你的追踪器出现任何问题，请尝试增加它。";
// 	language.PARAMS_NOTE_QUAD_NONSTANDARD = L"提示：欠采样倍率不是标准值。\n\n保持它为 1、1.5、2、3、4，否则可能会导致追踪器不被定位。";
// 	language.PARAMS_NOTE_NO_DSHOW_CAMSETTINGS = L"提示：启用了\"尝试打开摄像头菜单\"，但视频捕获 API 不是700。\n\n\"尝试打开摄像头菜单\"功能仅在视频捕获 API 为 DirectShow （700）时有效。";
// 	language.PARAMS_NOTE_LATENCY_GREATER_SMOOTHING = L"提示：摄像头延迟（latency）的值永远不应高于 \"时间窗口\" 的值，否则追踪器不会被定位。\n\n您也许应在 摄像头延迟（latency）尝试 0.1 以下的值。除非您的摄像头拥有很快的拍摄速度，否则不建议在 \"时间窗口\" 使用0.2以下的值。";
// 	language.PARAMS_NOTE_HIGH_SMOOTHING = L"提示：\"时间窗口\" 的值大于 1，这将导致很高的延迟！\n\n您也许应尝试像是 0.5 这样的值。";
// 	language.PARAMS_NOTE_2TRACKERS_IGNORE0 = L"追踪器数量为 2 但却启用了 \"忽略第一个追踪器\"。这将导致 SteamVR 中只显示一个腰部追踪器。\n如果您想同时使用双脚上的追踪器，需要将追踪器数量设置为 3。";
// 	language.PARAMS_NOTE_LANGUAGECHANGE = L"语言已更改！请重启这个软件以应用最新设置。（简体中文）";

// 	language.PARAMS_SAVED_MSG = L"参数已保存！";

// 	language.PARAMS_WRONG_VALUES = L"请输入格式正确的参数。由于数据类型转换时发生异常，所有参数将不会保存。";

// 	language.TRACKER_CAMERA_START_ERROR = L"无法启动摄像头，请确保您在参数中输入了正确的视频流地址或摄像头 ID。\n"
// 		L"对于 USB摄像头，它的 ID 通常为 0、1、2 这样的数字，反复尝试直到此错误不再出现。\n"
// 		L"对于 IP-Webcam, 视频流地址的格式为 http://'此处替换为手机的ip':8080/video";
// 	language.TRACKER_CAMERA_ERROR = L"摄像头已启动，但在读取下一帧图像时发生异常。";
// 	language.TRACKER_CAMERA_PREVIEW = L"摄像头视角";

// 	language.TRACKER_CAMERA_NOTRUNNING = L"先启动摄像头才能使用此功能。";

// 	language.TRACKER_CAMERA_CALIBRATION_INSTRUCTIONS = L"开始校准摄像头！\n\n"
// 		L"将打印好的 charuco_board 放在平坦的表面上，或直接在（非曲面）显示器上显示 charuco_board。摄像头将自动拍摄一些 charuco_board 的图像，从尽可能多的不同角度和距离拍摄 charuco_board 的图像。\n\n"
// 		L"确保 charuco_board 清晰可见，并缓慢的移动您的摄像头。\n\n"
// 		L"紫点 = 优秀\n绿点 = 一般\n黄点 = 很差\n\n"
// 		L"这些网格应该笔直，平坦，并铺满整个画面。网格需要保持稳定，但可以在几个像素的范围内有轻微的抖动。\n"
// 		L"这些彩色的点应该均匀地分布在整个画面上。\n\n"
// 		L"当您对校准结果感到满意时，按下确定按钮以保存校准数据。";

// 	language.TRACKER_CAMERA_CALIBRATION_COMPLETE = L"校准成功，数据已保存。";
// 	language.TRACKER_CAMERA_CALIBRATION_NOTDONE = L"校准失败，因为拍摄到的图像太少。";

// 	language.TRACKER_CAMERA_NOTCALIBRATED = L"先校准摄像头才能使用此功能。";

// 	language.TRACKER_TRACKER_CALIBRATION_INSTRUCTIONS = L"开始校准追踪器！ \n\n"
// 		L"在开始校准之前需要正确设置追踪器的数量以及边界长度。确保追踪器的刚性结构并且二维码表面没有弯曲，"
// 		L"二维码或者是二维码之间都不可以有弯曲，它们应该呈现出一个直角。您可以佩戴好追踪器，然后将它们移动到距离摄像头不到 30 厘米的地方以进行校准。\n\n"
// 		L"绿色边框: 这个二维码已被校准，可用于校准其它二维码\n"
// 		L"蓝色边框: 这个二维码的 ID 不属于已启用的 ID。您可能必须在参数页面增加追踪器的数量。\n"
// 		L"紫色边框: 这个二维码距离摄像头太远，将其移动到距离摄像头 30 厘米左右的地方。\n"
// 		L"红色边框: 这个二维码不能被校准，因为在这个追踪器上没有找到已被校准的二维码。 转动这个追踪器，直到有绿色边框的二维码出现。\n"
// 		L"黄色边框: 这个二维码正在被校准，请保持姿势直到它获得绿色边框。\n\n"
// 		L"当所有的二维码都已获得绿色边框时，按下 OK 按钮以保存校准数据。";

// 	language.TRACKER_TRACKER_NOTCALIBRATED = L"无法启动追踪器，因为还没有校准过追踪器。";

// 	language.TRACKER_STEAMVR_NOTCONNECTED = L"无法启动追踪器，因为还没有连接到 SteamVR 。（如果您已经点击过连接到 SteamVR，可能是驱动未正确安装，或是驱动未启用，或是头显出现问题。）";


// 	language.TRACKER_CALIBRATION_SOMETHINGWRONG = L"无法校准追踪器，因为软件内部发生异常。请再试一次。";
// 	language.CONNECT_SOMETHINGWRONG = L"与驱动通信时发生异常。请再试一次。";
// 	language.TRACKER_DETECTION_SOMETHINGWRONG = L"位姿估算时发生异常，重新启动追踪器！\n"
// 		L"如果问题仍然存在，请尝试重新校准摄像头和追踪器";

// 	language.CONNECT_ALREADYCONNECTED = L"已经连接到 SteamVR 了，还需要再连接一次吗？";
// 	language.CONNECT_CLIENT_ERROR = L"当前实例在连接 SteamVR 时出现错误! 确保您的头显已连接。\n错误代码：";
// 	language.CONNECT_DRIVER_ERROR = L"无法连接到安装在 SteamVR 的驱动程序。如果错误代码为 2，确保 SteamVR 正在运行，并且 AprilTagTrackers 的驱动已正确安装，并且在 设置->启动/关闭->管理加载项 中启用了 apriltagtrackers。\n"
// 		L"如果错误代码不是 2，您可能还需要以管理员身份运行此程序。\n错误代码: ";

// 	language.CONNECT_DRIVER_MISSMATCH1 = L"驱动版本和当前的软件版本不匹配！ \n\n驱动版本：";
// 	language.CONNECT_DRIVER_MISSMATCH2 = L"\n应使用的驱动版本：";

// 	return language;
// };

