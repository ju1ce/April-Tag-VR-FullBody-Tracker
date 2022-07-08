#include "Serializable.hpp"

namespace FS
{

void SerializeField(cv::FileStorage& fs, const char*, const cv::Ptr<cv::aruco::DetectorParameters>& field)
{
    // Aruco parameters.
    fs << "adaptiveThreshWinSizeMin" << field->adaptiveThreshWinSizeMin;
    fs << "adaptiveThreshWinSizeMax" << field->adaptiveThreshWinSizeMax;
    fs << "adaptiveThreshWinSizeStep" << field->adaptiveThreshWinSizeStep;
    fs << "adaptiveThreshConstant" << field->adaptiveThreshConstant;
    fs << "minMarkerPerimeterRate" << field->minMarkerPerimeterRate;
    fs << "maxMarkerPerimeterRate" << field->maxMarkerPerimeterRate;
    fs << "polygonalApproxAccuracyRate" << field->polygonalApproxAccuracyRate;
    fs << "minCornerDistanceRate" << field->minCornerDistanceRate;
    fs << "minDistanceToBorder" << field->minDistanceToBorder;
    fs << "minMarkerDistanceRate" << field->minMarkerDistanceRate;
    fs << "cornerRefinementMethod" << field->cornerRefinementMethod;
    fs << "cornerRefinementWinSize" << field->cornerRefinementWinSize;
    fs << "cornerRefinementMaxIterations" << field->cornerRefinementMaxIterations;
    fs << "cornerRefinementMinAccuracy" << field->cornerRefinementMinAccuracy;
    fs << "markerBorderBits" << field->markerBorderBits;
    fs << "perspectiveRemovePixelPerCell" << field->perspectiveRemovePixelPerCell;
    fs << "perspectiveRemoveIgnoredMarginPerCell" << field->perspectiveRemoveIgnoredMarginPerCell;
    fs << "maxErroneousBitsInBorderRate" << field->maxErroneousBitsInBorderRate;
    fs << "minOtsuStdDev" << field->minOtsuStdDev;
    fs << "errorCorrectionRate" << field->errorCorrectionRate;

    // April :: User-configurable parameters.
    fs << "aprilTagQuadDecimate" << field->aprilTagQuadDecimate;
    fs << "aprilTagQuadSigma" << field->aprilTagQuadSigma;

    // April :: Internal variables
    fs << "aprilTagMinClusterPixels" << field->aprilTagMinClusterPixels;
    fs << "aprilTagMaxNmaxima" << field->aprilTagMaxNmaxima;
    fs << "aprilTagCriticalRad" << field->aprilTagCriticalRad;
    fs << "aprilTagMaxLineFitMse" << field->aprilTagMaxLineFitMse;
    fs << "aprilTagMinWhiteBlackDiff" << field->aprilTagMinWhiteBlackDiff;
    fs << "aprilTagDeglitch" << field->aprilTagDeglitch;

    // to detect white (inverted) markers
    fs << "detectInvertedMarker" << field->detectInvertedMarker;
}

};
