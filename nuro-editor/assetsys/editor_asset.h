#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <utils/fsutil.h>

#include "asset_meta.h"

class ProjectAssets;

enum class AssetType {
	FALLBACK,

	ANIMATION,
	AUDIO_CLIP,
	FONT,
	MATERIAL,
	MODEL,
	TEXTURE
};

class EditorAsset {
private:
	friend class ProjectAssets;

	// Assets id
	AssetKey _assetKey;

	// Type of the asset
	AssetType _assetType;

	// Path to asset relative to project root
	FS::Path _assetPath;

	// List of assets this asset depends on
	std::vector<AssetGUID> _assetDependencies;

	// If asset is currently loading
	bool _assetLoading;

protected:
	EditorAsset() : _assetKey(), _assetType(AssetType::FALLBACK), _assetPath(), _assetDependencies(), _assetLoading(false) {};

public:
	virtual ~EditorAsset() = 0;

	// Returns the editor assets key
	AssetKey key() const {
		return _assetKey;
	}

	// Returns the type of the asset
	AssetType type() const {
		return _assetType;
	}

	// Returns the path to the asset relative to project root path
	FS::Path path() const {
		return _assetPath;
	}

	// Returns the editor assets dependencies
	const auto& dependencies() const {
		return _assetDependencies;
	}

	// Returns if the asset is currently in a loading state
	bool loading() const {
		return _assetLoading;
	}

	// Event when the asset is first loaded within the editor
	virtual void onDefaultLoad(const FS::Path& metaPath) = 0;

	// Event when the asset is unloaded or destroyed within the editor
	virtual void onUnload() = 0;

	// Event when the asset is reloaded, e.g. due to file modifications
	virtual void onReload() = 0;

	// Renders the assets insight panel inspectable ui
	virtual void renderInspectableUI() = 0;

	// Returns the assets icon
	virtual uint32_t icon() const = 0;
};

inline EditorAsset::~EditorAsset() {}