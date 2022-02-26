#include "Serializable.h"

bool FileStorageSerializable::Save() const
{
    cv::FileStorage fs(file_path, cv::FileStorage::WRITE);
    if (!fs.isOpened()) return false;
    assert(fs.getFormat() == cv::FileStorage::FORMAT_YAML);
    SerializeAll(fs);
    fs.release();
    return true;
}
bool FileStorageSerializable::Load()
{
    cv::FileStorage fs(file_path, cv::FileStorage::READ);
    if (!fs.isOpened()) return false;
    assert(fs.getFormat() == cv::FileStorage::FORMAT_YAML);
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