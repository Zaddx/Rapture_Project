#pragma once
#include <vector>
#include "Content\ShaderStructures.h"

float Clamp(float _val, float _max, float _min);

DirectX::XMFLOAT3 Lerp(DirectX::XMFLOAT3 _c1, DirectX::XMFLOAT3 _c2, float ratio);

float Vector_Dot(DirectX::XMFLOAT3 v, DirectX::XMFLOAT3 w);

DirectX::XMFLOAT3 Vector_Scalar_Multiply(DirectX::XMFLOAT3 v, float s);

bool loadOBJ(const char * path, std::vector<DX11UWA::VertexPositionColor> &out_vertices, std::vector<unsigned int> &out_indices);
