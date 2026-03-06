#pragma once
#include <unordered_map>
#include <memory>
#include <string>

#include "ModelType.h"

class InterleavedGltfModel;

class AssetManager
{
public:
    static AssetManager& Get();

    std::shared_ptr<InterleavedGltfModel> LoadModel(ID3D11Device* device, const std::string& filename, ModelTypes::ModelMode mode, bool isSaveVerticesData = false);
private:

    std::unordered_map<std::string,std::weak_ptr<InterleavedGltfModel>> modelCache;
};