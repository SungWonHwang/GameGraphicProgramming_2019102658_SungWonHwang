#pragma once
#include "Common.h"
#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class YourCube : public BaseCube
{
public:
	YourCube() = default;
	~YourCube() = default;

	virtual void Update(FLOAT deltaTime) override;
};