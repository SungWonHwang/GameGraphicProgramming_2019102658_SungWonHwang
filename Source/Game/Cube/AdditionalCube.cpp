#include "Cube/AdditionalCube.h"

void AdditionalCube::Update(_In_ FLOAT t)
{
    XMMATRIX mSpin = XMMatrixRotationY(t);
    XMMATRIX mOrbit = XMMatrixRotationZ(-t * 2.0f);
    XMMATRIX mTranslate = XMMatrixTranslation(3.0f, 10.0f, 10.0f);
    XMMATRIX mScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);

    m_world = mScale * mSpin * mTranslate * mOrbit;
}