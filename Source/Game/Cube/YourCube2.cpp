#include "Cube/YourCube2.h"

void YourCube2::Update(_In_ FLOAT t)
{
    m_world = XMMatrixRotationY(t);
}