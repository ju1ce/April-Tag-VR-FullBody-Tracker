#include "ReaderWriter.hpp"

template <>
void serial::Write(serial::Writer& writer, const std::string&, const cv::Ptr<cv::aruco::DetectorParameters>& value)
{
    // Aruco parameters.
    writer << "adaptiveThreshWinSizeMin" << value->adaptiveThreshWinSizeMin;
    writer << "adaptiveThreshWinSizeMax" << value->adaptiveThreshWinSizeMax;
    writer << "adaptiveThreshWinSizeStep" << value->adaptiveThreshWinSizeStep;
    writer << "adaptiveThreshConstant" << value->adaptiveThreshConstant;
    writer << "minMarkerPerimeterRate" << value->minMarkerPerimeterRate;
    writer << "maxMarkerPerimeterRate" << value->maxMarkerPerimeterRate;
    writer << "polygonalApproxAccuracyRate" << value->polygonalApproxAccuracyRate;
    writer << "minCornerDistanceRate" << value->minCornerDistanceRate;
    writer << "minDistanceToBorder" << value->minDistanceToBorder;
    writer << "minMarkerDistanceRate" << value->minMarkerDistanceRate;
    writer << "cornerRefinementMethod" << value->cornerRefinementMethod;
    writer << "cornerRefinementWinSize" << value->cornerRefinementWinSize;
    writer << "cornerRefinementMaxIterations" << value->cornerRefinementMaxIterations;
    writer << "cornerRefinementMinAccuracy" << value->cornerRefinementMinAccuracy;
    writer << "markerBorderBits" << value->markerBorderBits;
    writer << "perspectiveRemovePixelPerCell" << value->perspectiveRemovePixelPerCell;
    writer << "perspectiveRemoveIgnoredMarginPerCell" << value->perspectiveRemoveIgnoredMarginPerCell;
    writer << "maxErroneousBitsInBorderRate" << value->maxErroneousBitsInBorderRate;
    writer << "minOtsuStdDev" << value->minOtsuStdDev;
    writer << "errorCorrectionRate" << value->errorCorrectionRate;

    // April :: User-configurable parameters.
    writer << "aprilTagQuadDecimate" << value->aprilTagQuadDecimate;
    writer << "aprilTagQuadSigma" << value->aprilTagQuadSigma;

    // April :: Internal variables
    writer << "aprilTagMinClusterPixels" << value->aprilTagMinClusterPixels;
    writer << "aprilTagMaxNmaxima" << value->aprilTagMaxNmaxima;
    writer << "aprilTagCriticalRad" << value->aprilTagCriticalRad;
    writer << "aprilTagMaxLineFitMse" << value->aprilTagMaxLineFitMse;
    writer << "aprilTagMinWhiteBlackDiff" << value->aprilTagMinWhiteBlackDiff;
    writer << "aprilTagDeglitch" << value->aprilTagDeglitch;

    // to detect white (inverted) markers
    writer << "detectInvertedMarker" << value->detectInvertedMarker;
}
