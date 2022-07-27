// OpenCV
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/affine.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/core/quaternion.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/registry.hpp>

// OpenVR
#include <openvr.h>

// std
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <vector>

#ifdef OS_WIN
#    define NOMINMAX
#    define WIN32_LEAN_AND_MEAN
#    include "Windows.h"
#    undef NOMINMAX
#    undef WIN32_LEAN_AND_MEAN
#endif
