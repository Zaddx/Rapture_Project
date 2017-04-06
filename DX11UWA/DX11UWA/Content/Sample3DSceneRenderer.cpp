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


	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));

	// Update the constant buffer data based on camera
	XMStoreFloat4x4(&m_constantBufferData.view, (XMMatrixLookAtLH(eye, at, up)));
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

	// Translate the position (Big Daddy)
	XMMATRIX bigDaddy_translationX = XMMatrixTranslation(0.0f, 0.0f, -1.0f);
	XMMATRIX bigDaddy_rotationY = XMMatrixRotationY(radians);

	XMStoreFloat4x4(&m_constantBufferData_big_daddy.model, (XMMatrixMultiply(bigDaddy_rotationY, bigDaddy_translationX)));
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

#pragma endregion

#pragma region Big Daddy Model

	if (!big_daddy_model._loadingComplete)
	{
		return;
	}

	ID3D11ShaderResourceView** texViews[] = { bigDaddyMeshSRV.GetAddressOf() };
	context->PSSetShaderResources(0, 1, *texViews);

	XMStoreFloat4x4(&m_constantBufferData_big_daddy.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Setup Vertex Buffer
	UINT bigDaddy_stride = sizeof(DX11UWA::VertexPositionUVNormal);
	UINT bigDaddy_offset = 0;
	context->IASetVertexBuffers(0, 1, big_daddy_model._vertexBuffer.GetAddressOf(), &bigDaddy_stride, &bigDaddy_offset);

	// Set Index buffer
	context->IASetIndexBuffer(big_daddy_model._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(big_daddy_model._inputLayout.Get());

	context->UpdateSubresource1(big_daddy_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_big_daddy, 0, 0, 0);

	// Attach our vertex shader.
	context->VSSetShader(big_daddy_model._vertexShader.Get(), nullptr, 0);

	// Attach our pixel shader.
	context->PSSetShader(big_daddy_model._pixelShader.Get(), nullptr, 0);

	context->DrawIndexed(big_daddy_model._indexCount, 0, 0);

#pragma endregion

}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	auto loadVSTaskTexture = DX::ReadDataAsync(L"TextureVertexShader.cso");
	auto loadPSTaskTexture = DX::ReadDataAsync(L"TexturePixelShader.cso");

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
			{ XMFLOAT3(-50.0f, -50.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-50.0f, -50.0f,  50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-50.0f,  50.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-50.0f,  50.0f,  50.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(50.0f, -50.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(50.0f, -50.0f,  50.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(50.0f,  50.0f, -50.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(50.0f,  50.0f,  50.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
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

	auto context = m_deviceResources->GetD3DDeviceContext();
	ID3D11Device *device;
	context->GetDevice(&device);

	const char *path = "Assets/Textures/Big_Daddy_Texture.dds";

	size_t pathSize = strlen(path) + 1;
	wchar_t *wc = new wchar_t[pathSize];
	mbstowcs(&wc[0], path, pathSize);

	HRESULT hr;
	hr = CreateDDSTextureFromFile(device, wc, &texture, &bigDaddyMeshSRV);

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSBigDaddyTaskModel = loadVSTaskTexture.then([this](const std::vector<byte>& bigDaddy_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&bigDaddy_fileData[0], bigDaddy_fileData.size(), nullptr, &big_daddy_model._vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC bigDaddy_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(bigDaddy_vertexDesc, ARRAYSIZE(bigDaddy_vertexDesc), &bigDaddy_fileData[0], bigDaddy_fileData.size(), &big_daddy_model._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSBigDaddyTaskModel = loadPSTaskTexture.then([this](const std::vector<byte>& bigDaddy_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&bigDaddy_fileData[0], bigDaddy_fileData.size(), nullptr, &big_daddy_model._pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &big_daddy_model._constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createBigDaddyTaskModel = (createPSBigDaddyTaskModel && createVSBigDaddyTaskModel).then([this]()
	{
		std::vector<DX11UWA::VertexPositionUVNormal> bigDaddy_vertices;
		std::vector<DirectX::XMFLOAT3> bigDaddy_normals;
		std::vector<DirectX::XMFLOAT2> bigDaddy_uvs;
		std::vector<unsigned int> bigDaddy_indices;

		loadOBJ("Assets/Models/Big_Daddy.obj", bigDaddy_vertices, bigDaddy_indices, bigDaddy_normals, bigDaddy_uvs);

		D3D11_SUBRESOURCE_DATA bigDaddy_vertexBufferData = { 0 };
		bigDaddy_vertexBufferData.pSysMem = bigDaddy_vertices.data();
		bigDaddy_vertexBufferData.SysMemPitch = 0;
		bigDaddy_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC bigDaddy_vertexBufferDesc(sizeof(DX11UWA::VertexPositionUVNormal) * bigDaddy_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
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
}