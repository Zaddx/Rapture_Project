﻿#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

// My Header Files
#include "ObjLoader.h"
#include "Structures.h"

// Texture header file
#include "DDSTextureLoader.h"

namespace DX11UWA
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources(void);
		void CreateWindowSizeDependentResources(void);
		void ReleaseDeviceDependentResources(void);
		void Update(DX::StepTimer const& timer);
		void Render(DirectX::XMFLOAT4X4 view_matrix);
		void StartTracking(void);
		void TrackingUpdate(float positionX);
		void StopTracking(void);
		inline bool IsTracking(void) { return m_tracking; }

		// Helper functions for keyboard and mouse input
		void SetKeyboardButtons(const char* list);
		void SetMousePosition(const Windows::UI::Input::PointerPoint^ pos);
		void SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos);


	private:
		void Rotate(float radians);
		void UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd);
		void UpdateLights();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// Texture Variables
		Microsoft::WRL::ComPtr<ID3D11Resource> skybox_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyboxSRV;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		// Data members for keyboard and mouse input
		char	m_kbuttons[256];
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;

		// My Model Variables/Resources
		////////////////////////////////////////////////////////////////
		//                BEGIN BIG DADDY MODELS STUFF                //
		////////////////////////////////////////////////////////////////
		// Big Daddy
		Model big_daddy_model;
		ModelViewProjectionConstantBuffer m_constantBufferData_big_daddy;

		// Texture Variables
		Microsoft::WRL::ComPtr<ID3D11Resource> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bigDaddyMeshSRV;
		////////////////////////////////////////////////////////////////
		//                  END BIG DADDY MODELS STUFF                //
		////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////
		//                  BEGIN FLOOR MODEL STUFF                   //
		////////////////////////////////////////////////////////////////
		// Floor
		Model floor_model;
		ModelViewProjectionConstantBuffer m_constantBufferData_floor;

		// Temporary Variables to use for updating
		std::vector<DX11UWA::VertexPositionUVNormal> floor_vertices_updater;

		// Lights
		DirectionalLight floor_directional_light;
		PointLight floor_point_light;
		SpotLight floor_spot_light;

		// Light Constant Buffers & data
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer_pointLight;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer_directionalLight;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer_spotLight;

		// Light Movement Variables
		float y_inc_dir;
		float x_inc_point;
		float x_inc_spot_pos;
		float z_inc_spot_pos;
		float x_inc_spot_dir;
		////////////////////////////////////////////////////////////////
		//                  BEGIN FLOOR MODEL STUFF                   //
		////////////////////////////////////////////////////////////////

	};
}

