#pragma once

#include <string>
#include <optional>

class FileManager {
public:
    explicit FileManager(std::string filename);
    virtual ~FileManager() = default;

    std::optional<std::string> readFile() const;
    bool writeFile(const std::string& content) const;
    bool deleteFile() const;

protected:
    std::string filename;
};
