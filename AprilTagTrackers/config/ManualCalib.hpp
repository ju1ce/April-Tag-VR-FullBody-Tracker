#pragma once

#include "Helpers.hpp"
#include "serial/Comment.hpp"
#include "utils/Reflectable.hpp"
#include "Validated.hpp"

#include <opencv2/core.hpp>

namespace cfg
{

class ManualCalib
{
private:
    /// meters to centimeters
    static constexpr double M_2_CM = 100;

public:
    /// Real representation of calib values, rather than what is displayed in gui and config
    struct Real
    {
        /// meters
        cv::Vec3d posOffset;
        /// (pitch, yaw, roll) in radians
        cv::Vec3d angleOffset;
        /// scale multiplier
        double scale;
    };

    Real GetAsReal() const
    {
        return {
            posOffset / M_2_CM,
            angleOffset * DEG_2_RAD,
            scale / 100.0};
    }
    void SetFromReal(const Real& real)
    {
        posOffset = real.posOffset * M_2_CM;
        angleOffset = real.angleOffset * RAD_2_DEG;
        scale = real.scale * 100.0;
    }

    REFLECTABLE_BEGIN;
    ATT_SERIAL_COMMENT("centimeters");
    REFLECTABLE_FIELD(cv::Vec3d, posOffset);
    ATT_SERIAL_COMMENT("(pitch, yaw, roll) in degrees");
    REFLECTABLE_FIELD(cv::Vec3d, angleOffset);
    ATT_SERIAL_COMMENT("percentage");
    REFLECTABLE_FIELD(cfg::Validated<double>, scale){100.0, cfg::Clamp(80.0, 120.0)};
    REFLECTABLE_END;
};

} // namespace cfg
