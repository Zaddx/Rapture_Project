#pragma once
#include <vector>

bool loadOBJ(const char * path, std::vector<DirectX::XMFLOAT3> &out_vertices, std::vector<DirectX::XMFLOAT2> &out_uvs, std::vector<DirectX::XMFLOAT3> &out_normals);