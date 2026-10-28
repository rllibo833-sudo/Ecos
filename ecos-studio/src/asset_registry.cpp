#include "ecos_studio/asset_registry.hpp"
#include "ecos_studio/platform.hpp"

#include <utility>

namespace ecos::studio {

Result<void> AssetRegistry::register_asset(AssetRecord asset) {
    if (asset.id.empty() || asset.type.empty()) {
        return Result<void>::failure({ErrorCode::invalid_argument, "Asset id and type must not be empty"});
    }
    auto safe_path = platform::safe_relative_path(asset.path);
    if (!safe_path) return Result<void>::failure(safe_path.error());
    if (assets_.find(asset.id) != assets_.end()) {
        return Result<void>::failure({ErrorCode::invalid_argument, "Duplicate asset id: " + asset.id});
    }
    for (const auto& dependency : asset.dependencies) {
        if (dependency.empty() || dependency == asset.id) {
            return Result<void>::failure({ErrorCode::invalid_argument, "Asset dependency must be non-empty and cannot reference itself"});
        }
    }
    asset.path = safe_path.value();
    // Preserve the key independently of the moved value.
    const std::string id = asset.id;
    assets_.emplace(id, std::move(asset));
    return Result<void>::success();
}

const AssetRecord* AssetRegistry::find(const std::string& id) const noexcept {
    const auto it = assets_.find(id);
    return it == assets_.end() ? nullptr : &it->second;
}

Result<void> AssetRegistry::validate_dependencies() const {
    for (const auto& [id, asset] : assets_) {
        for (const auto& dependency : asset.dependencies) {
            if (assets_.find(dependency) == assets_.end()) {
                return Result<void>::failure({ErrorCode::not_found,
                    "Asset '" + id + "' depends on missing asset '" + dependency + "'"});
            }
        }
    }
    return Result<void>::success();
}

std::size_t AssetRegistry::size() const noexcept { return assets_.size(); }
void AssetRegistry::clear() noexcept { assets_.clear(); }

} // namespace ecos::studio
