#include "Serializable.h"

bool FileStorageSerializable::Save() const
{
    cv::FileStorage fs(file_path, cv::FileStorage::WRITE);
    if (!fs.isOpened()) return false;
    SerializeAll(fs);
    fs.release();
    return true;
}
bool FileStorageSerializable::Load()
{
    cv::FileStorage fs(file_path, cv::FileStorage::READ);
    if (!fs.isOpened()) return false;
    DeserializeAll(fs.root());
    fs.release();
    return true;
}

void FileStorageSerializable::SerializeAll(cv::FileStorage& fs) const
{
    for (const auto& field : fields)
        field.get().Serialize(fs);
}
void FileStorageSerializable::DeserializeAll(const cv::FileNode& fn)
{
    for (const auto& field : fields)
        field.get().Deserialize(fn);
}

template<>
void FileStorageField<cv::Ptr<cv::aruco::DetectorParameters>>::Serialize(cv::FileStorage& fs) const
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
