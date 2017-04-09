#pragma once
#include <vector>

struct Model
{
	// For rendering
	bool _loadingComplete;

	// Index Count
	uint32	_indexCount;

	// Direct 3D resources for the model
	// ComPtr are safe to share
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		_constantBuffer;

	// Stuff that individuals models will have
	std::vector<DirectX::XMFLOAT3>				_vertices;
	std::vector<DirectX::XMFLOAT2>				_uvs;
	std::vector<DirectX::XMFLOAT3>				_normals;

	// Path to the texture if it exists
	const char 									*_texture_path;

	// The World Matrix
	DirectX::XMMATRIX							_world_matrix;
};

struct DirectionalLight {
	DirectX::XMFLOAT3 direction;
	DirectX::XMFLOAT3 color;
};

struct PointLight {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;
	float radius;
};

struct SpotLight {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;
	DirectX::XMFLOAT3 cone_direction;
	float cone_ratio;
	float inner_cone_ratio;
	float outer_cone_ratio;
};