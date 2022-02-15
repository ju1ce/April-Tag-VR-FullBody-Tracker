#include "ParamSerializer.h"

UserConfigStorage::UserConfigStorage()
    : ConfigStorage("config.yaml")
{

}

CalibrationStorage::CalibrationStorage()
    : ConfigStorage("calibration.yaml")
{

}

void ParamSerializer::Save(const std::string &file_path) const
{
    // Force yaml because we assume FileNode::FLOW structures.
    // Encoding arg doesn't do anything for yaml.
    cv::FileStorage fs(file_path, cv::FileStorage::WRITE);
    if (!fs.isOpened()) return;

    assert(fs.getFormat() == cv::FileStorage::FORMAT_YAML);

    for (const auto &p : params)
    {
        fs << p.first;
        p.second->Serialize(fs);
    }

    fs.release();
}

void ParamSerializer::Load(const std::string &file_path)
{
    cv::FileStorage fs(file_path, cv::FileStorage::READ);
    if (!fs.isOpened()) return;

    assert(fs.getFormat() == cv::FileStorage::FORMAT_YAML);

    for (const auto &p : params)
    {
        p.second->Deserialize(fs[p.first]);
    }

    fs.release();
}