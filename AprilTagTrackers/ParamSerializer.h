#pragma once

#include "Parameter.h"

// Write and read a ParamNode to a file
class ParamSerializer : public ParamNode
{
protected:
    void Save(const std::string &file_path) const;
    void Load(const std::string &file_path);
};

// Simple single file storage
class ConfigStorage : public ParamSerializer
{
public:
    ConfigStorage(const std::string &file_path)
        : file_path(file_path){};

    void Save() const { ParamSerializer::Save(file_path); }
    void Load() { ParamSerializer::Load(file_path); }

protected:
    std::string file_path;
};

// User editable storage
class UserConfigStorage : public ConfigStorage
{
public:
    UserConfigStorage();
};

// Non-user editable calibration data, long lists of numbers.
// Potentially store this in a file.yaml.gz to reduce size, 
//  and show that it is not user editable. FileStorage has this built in
class CalibrationStorage : public ConfigStorage
{
public:
    CalibrationStorage();
};

// Stores a list of paths to each translation file, mapping the name to a node for each
class LocaleStore : public ConfigStorage
{
public:
    LocaleStore(const std::string& lang_id);
    
    //void Save() const = delete;
};