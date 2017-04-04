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
	XMStoreFloat4x4(&m_constantBufferData_2.projection, (perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_constantBufferData_3.projection, (perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_constantBufferData_pyramid.projection, (perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_constantBufferData_pyramid2.projection, (perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_constantBufferData_pyramid3.projection, (perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_constantBufferData_big_daddy.projection, (perspectiveMatrix * orientationMatrix));


	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));

	// Update the constant buffer data based on camera
	XMStoreFloat4x4(&m_constantBufferData.view, (XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData_2.view, (XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData_3.view, (XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData_pyramid.view, (XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData_pyramid2.view, (XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData_pyramid3.view, (XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData_big_daddy.view, (XMMatrixLookAtLH(eye, at, up)));
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

}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, (XMMatrixRotationY(radians)));

	// Translate the position then rotate it on the x (2nd Cube)
	XMMATRIX rotationX = XMMatrixRotationX(radians);
	XMMATRIX translationX = XMMatrixTranslation(3, 0, 0);

	XMStoreFloat4x4(&m_constantBufferData_2.model, (XMMatrixMultiply(rotationX,translationX)));

	// Translate the position then rotate it on the z (3rd Cube)
	XMMATRIX rotationZ = XMMatrixRotationZ(radians);
	XMMATRIX translationXY = XMMatrixTranslation(1.5f, 1, 0);

	XMStoreFloat4x4(&m_constantBufferData_3.model, (XMMatrixMultiply(rotationZ, translationXY)));

	// Translate the position (Pyramid)
	XMMATRIX pyramid_translationX = XMMatrixTranslation(-1.5f, 0, 0);

	XMStoreFloat4x4(&m_constantBufferData_pyramid.model, pyramid_translationX);

	// Translate the position (Pyramid 2)
	XMMATRIX pyramid2_translationXY = XMMatrixTranslation(-1.5f, 1.0f, 0);

	XMStoreFloat4x4(&m_constantBufferData_pyramid2.model, pyramid2_translationXY);

	// Translate the position (Pyramid 3)
	XMMATRIX pyramid3_translationXY = XMMatrixTranslation(-1.5f, 2.0f, 0);

	XMStoreFloat4x4(&m_constantBufferData_pyramid3.model, pyramid3_translationXY);

	// Translate the position (Big Daddy)
	XMMATRIX bigDaddy_translationX = XMMatrixTranslation(-4.5f, 0, 0);

	XMStoreFloat4x4(&m_constantBufferData_big_daddy.model, bigDaddy_translationX);
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
void Sample3DSceneRenderer::Render(void)
{

#pragma region Cube

	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

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

	// Drawing a second cube
	XMStoreFloat4x4(&m_constantBufferData_2.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData_2, 0, 0, 0);

	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);

	// Drawing a third cube
	XMStoreFloat4x4(&m_constantBufferData_3.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData_3, 0, 0, 0);

	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);

#pragma endregion

#pragma region Test Pyramid Model


	// Load pyramid 1
	if (!test_pyramid_model._loadingComplete)
	{
		return;
	}

	XMStoreFloat4x4(&m_constantBufferData_pyramid.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Setup Vertex Buffer
	UINT pyramid_stride = sizeof(DX11UWA::VertexPositionColor);
	UINT pyramid_offset = 0;
	context->IASetVertexBuffers(0, 1, test_pyramid_model._vertexBuffer.GetAddressOf(), &pyramid_stride, &pyramid_offset);
	
	// Set Index buffer
	context->IASetIndexBuffer(test_pyramid_model._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData_pyramid, 0, 0, 0);

	context->DrawIndexed(test_pyramid_model._indexCount, 0, 0);
	
	// Load pyramid 2
	if (!test_pyramid_model2._loadingComplete)
	{
		return;
	}

	XMStoreFloat4x4(&m_constantBufferData_pyramid2.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Setup Vertex Buffer
	UINT pyramid2_stride = sizeof(DX11UWA::VertexPositionColor);
	UINT pyramid2_offset = 0;
	context->IASetVertexBuffers(0, 1, test_pyramid_model2._vertexBuffer.GetAddressOf(), &pyramid2_stride, &pyramid2_offset);

	// Set Index buffer
	context->IASetIndexBuffer(test_pyramid_model2._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData_pyramid2, 0, 0, 0);

	context->DrawIndexed(test_pyramid_model2._indexCount, 0, 0);


	// Load Pyramid 3
	if (!test_pyramid_model3._loadingComplete)
	{
		return;
	}

	XMStoreFloat4x4(&m_constantBufferData_pyramid3.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Setup Vertex Buffer
	UINT pyramid3_stride = sizeof(DX11UWA::VertexPositionColor);
	UINT pyramid3_offset = 0;
	context->IASetVertexBuffers(0, 1, test_pyramid_model3._vertexBuffer.GetAddressOf(), &pyramid3_stride, &pyramid3_offset);

	// Set Index buffer
	context->IASetIndexBuffer(test_pyramid_model3._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData_pyramid3, 0, 0, 0);

	context->DrawIndexed(test_pyramid_model3._indexCount, 0, 0);

#pragma endregion

#pragma region Big Daddy Model

	if (!big_daddy_model._loadingComplete)
	{
		return;
	}

	XMStoreFloat4x4(&m_constantBufferData_big_daddy.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Setup Vertex Buffer
	UINT bigDaddy_stride = sizeof(DX11UWA::VertexPositionColor);
	UINT bigDaddy_offset = 0;
	context->IASetVertexBuffers(0, 1, big_daddy_model._vertexBuffer.GetAddressOf(), &bigDaddy_stride, &bigDaddy_offset);

	// Set Index buffer
	context->IASetIndexBuffer(big_daddy_model._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData_big_daddy, 0, 0, 0);

	context->DrawIndexed(big_daddy_model._indexCount, 0, 0);

#pragma endregion

}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	auto loadVSTaskModel = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTaskModel = DX::ReadDataAsync(L"SamplePixelShader.cso");

#pragma region Cube

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
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
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
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
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

#pragma region Test Pyramid Model (Both Lights)

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTasPyramidkModel = loadVSTaskModel.then([this](const std::vector<byte>& pyramid_fileData)
	{
		static const D3D11_INPUT_ELEMENT_DESC pyramid_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(pyramid_vertexDesc, ARRAYSIZE(pyramid_vertexDesc), &pyramid_fileData[0], pyramid_fileData.size(), &test_pyramid_model._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTaskPyramidModel = loadPSTaskModel.then([this](const std::vector<byte>& pyramid_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&pyramid_fileData[0], pyramid_fileData.size(), nullptr, &test_pyramid_model._pixelShader));
	});

	// Once both shaders are loaded, create the mesh.
	auto createPyramidTaskModel = (createPSTaskPyramidModel && createVSTasPyramidkModel).then([this]()
	{
		std::vector<DX11UWA::VertexPositionColor> pyramid_vertices;
		std::vector<DirectX::XMFLOAT3> pyramid_normals;
		std::vector<unsigned int> pyramid_indices;

		loadOBJ("Assets/Models/test pyramid.obj", pyramid_vertices, pyramid_indices, pyramid_normals);

		// Change the colors of all the Vertices to Sand/Beige
		for (unsigned int i = 0; i < pyramid_vertices.size(); i++)
		{
			DirectX::XMFLOAT3 temp_color_purple, temp_color_beige, temp_color_mixed;

			temp_color_purple.x = .501f;
			temp_color_purple.y = .000f;
			temp_color_purple.z = .501f;

			temp_color_beige.x = .933f;
			temp_color_beige.y = .839f;
			temp_color_beige.z = .568f;

			temp_color_mixed = Lerp(temp_color_beige, temp_color_purple, 0.5f);

			pyramid_vertices[i].color = temp_color_mixed;
		}

#if 1

		// Initializing the Directional light
		pyramid_directional_Light.direction.x = 0.0f;
		pyramid_directional_Light.direction.y = 1.0f;
		pyramid_directional_Light.direction.z = -0.5f;

		pyramid_directional_Light.color.x = 0.5f;
		pyramid_directional_Light.color.y = 0.5f;
		pyramid_directional_Light.color.z = 0.5f;

		for (unsigned int i = 0; i < pyramid_normals.size(); i++)
		{
			DirectX::XMFLOAT3 temp_Normal = pyramid_normals[i];
			DirectX::XMFLOAT3 temp_color = pyramid_vertices[i].color;

			// Get the light ratio
			float lightRatio = Clamp(Vector_Dot(Vector_Scalar_Multiply(pyramid_directional_Light.direction, 1.0f), temp_Normal), 1.0f, 0.0f);

			// Apply the new color
			pyramid_vertices[i].color.x = lightRatio * pyramid_directional_Light.color.x * temp_color.x;
			pyramid_vertices[i].color.y = lightRatio * pyramid_directional_Light.color.y * temp_color.y;
			pyramid_vertices[i].color.z = lightRatio * pyramid_directional_Light.color.z * temp_color.z;

		}

#endif // Directional Lighting

#if 1

		// Initialize the Point Light
		pyramid_point_light.position = { 0.0f, 1.0f, 0.0f };
		pyramid_point_light.color = { 0.5f, 0.5f, 0.5f };

		for (unsigned int i = 0; i < pyramid_normals.size(); i++)
		{
			DirectX::XMFLOAT3 surface_normal = pyramid_normals[i];
			DirectX::XMFLOAT3 surface_color = pyramid_vertices[i].color;
			DirectX::XMFLOAT3 surface_position = pyramid_vertices[i].pos;

			// Get the values for the lighting
			DirectX::XMFLOAT3 lightDir = Vector_Normalize(Vector_Subtraction(pyramid_point_light.position, surface_position));
			float lightRatio = Clamp(Vector_Dot(lightDir, surface_normal), 1.0f, 0.0f);
			
			// Get the new color
			pyramid_vertices[i].color.x = lightRatio * pyramid_point_light.color.x * surface_color.x;
			pyramid_vertices[i].color.y = lightRatio * pyramid_point_light.color.y * surface_color.y;
			pyramid_vertices[i].color.z = lightRatio * pyramid_point_light.color.z * surface_color.z;
		}

#endif // Point Lighting

		D3D11_SUBRESOURCE_DATA pyramid_vertexBufferData = { 0 };
		pyramid_vertexBufferData.pSysMem = pyramid_vertices.data();
		pyramid_vertexBufferData.SysMemPitch = 0;
		pyramid_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC pyramid_vertexBufferDesc(sizeof(DX11UWA::VertexPositionColor) * pyramid_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pyramid_vertexBufferDesc, &pyramid_vertexBufferData, &test_pyramid_model._vertexBuffer));

		test_pyramid_model._indexCount = pyramid_indices.size();

		D3D11_SUBRESOURCE_DATA pyramid_indexBufferData = { 0 };
		pyramid_indexBufferData.pSysMem = pyramid_indices.data();
		pyramid_indexBufferData.SysMemPitch = 0;
		pyramid_indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC pyramid_indexBufferDesc(sizeof(unsigned int) * pyramid_indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pyramid_indexBufferDesc, &pyramid_indexBufferData, &test_pyramid_model._indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createPyramidTaskModel.then([this]()
	{
		test_pyramid_model._loadingComplete = true;
	});

#pragma endregion

#pragma region Test Pyramid Model 2 (Point Light)

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTasPyramidkModel2 = loadVSTaskModel.then([this](const std::vector<byte>& pyramid2_fileData)
	{
		static const D3D11_INPUT_ELEMENT_DESC pyramid2_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(pyramid2_vertexDesc, ARRAYSIZE(pyramid2_vertexDesc), &pyramid2_fileData[0], pyramid2_fileData.size(), &test_pyramid_model2._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTaskPyramidModel2 = loadPSTaskModel.then([this](const std::vector<byte>& pyramid2_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&pyramid2_fileData[0], pyramid2_fileData.size(), nullptr, &test_pyramid_model2._pixelShader));
	});

	// Once both shaders are loaded, create the mesh.
	auto createPyramidTaskModel2 = (createPSTaskPyramidModel2 && createVSTasPyramidkModel2).then([this]()
	{
		std::vector<DX11UWA::VertexPositionColor> pyramid2_vertices;
		std::vector<DirectX::XMFLOAT3> pyramid2_normals;
		std::vector<unsigned int> pyramid2_indices;

		loadOBJ("Assets/Models/test pyramid.obj", pyramid2_vertices, pyramid2_indices, pyramid2_normals);

		// Change the colors of all the Vertices to Purple
		for (unsigned int i = 0; i < pyramid2_vertices.size(); i++)
		{
			DirectX::XMFLOAT3 temp_color;

			temp_color.x = .501f;
			temp_color.y = .000f;
			temp_color.z = .501f;

			pyramid2_vertices[i].color = temp_color;
		}

#if 1

		// Initialize the Point Light
		pyramid_point_light.position = { 0.0f, 1.0f, 0.0f };
		pyramid_point_light.color = { 0.5f, 0.5f, 0.5f };

		for (unsigned int i = 0; i < pyramid2_normals.size(); i++)
		{
			DirectX::XMFLOAT3 surface_normal = pyramid2_normals[i];
			DirectX::XMFLOAT3 surface_color = pyramid2_vertices[i].color;
			DirectX::XMFLOAT3 surface_position = pyramid2_vertices[i].pos;

			// Get the values for the lighting
			DirectX::XMFLOAT3 lightDir = Vector_Normalize(Vector_Subtraction(pyramid_point_light.position, surface_position));
			float lightRatio = Clamp(Vector_Dot(lightDir, surface_normal), 1.0f, 0.0f);

			// Get the new color
			pyramid2_vertices[i].color.x = lightRatio * pyramid_point_light.color.x * surface_color.x;
			pyramid2_vertices[i].color.y = lightRatio * pyramid_point_light.color.y * surface_color.y;
			pyramid2_vertices[i].color.z = lightRatio * pyramid_point_light.color.z * surface_color.z;
		}

#endif // Point Lighting

		D3D11_SUBRESOURCE_DATA pyramid2_vertexBufferData = { 0 };
		pyramid2_vertexBufferData.pSysMem = pyramid2_vertices.data();
		pyramid2_vertexBufferData.SysMemPitch = 0;
		pyramid2_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC pyramid2_vertexBufferDesc(sizeof(DX11UWA::VertexPositionColor) * pyramid2_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pyramid2_vertexBufferDesc, &pyramid2_vertexBufferData, &test_pyramid_model2._vertexBuffer));

		test_pyramid_model2._indexCount = pyramid2_indices.size();

		D3D11_SUBRESOURCE_DATA pyramid2_indexBufferData = { 0 };
		pyramid2_indexBufferData.pSysMem = pyramid2_indices.data();
		pyramid2_indexBufferData.SysMemPitch = 0;
		pyramid2_indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC pyramid2_indexBufferDesc(sizeof(unsigned int) * pyramid2_indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pyramid2_indexBufferDesc, &pyramid2_indexBufferData, &test_pyramid_model2._indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createPyramidTaskModel2.then([this]()
	{
		test_pyramid_model2._loadingComplete = true;
	});

#pragma endregion

#pragma region Test Pyramid Model 3 (Directional Light)

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTasPyramidkModel3 = loadVSTaskModel.then([this](const std::vector<byte>& pyramid3_fileData)
	{
		static const D3D11_INPUT_ELEMENT_DESC pyramid3_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(pyramid3_vertexDesc, ARRAYSIZE(pyramid3_vertexDesc), &pyramid3_fileData[0], pyramid3_fileData.size(), &test_pyramid_model3._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTaskPyramidModel3 = loadPSTaskModel.then([this](const std::vector<byte>& pyramid3_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&pyramid3_fileData[0], pyramid3_fileData.size(), nullptr, &test_pyramid_model3._pixelShader));
	});

	// Once both shaders are loaded, create the mesh.
	auto createPyramidTaskModel3 = (createPSTaskPyramidModel3 && createVSTasPyramidkModel3).then([this]()
	{
		std::vector<DX11UWA::VertexPositionColor> pyramid3_vertices;
		std::vector<DirectX::XMFLOAT3> pyramid3_normals;
		std::vector<unsigned int> pyramid3_indices;

		loadOBJ("Assets/Models/test pyramid.obj", pyramid3_vertices, pyramid3_indices, pyramid3_normals);

		// Change the colors of all the Vertices to Sand/Beige
		for (unsigned int i = 0; i < pyramid3_vertices.size(); i++)
		{
			DirectX::XMFLOAT3 temp_color;

			temp_color.x = .933f;
			temp_color.y = .839f;
			temp_color.z = .568f;

			pyramid3_vertices[i].color = temp_color;
		}

#if 1

		// Initializing the Directional light
		pyramid_directional_Light.direction.x = 0.0f;
		pyramid_directional_Light.direction.y = 1.0f;
		pyramid_directional_Light.direction.z = -0.5f;

		pyramid_directional_Light.color.x = 0.5f;
		pyramid_directional_Light.color.y = 0.5f;
		pyramid_directional_Light.color.z = 0.5f;

		for (unsigned int i = 0; i < pyramid3_normals.size(); i++)
		{
			DirectX::XMFLOAT3 temp_Normal = pyramid3_normals[i];
			DirectX::XMFLOAT3 temp_color = pyramid3_vertices[i].color;

			// Get the light ratio
			float lightRatio = Clamp(Vector_Dot(Vector_Scalar_Multiply(pyramid_directional_Light.direction, 1.0f), temp_Normal), 1.0f, 0.0f);

			// Apply the new color
			pyramid3_vertices[i].color.x = lightRatio * pyramid_directional_Light.color.x * temp_color.x;
			pyramid3_vertices[i].color.y = lightRatio * pyramid_directional_Light.color.y * temp_color.y;
			pyramid3_vertices[i].color.z = lightRatio * pyramid_directional_Light.color.z * temp_color.z;

		}

#endif // Directional Lighting

		D3D11_SUBRESOURCE_DATA pyramid3_vertexBufferData = { 0 };
		pyramid3_vertexBufferData.pSysMem = pyramid3_vertices.data();
		pyramid3_vertexBufferData.SysMemPitch = 0;
		pyramid3_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC pyramid3_vertexBufferDesc(sizeof(DX11UWA::VertexPositionColor) * pyramid3_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pyramid3_vertexBufferDesc, &pyramid3_vertexBufferData, &test_pyramid_model3._vertexBuffer));

		test_pyramid_model3._indexCount = pyramid3_indices.size();

		D3D11_SUBRESOURCE_DATA pyramid3_indexBufferData = { 0 };
		pyramid3_indexBufferData.pSysMem = pyramid3_indices.data();
		pyramid3_indexBufferData.SysMemPitch = 0;
		pyramid3_indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC pyramid3_indexBufferDesc(sizeof(unsigned int) * pyramid3_indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pyramid3_indexBufferDesc, &pyramid3_indexBufferData, &test_pyramid_model3._indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createPyramidTaskModel3.then([this]()
	{
		test_pyramid_model3._loadingComplete = true;
	});

#pragma endregion



#pragma region Big Daddy Model

	auto context = m_deviceResources->GetD3DDeviceContext();
	ID3D11Device *device;
	context->GetDevice(&device);

	const char *path = "Assets/Textures/Big_Daddy_Texture.dds";

	size_t pathSize = strlen(path) + 1;
	wchar_t *wc = new wchar_t[pathSize];
	mbstowcs(&wc[0], path, pathSize);

	HRESULT hr;

	ID3D11Resource *texture;
	ID3D11ShaderResourceView *bigDaddyMeshSRV;
	hr = CreateDDSTextureFromFile(device, wc, &texture, &bigDaddyMeshSRV);

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSBigDaddyTaskModel = loadVSTaskModel.then([this](const std::vector<byte>& bigDaddy_fileData)
	{
		static const D3D11_INPUT_ELEMENT_DESC bigDaddy_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(bigDaddy_vertexDesc, ARRAYSIZE(bigDaddy_vertexDesc), &bigDaddy_fileData[0], bigDaddy_fileData.size(), &big_daddy_model._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSBigDaddyTaskModel = loadPSTaskModel.then([this](const std::vector<byte>& bigDaddy_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&bigDaddy_fileData[0], bigDaddy_fileData.size(), nullptr, &big_daddy_model._pixelShader));
	});

	// Once both shaders are loaded, create the mesh.
	auto createBigDaddyTaskModel = (createPSBigDaddyTaskModel && createVSBigDaddyTaskModel).then([this]()
	{
		std::vector<DX11UWA::VertexPositionColor> bigDaddy_vertices;
		std::vector<DirectX::XMFLOAT3> bigDaddy_normals;
		std::vector<unsigned int> bigDaddy_indices;

		loadOBJ("Assets/Models/Big_Daddy.obj", bigDaddy_vertices, bigDaddy_indices, bigDaddy_normals);

		D3D11_SUBRESOURCE_DATA bigDaddy_vertexBufferData = { 0 };
		bigDaddy_vertexBufferData.pSysMem = bigDaddy_vertices.data();
		bigDaddy_vertexBufferData.SysMemPitch = 0;
		bigDaddy_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC bigDaddy_vertexBufferDesc(sizeof(DX11UWA::VertexPositionColor) * bigDaddy_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bigDaddy_vertexBufferDesc, &bigDaddy_vertexBufferData, &big_daddy_model._vertexBuffer));

		big_daddy_model._indexCount = bigDaddy_indices.size();

		D3D11_SUBRESOURCE_DATA bigDaddy_indexBufferData = { 0 };
		bigDaddy_indexBufferData.pSysMem = bigDaddy_indices.data();
		bigDaddy_indexBufferData.SysMemPitch = 0;
		bigDaddy_indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC bigDaddy_indexBufferDesc(sizeof(unsigned int) * bigDaddy_indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bigDaddy_indexBufferDesc, &bigDaddy_indexBufferData, &big_daddy_model._indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createBigDaddyTaskModel.then([this]()
	{
		big_daddy_model._loadingComplete = true;
	});

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

	test_pyramid_model._loadingComplete = false;
	test_pyramid_model._vertexShader.Reset();
	test_pyramid_model._inputLayout.Reset();
	test_pyramid_model._pixelShader.Reset();
	test_pyramid_model._constantBuffer.Reset();
	test_pyramid_model._vertexBuffer.Reset();
	test_pyramid_model._indexBuffer.Reset();
}