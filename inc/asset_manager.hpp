#pragma once
#include <vector>
#include "asset.hpp"

class AssetManager {
public:
    void addAsset(const Asset& asset) { m_assets.push_back(asset); }
    const std::vector<Asset>& getAssets() const { return m_assets; }
    
    void clear() { m_assets.clear(); }
    size_t size() const { return m_assets.size(); }
    bool empty() const { return m_assets.empty(); }

private:
    std::vector<Asset> m_assets;
};
