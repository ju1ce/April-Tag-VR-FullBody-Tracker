#include "CustomSerial.hpp"

void serial::Serial<cv::Ptr<cv::aruco::DetectorParameters>>::Format(FileStorageWriter& ctx, const cv::Ptr<cv::aruco::DetectorParameters>& value)
{
    // Aruco parameters.
    ctx.WriteKey("adaptiveThreshWinSizeMin", value->adaptiveThreshWinSizeMin);
    ctx.WriteKey("adaptiveThreshWinSizeMax", value->adaptiveThreshWinSizeMax);
    ctx.WriteKey("adaptiveThreshWinSizeStep", value->adaptiveThreshWinSizeStep);
    ctx.WriteKey("adaptiveThreshConstant", value->adaptiveThreshConstant);
    ctx.WriteKey("minMarkerPerimeterRate", value->minMarkerPerimeterRate);
    ctx.WriteKey("maxMarkerPerimeterRate", value->maxMarkerPerimeterRate);
    ctx.WriteKey("polygonalApproxAccuracyRate", value->polygonalApproxAccuracyRate);
    ctx.WriteKey("minCornerDistanceRate", value->minCornerDistanceRate);
    ctx.WriteKey("minDistanceToBorder", value->minDistanceToBorder);
    ctx.WriteKey("minMarkerDistanceRate", value->minMarkerDistanceRate);
    ctx.WriteKey("cornerRefinementMethod", value->cornerRefinementMethod);
    ctx.WriteKey("cornerRefinementWinSize", value->cornerRefinementWinSize);
    ctx.WriteKey("cornerRefinementMaxIterations", value->cornerRefinementMaxIterations);
    ctx.WriteKey("cornerRefinementMinAccuracy", value->cornerRefinementMinAccuracy);
    ctx.WriteKey("markerBorderBits", value->markerBorderBits);
    ctx.WriteKey("perspectiveRemovePixelPerCell", value->perspectiveRemovePixelPerCell);
    ctx.WriteKey("perspectiveRemoveIgnoredMarginPerCell", value->perspectiveRemoveIgnoredMarginPerCell);
    ctx.WriteKey("maxErroneousBitsInBorderRate", value->maxErroneousBitsInBorderRate);
    ctx.WriteKey("minOtsuStdDev", value->minOtsuStdDev);
    ctx.WriteKey("errorCorrectionRate", value->errorCorrectionRate);

    // April :: User-configurable parameters.
    ctx.WriteKey("aprilTagQuadDecimate", value->aprilTagQuadDecimate);
    ctx.WriteKey("aprilTagQuadSigma", value->aprilTagQuadSigma);

    // April :: Internal variables
    ctx.WriteKey("aprilTagMinClusterPixels", value->aprilTagMinClusterPixels);
    ctx.WriteKey("aprilTagMaxNmaxima", value->aprilTagMaxNmaxima);
    ctx.WriteKey("aprilTagCriticalRad", value->aprilTagCriticalRad);
    ctx.WriteKey("aprilTagMaxLineFitMse", value->aprilTagMaxLineFitMse);
    ctx.WriteKey("aprilTagMinWhiteBlackDiff", value->aprilTagMinWhiteBlackDiff);
    ctx.WriteKey("aprilTagDeglitch", value->aprilTagDeglitch);

    // to detect white (inverted) markers
    ctx.WriteKey("detectInvertedMarker", value->detectInvertedMarker);
}
