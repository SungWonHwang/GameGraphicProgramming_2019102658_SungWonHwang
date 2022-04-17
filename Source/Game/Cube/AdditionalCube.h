#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"

class AdditionalCube : public BaseCube
{
public:
    AdditionalCube(const std::filesystem::path& textureFilePath);
    AdditionalCube(const AdditionalCube& other) = delete;
    AdditionalCube(AdditionalCube&& other) = delete;
    AdditionalCube& operator=(const AdditionalCube& other) = delete;
    AdditionalCube& operator=(AdditionalCube&& other) = delete;
    ~AdditionalCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};