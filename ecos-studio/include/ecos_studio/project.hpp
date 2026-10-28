#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ecos::studio {

struct ValidationResult {
    bool ok{true};
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

bool create_project(const std::filesystem::path& root, const std::string& name, std::string& error);
ValidationResult validate_project(const std::filesystem::path& root);
bool create_mod(const std::filesystem::path& root, const std::string& id, std::string& error);
ValidationResult validate_mod(const std::filesystem::path& root);

} // namespace ecos::studio
