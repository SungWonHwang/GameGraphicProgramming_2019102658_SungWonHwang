#pragma once
#include "Common.h"
#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class AdditionalCube : public BaseCube
{
public:
	AdditionalCube() = default;
	~AdditionalCube() = default;

	virtual void Update(FLOAT deltaTime) override;
};
