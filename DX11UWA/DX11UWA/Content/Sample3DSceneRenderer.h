#pragma once

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
		void Render(void);
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

		// To draw additional Cubes
		ModelViewProjectionConstantBuffer m_constantBufferData_2;
		ModelViewProjectionConstantBuffer m_constantBufferData_3;

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
		//                BEGIN PYRAMID MODELS STUFF                  //
		////////////////////////////////////////////////////////////////
		// Test Pyramid 
		Model test_pyramid_model;
		Model test_pyramid_model2;
		Model test_pyramid_model3;

		// Constant Buffer Data
		ModelViewProjectionConstantBuffer m_constantBufferData_pyramid;
		ModelViewProjectionConstantBuffer m_constantBufferData_pyramid2;
		ModelViewProjectionConstantBuffer m_constantBufferData_pyramid3;


		// Lighting 
		DirectionalLight pyramid_directional_Light;
		PointLight pyramid_point_light;
		////////////////////////////////////////////////////////////////
		//                  END PYRAMID MODELS STUFF                  //
		////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////
		//                BEGIN BIG DADDY MODELS STUFF                //
		////////////////////////////////////////////////////////////////
		// Big Daddy
		Model big_daddy_model;
		ModelViewProjectionConstantBuffer m_constantBufferData_big_daddy;

		// Texture Variables
		Microsoft::WRL::ComPtr<ID3D11Texture2D> diffuseTexture;
		////////////////////////////////////////////////////////////////
		//                  END BIG DADDY MODELS STUFF                //
		////////////////////////////////////////////////////////////////

	};
}

