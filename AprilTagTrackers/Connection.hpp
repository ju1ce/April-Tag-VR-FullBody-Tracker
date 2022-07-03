#include "Config.hpp"
#include "Helpers.hpp"
#include "IPC/IPC.hpp"

#include <openvr.h>

#include <memory>

struct TrackerConnection
{
    int TrackerId;
    int DriverId;
    std::string Name;
    std::string Role;
};

/// 3d vector position and quaternion translation
/// right handed system, -x right, +y up, +z forward
struct Pose
{
    Pose(const cv::Vec3d& pos, const cv::Quatd& rot)
        : position(pos), rotation(rot)
    {
        ATASSERT("Pose rotation is a unit quaternion.", rotation.isNormal());
    }

    /// When multiplied, applies no translation or rotation
    static Pose Ident() { return {{0, 0, 0}, {1, 0, 0, 0}}; }

    cv::Vec3d position;
    /// unit quaternion
    cv::Quatd rotation;
};

class Connection
{
public:
    enum ConnectionStatus
    {
        DISCONNECTED,
        WAITING,
        CONNECTED
    };

    enum ErrorCode
    {
        OK = 0,
        ALREADY_WAITING,
        ALREADY_CONNECTED,
        BINDINGS_MISSING,
        CLIENT_ERROR,
        DRIVER_ERROR,
        DRIVER_MISMATCH,
        SOMETHING_WRONG,
    };

    Connection(const UserConfig& user_config);
    void StartConnection();
    std::istringstream Send(const std::string& buffer);
    std::istringstream SendTracker(int id, double a, double b, double c, double qw, double qx, double qy, double qz, double time, double smoothing);
    std::istringstream SendStation(int id, double a, double b, double c, double qw, double qx, double qy, double qz);
    Pose GetControllerPose();
    /// Returns non-zero on the frame the button is pressed
    int GetButtonStates();
    /// returns true if ovr quit
    bool PollQuitEvent();

    ConnectionStatus status = DISCONNECTED;
    vr::IVRSystem* openvr_handle = nullptr;
    std::vector<TrackerConnection> connectedTrackers;

    // Basic error handling across threads, not synchronized yet.
    ErrorCode GetAndResetErrorState()
    {
        ErrorCode code = errorCode;
        errorCode = ErrorCode::OK;
        return code;
    }
    const std::string& GetErrorMsg() { return errorMsg; }

private:
    void Connect();

    void SetError(ErrorCode code, std::string msg = "");

    ErrorCode errorCode = ErrorCode::OK;
    std::string errorMsg;

    const UserConfig& user_config;
    std::unique_ptr<IPC::IClient> bridge_driver;

    vr::VRActionHandle_t m_actionCamera = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionsetDemo = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionTrackers = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionHand = vr::k_ulInvalidActionHandle;
};
