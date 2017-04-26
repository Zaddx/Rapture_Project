#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	// Update constant buffer to be in Perspective Space (I think)
	XMStoreFloat4x4(&m_constantBufferData.projection, (perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_constantBufferData_big_daddy.projection, (perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_constantBufferData_floor.projection, (perspectiveMatrix * orientationMatrix));


	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));

	// Update the constant buffer data based on camera
	XMStoreFloat4x4(&m_constantBufferData.view, (XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData_big_daddy.view, (XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData_floor.view, (XMMatrixLookAtLH(eye, at, up)));

}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}


	// Update or move camera here
	UpdateCamera(timer, 1.0f, 0.75f);

	// Update lights
	// Update the directional light
	y_inc_dir = timer.GetElapsedSeconds();
	float directional_light_boundaries = 5.0f;

	if (floor_directional_light.direction.y >= directional_light_boundaries)
	{
		floor_directional_light.direction.y = directional_light_boundaries;
		y_inc_dir *= -1.0f;
	}
	if (floor_directional_light.direction.y <= -directional_light_boundaries)
	{
		floor_directional_light.direction.y = -directional_light_boundaries;
		y_inc_dir *= -1.0f;
	}

	floor_directional_light.direction.y += y_inc_dir;

	// Point Light
	x_inc_point = timer.GetElapsedSeconds();
	float point_light_boundaries = 4.0f;

	// Update the position of the point light
	if (floor_point_light.position.x >= point_light_boundaries)
	{
		floor_point_light.position.x = point_light_boundaries;
		x_inc_point *= -1.0f;
	}

	if (floor_point_light.position.x <= -point_light_boundaries)
	{
		floor_point_light.position.x = -point_light_boundaries;
		x_inc_point *= -1.0f;
	}

	floor_point_light.position.x += x_inc_point;

	// Update the position of the spot light
	x_inc_spot_pos = timer.GetElapsedSeconds();
	z_inc_spot_pos = timer.GetElapsedSeconds();
	x_inc_spot_dir = timer.GetElapsedSeconds();

	if (floor_spot_light.position.x >= 0.25f || floor_spot_light.position.x <= -0.25f)
		x_inc_spot_pos *= -1.0f;
	if (floor_spot_light.position.z >= 0.25f || floor_spot_light.position.z <= -0.25f)
		z_inc_spot_pos *= -1.0f;

	if (floor_spot_light.cone_direction.x >= 0.25f || floor_spot_light.cone_direction.x <= -0.25f)
		x_inc_spot_dir *= -1.0f;

	floor_spot_light.position.x += x_inc_spot_pos;
	floor_spot_light.position.z += z_inc_spot_pos;
	floor_spot_light.cone_direction.x += x_inc_spot_dir;

	// Update the Lights
	UpdateLights();
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, (XMMatrixRotationY(0)));
	XMStoreFloat4x4(&m_constantBufferData.model, (XMMatrixTranslation(0.0f, 10.0f, 0.0f)));


	// Translate the position (Big Daddy)
	XMMATRIX bigDaddy_rotationY = XMMatrixRotationY(radians);
	XMMATRIX bigDaddy_translationY = XMMatrixTranslation(0.0f, 10.0f, 0.0f);

	XMStoreFloat4x4(&m_constantBufferData_big_daddy.model, XMMatrixMultiply(bigDaddy_rotationY, bigDaddy_translationY));

}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}

	if (m_currMousePos) 
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}


}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(DirectX::XMFLOAT4X4 view_matrix)
{
	m_constantBufferData.view = view_matrix;
	m_constantBufferData_big_daddy.view = view_matrix;
	m_constantBufferData_floor.view = view_matrix;

	auto context = m_deviceResources->GetD3DDeviceContext();

#pragma region Skybox

	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}


	// Setup the Cubemap
	ID3D11ShaderResourceView** skyboxViews[] = { skyboxSRV.GetAddressOf() };
	context->PSSetShaderResources(0, 1, *skyboxViews);

	XMStoreFloat4x4(&m_constantBufferData.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);

#pragma endregion

#pragma region Big Daddy Model

	//if (!big_daddy_model._loadingComplete)
	//{
	//	return;
	//}

	//ID3D11ShaderResourceView** texViews[] = { bigDaddyMeshSRV.GetAddressOf() };
	//context->PSSetShaderResources(0, 1, *texViews);

	//XMStoreFloat4x4(&m_constantBufferData_big_daddy.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	//// Setup Vertex Buffer
	//UINT bigDaddy_stride = sizeof(DX11UWA::VertexPositionUVNormal);
	//UINT bigDaddy_offset = 0;
	//context->IASetVertexBuffers(0, 1, big_daddy_model._vertexBuffer.GetAddressOf(), &bigDaddy_stride, &bigDaddy_offset);

	//// Set Index buffer
	//context->IASetIndexBuffer(big_daddy_model._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//context->IASetInputLayout(big_daddy_model._inputLayout.Get());

	//context->UpdateSubresource1(big_daddy_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_big_daddy, 0, 0, 0);

	//// Attach our vertex shader.
	//context->VSSetShader(big_daddy_model._vertexShader.Get(), nullptr, 0);

	//// Attach our pixel shader.
	//context->PSSetShader(big_daddy_model._pixelShader.Get(), nullptr, 0);

	//context->DrawIndexed(big_daddy_model._indexCount, 0, 0);

#pragma endregion

#pragma region Floor

	if (!floor_model._loadingComplete)
	{
		return;
	}

	XMStoreFloat4x4(&m_constantBufferData_floor.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Setup Vertex Buffer
	UINT floor_stride = sizeof(DX11UWA::VertexPositionUVNormal);
	UINT floor_offset = 0;
	context->IASetVertexBuffers(0, 1, floor_model._vertexBuffer.GetAddressOf(), &floor_stride, &floor_offset);

	// Set Index buffer
	context->IASetIndexBuffer(floor_model._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(floor_model._inputLayout.Get());

	context->UpdateSubresource1(floor_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_floor, 0, 0, 0);

	// Update subresources for the lights
	context->UpdateSubresource1(m_constantBuffer_pointLight.Get(), 0, NULL, &floor_point_light, 0, 0, 0);
	context->UpdateSubresource1(m_constantBuffer_directionalLight.Get(), 0, NULL, &floor_directional_light, 0, 0, 0);
	context->UpdateSubresource1(m_constantBuffer_spotLight.Get(), 0, NULL, &floor_spot_light, 0, 0, 0);

	// Set the light constant buffers to the floor
	context->PSSetConstantBuffers1(0, 1, m_constantBuffer_pointLight.GetAddressOf(), nullptr, nullptr);
	context->PSSetConstantBuffers1(1, 1, m_constantBuffer_directionalLight.GetAddressOf(), nullptr, nullptr);
	context->PSSetConstantBuffers1(2, 1, m_constantBuffer_spotLight.GetAddressOf(), nullptr, nullptr);

	// Attach our vertex shader.
	context->VSSetShader(floor_model._vertexShader.Get(), nullptr, 0);

	// Attach our pixel shader.
	context->PSSetShader(floor_model._pixelShader.Get(), nullptr, 0);

	context->DrawIndexed(floor_model._indexCount, 0, 0);

#pragma endregion

}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SkyboxVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SkyboxPixelShader.cso");

	auto loadVSTaskTexture = DX::ReadDataAsync(L"TextureVertexShader.cso");
	auto loadPSTaskTexture = DX::ReadDataAsync(L"TexturePixelShader.cso");

	auto LoadVSTaskModel = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto LoadPSTaskModel = DX::ReadDataAsync(L"SamplePixelShader.cso");

#pragma region Floor

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTaskFloorModel = LoadVSTaskModel.then([this](const std::vector<byte>& floor_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&floor_fileData[0], floor_fileData.size(), nullptr, &floor_model._vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC floor_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(floor_vertexDesc, ARRAYSIZE(floor_vertexDesc), &floor_fileData[0], floor_fileData.size(), &floor_model._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTaskFloorModel = LoadPSTaskModel.then([this](const std::vector<byte>& floor_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&floor_fileData[0], floor_fileData.size(), nullptr, &floor_model._pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &floor_model._constantBuffer));

		// Create the constant buffers for the floor lights
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer_pointLight));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer_spotLight));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer_directionalLight));
	});

	// Once both shaders are loaded, create the mesh.
	auto createTaskFloorModel = (createPSTaskFloorModel && createVSTaskFloorModel).then([this]()
	{
		std::vector<DX11UWA::VertexPositionUVNormal> floor_vertices;
		std::vector<DirectX::XMFLOAT3> floor_normals;
		std::vector<DirectX::XMFLOAT2> floor_uvs;
		std::vector<unsigned int> floor_indices;

		loadOBJ("Assets/Models/Floor.obj", floor_vertices, floor_indices, floor_normals, floor_uvs);

		// Change uv's so the floor is a dark color or black
		for (unsigned int i = 0; i < floor_vertices.size(); i++)
		{
			floor_vertices[i].uv.x = 0.5f;
			floor_vertices[i].uv.y = 0.0f;
			floor_vertices[i].normal = DirectX::XMFLOAT3(0, 1, 0);
		}

		// Move down the floor, so it's below the big daddys feet
		for (unsigned int i = 0; i < floor_vertices.size(); i++)
		{
			floor_vertices[i].pos.y -= 10.35f;
		}

		floor_vertices_updater = floor_vertices;

		// Set the new color of the surface
		DirectX::XMFLOAT2 overall_result = { 0.0f, 0.0f };

		// Initialize the directional light data
		floor_directional_light.direction = { 0.0f, -4.0f, 1.0f, 0.0f };
		floor_directional_light.color = { 0.250980f , 0.611764f, 1.0f, 0.0f };

		// Initialize the point light data
		floor_point_light.position = { 0.0f, 2.0f, 0.0f, 0.0f };
		floor_point_light.color = { 0.788f, 0.886f, 1.0f, 0.0f };
		floor_point_light.radius.x = 3.0f;

		// Initialize the spot light data
		floor_spot_light.position = { 0.0f, 2.0f, 0.0f, 0.0f };
		floor_spot_light.color = { 1.0f, 0.945f, 0.878f, 0.0f };
		floor_spot_light.cone_direction = { 0.0f, -0.35f, -0.1f, 0.0f };
		floor_spot_light.cone_ratio.x = 0.5f;
		floor_spot_light.inner_cone_ratio.x = 0.96f;
		floor_spot_light.outer_cone_ratio.x = 0.95f;

//		for (unsigned int i = 0; i < floor_vertices.size(); i++)
//		{
//			DirectX::XMFLOAT3 surfacePosition = floor_vertices[i].pos;
//			DirectX::XMFLOAT3 surfaceNormal = floor_vertices[i].normal;
//			DirectX::XMFLOAT2 surfaceColor = floor_vertices[i].uv;
//
//#pragma region Directional Light
//
//			{
//				/*float lightRatio;
//				DirectX::XMFLOAT2 result;
//
//				lightRatio = Clamp(Vector_Dot(directionalLightDirection, surfaceNormal), 1.0f, 0.0f);
//				result.x = lightRatio * directionalLightColor.x * surfaceColor.x;
//				result.y = lightRatio * directionalLightColor.y * surfaceColor.y;
//
//				overall_result.x = result.x;
//				overall_result.y = result.y;*/
//			}
//
//#pragma endregion
//
//#pragma region Point Light
//
//			{
//				/*float attenuation;
//				DirectX::XMFLOAT3 lightDirection;
//				DirectX::XMFLOAT2 result;
//
//				lightDirection = Vector_Normalize(Vector_Subtraction(lightPosition, surfacePosition));
//
//				attenuation = 1.0f - Clamp(Vector_Length(Vector_Subtraction(lightPosition, surfacePosition)) - lightRadius, 1.0f, 0.0f);
//				result.x = attenuation * lightColor.x * surfaceColor.x;
//				result.y = attenuation * lightColor.y * surfaceColor.y;
//
//				overall_result.x += result.x;
//				overall_result.y += result.y;*/
//			}
//
//#pragma endregion
//
//#pragma region Spot Light
//			{
//				/*float attenuation;
//				float surfaceRatio;
//				float spotFactor;
//				DirectX::XMFLOAT3 lightDirection;
//				DirectX::XMFLOAT2 result;
//
//				lightDirection = Vector_Normalize(Vector_Subtraction(spotLightPosition, surfacePosition));
//
//				surfaceRatio = Clamp(Vector_Dot(Vector_Scalar_Multiply(lightDirection, -1.0f), coneDirection), 1.0f, 0.0f);
//
//				spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;
//
//				attenuation = 1.0f - Clamp((innerConeRatio - surfaceRatio) / (innerConeRatio - outerConeRatio), 1.0f, 0.0f);
//
//				result.x = spotFactor * attenuation * spotLightColor.x * surfaceColor.x;
//				result.y = spotFactor * attenuation * spotLightColor.y * surfaceColor.y;
//
//				overall_result.x += result.x;
//				overall_result.y += result.y;*/
//			}
//#pragma endregion
//
//			floor_vertices[i].uv = overall_result;
//		}
			

		D3D11_SUBRESOURCE_DATA floor_vertexBufferData = { 0 };
		floor_vertexBufferData.pSysMem = floor_vertices.data();
		floor_vertexBufferData.SysMemPitch = 0;
		floor_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC floor_vertexBufferDesc(sizeof(DX11UWA::VertexPositionUVNormal) * floor_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&floor_vertexBufferDesc, &floor_vertexBufferData, &floor_model._vertexBuffer));

		floor_model._indexCount = floor_indices.size();

		D3D11_SUBRESOURCE_DATA floor_indexBufferData = { 0 };
		floor_indexBufferData.pSysMem = floor_indices.data();
		floor_indexBufferData.SysMemPitch = 0;
		floor_indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC floor_indexBufferDesc(sizeof(unsigned int) * floor_indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&floor_indexBufferDesc, &floor_indexBufferData, &floor_model._indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createTaskFloorModel.then([this]()
	{
		floor_model._loadingComplete = true;
	});

#pragma endregion

#pragma region Skybox

	auto context_Skybox = m_deviceResources->GetD3DDeviceContext();
	ID3D11Device *device_Skybox;
	context_Skybox->GetDevice(&device_Skybox);

	const char *skybox_path = "Assets/Cubemaps/Rapture.dds";

	size_t skybox_pathSize = strlen(skybox_path) + 1;
	wchar_t *skybox_wc = new wchar_t[skybox_pathSize];
	mbstowcs(&skybox_wc[0], skybox_path, skybox_pathSize);

	HRESULT hr2;
	hr2 = CreateDDSTextureFromFile(device_Skybox, skybox_wc, &skybox_texture, &skyboxSRV);

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-15.0f, -15.0f, -15.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-15.0f, -15.0f,  15.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-15.0f,  15.0f, -15.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-15.0f,  15.0f,  15.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(15.0f, -15.0f, -15.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(15.0f, -15.0f,  15.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(15.0f,  15.0f, -15.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(15.0f,  15.0f,  15.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			2,1,0, // -x
			2,3,1,

			5,6,4, // +x
			7,6,5,

			1,5,0, // -y
			5,4,0,

			6,7,2, // +y
			7,3,2,

			4,6,0, // -z
			6,2,0,

			3,7,1, // +z
			7,5,1,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});

#pragma endregion

#pragma region Big Daddy Model

	//auto context = m_deviceResources->GetD3DDeviceContext();
	//ID3D11Device *device;
	//context->GetDevice(&device);

	//const char *path = "Assets/Textures/Big_Daddy_Texture.dds";

	//size_t pathSize = strlen(path) + 1;
	//wchar_t *wc = new wchar_t[pathSize];
	//mbstowcs(&wc[0], path, pathSize);

	//HRESULT hr;
	//hr = CreateDDSTextureFromFile(device, wc, &texture, &bigDaddyMeshSRV);

	//// After the vertex shader file is loaded, create the shader and input layout.
	//auto createVSBigDaddyTaskModel = loadVSTaskTexture.then([this](const std::vector<byte>& bigDaddy_fileData)
	//{
	//	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&bigDaddy_fileData[0], bigDaddy_fileData.size(), nullptr, &big_daddy_model._vertexShader));

	//	static const D3D11_INPUT_ELEMENT_DESC bigDaddy_vertexDesc[] =
	//	{
	//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	};

	//	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(bigDaddy_vertexDesc, ARRAYSIZE(bigDaddy_vertexDesc), &bigDaddy_fileData[0], bigDaddy_fileData.size(), &big_daddy_model._inputLayout));
	//});

	//// After the pixel shader file is loaded, create the shader and constant buffer.
	//auto createPSBigDaddyTaskModel = loadPSTaskTexture.then([this](const std::vector<byte>& bigDaddy_fileData)
	//{
	//	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&bigDaddy_fileData[0], bigDaddy_fileData.size(), nullptr, &big_daddy_model._pixelShader));

	//	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	//	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &big_daddy_model._constantBuffer));
	//});

	//// Once both shaders are loaded, create the mesh.
	//auto createBigDaddyTaskModel = (createPSBigDaddyTaskModel && createVSBigDaddyTaskModel).then([this]()
	//{
	//	std::vector<DX11UWA::VertexPositionUVNormal> bigDaddy_vertices;
	//	std::vector<DirectX::XMFLOAT3> bigDaddy_normals;
	//	std::vector<DirectX::XMFLOAT2> bigDaddy_uvs;
	//	std::vector<unsigned int> bigDaddy_indices;

	//	loadOBJ("Assets/Models/Big_Daddy.obj", bigDaddy_vertices, bigDaddy_indices, bigDaddy_normals, bigDaddy_uvs);

	//	// Move down the big daddy, so the floor is below his feet
	//	for (unsigned int i = 0; i < bigDaddy_vertices.size(); i++)
	//	{
	//		bigDaddy_vertices[i].pos.y -= 10.00f;
	//	}

	//	D3D11_SUBRESOURCE_DATA bigDaddy_vertexBufferData = { 0 };
	//	bigDaddy_vertexBufferData.pSysMem = bigDaddy_vertices.data();
	//	bigDaddy_vertexBufferData.SysMemPitch = 0;
	//	bigDaddy_vertexBufferData.SysMemSlicePitch = 0;
	//	CD3D11_BUFFER_DESC bigDaddy_vertexBufferDesc(sizeof(DX11UWA::VertexPositionUVNormal) * bigDaddy_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
	//	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bigDaddy_vertexBufferDesc, &bigDaddy_vertexBufferData, &big_daddy_model._vertexBuffer));

	//	big_daddy_model._indexCount = bigDaddy_indices.size();

	//	D3D11_SUBRESOURCE_DATA bigDaddy_indexBufferData = { 0 };
	//	bigDaddy_indexBufferData.pSysMem = bigDaddy_indices.data();
	//	bigDaddy_indexBufferData.SysMemPitch = 0;
	//	bigDaddy_indexBufferData.SysMemSlicePitch = 0;
	//	CD3D11_BUFFER_DESC bigDaddy_indexBufferDesc(sizeof(unsigned int) * bigDaddy_indices.size(), D3D11_BIND_INDEX_BUFFER);
	//	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bigDaddy_indexBufferDesc, &bigDaddy_indexBufferData, &big_daddy_model._indexBuffer));
	//});

	//// Once the cube is loaded, the object is ready to be rendered.
	//createBigDaddyTaskModel.then([this]()
	//{
	//	big_daddy_model._loadingComplete = true;
	//});

#pragma endregion

}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

void Sample3DSceneRenderer::UpdateLights()
{
	// Get the context so I can update the lights
	auto context = m_deviceResources->GetD3DDeviceContext();

	// Update subresources for the lights
	context->UpdateSubresource1(m_constantBuffer_pointLight.Get(), 0, NULL, &floor_point_light, 0, 0, 0);
	context->UpdateSubresource1(m_constantBuffer_directionalLight.Get(), 0, NULL, &floor_directional_light, 0, 0, 0);
	context->UpdateSubresource1(m_constantBuffer_spotLight.Get(), 0, NULL, &floor_spot_light, 0, 0, 0);


	// Set the light constant buffers to the floor
	context->PSSetConstantBuffers1(0, 1, m_constantBuffer_pointLight.GetAddressOf(), nullptr, nullptr);
	context->PSSetConstantBuffers1(1, 1, m_constantBuffer_directionalLight.GetAddressOf(), nullptr, nullptr);
	context->PSSetConstantBuffers1(2, 1, m_constantBuffer_spotLight.GetAddressOf(), nullptr, nullptr);

}