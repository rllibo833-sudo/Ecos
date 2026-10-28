#pragma once

#include "ecos_studio/result.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace ecos::studio {

struct AssetRecord {
    std::string id;
    std::string type;
    std::filesystem::path path;
    std::vector<std::string> dependencies;
};

class AssetRegistry {
public:
    [[nodiscard]] Result<void> register_asset(AssetRecord asset);
    [[nodiscard]] const AssetRecord* find(const std::string& id) const noexcept;
    [[nodiscard]] Result<void> validate_dependencies() const;
    [[nodiscard]] std::size_t size() const noexcept;
    void clear() noexcept;

private:
    std::unordered_map<std::string, AssetRecord> assets_;
};

} // namespace ecos::studio
