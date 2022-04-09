#pragma once
#include "Common.h"
#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class YourCube2 : public BaseCube
{
public:
	YourCube2() = default;
	~YourCube2() = default;

	virtual void Update(FLOAT deltaTime) override;
};