#include "pch.h"
#include "DX11UWAMain.h"
#include "Common\DirectXHelper.h"
#include <DirectXMath.h>

using namespace DX11UWA;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
DX11UWAMain::DX11UWAMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources), m_deviceResources2(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	m_deviceResources2->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	m_sceneRenderer2 = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));
	m_fpsTextRenderer2 = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
}

DX11UWAMain::~DX11UWAMain(void)
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);

	m_deviceResources2->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void DX11UWAMain::CreateWindowSizeDependentResources(void)
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();

	m_sceneRenderer2->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void DX11UWAMain::Update(void)
{
	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		m_sceneRenderer->Update(m_timer);
		m_sceneRenderer->SetInputDeviceData(main_kbuttons, main_currentpos);
		m_fpsTextRenderer->Update(m_timer);

		m_sceneRenderer2->Update(m_timer);
		m_sceneRenderer2->SetInputDeviceData(main_kbuttons, main_currentpos);
		m_fpsTextRenderer2->Update(m_timer);
	});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool DX11UWAMain::Render(void)
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	viewport.Width = m_deviceResources->GetScreenViewport().Width / 2.0f;

	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	//context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	DirectX::XMMATRIX temp;
	temp = DirectX::XMMatrixIdentity();

	DirectX::XMFLOAT4X4 temp_4x4;
	DirectX::XMStoreFloat4x4(&temp_4x4, temp);

	m_sceneRenderer->Render(temp_4x4);
	m_fpsTextRenderer->Render();


	// Setup secondary Viewport
	DirectX::XMMATRIX temp2a, temp2b, temp2c;
	temp2a = DirectX::XMMatrixIdentity();
	temp2b = DirectX::XMMatrixTranslation(0.0f, -3.0f, 5.0f);
	temp2c = DirectX::XMMatrixMultiply(temp2a, temp2b);
	
	DirectX::XMFLOAT4X4 temp_4x4_2;
	DirectX::XMStoreFloat4x4(&temp_4x4_2, temp2c);

	auto viewport_custom = m_deviceResources2->GetScreenViewport();
	viewport_custom.Width = m_deviceResources2->GetScreenViewport().Width / 2.0f;
	viewport_custom.TopLeftX = m_deviceResources2->GetScreenViewport().Width / 2.0f;

	context->RSSetViewports(1, &viewport_custom);

	ID3D11RenderTargetView *const targets_custom[1] = { m_deviceResources2->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets_custom, m_deviceResources2->GetDepthStencilView());

	context->ClearDepthStencilView(m_deviceResources2->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	m_sceneRenderer2->Render(temp_4x4_2);
	m_fpsTextRenderer2->Render();

	return true;
}

// Notifies renderers that device resources need to be released.
void DX11UWAMain::OnDeviceLost(void)
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();

	m_sceneRenderer2->ReleaseDeviceDependentResources();
	m_fpsTextRenderer2->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void DX11UWAMain::OnDeviceRestored(void)
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	m_sceneRenderer2->CreateDeviceDependentResources();
	m_fpsTextRenderer2->CreateDeviceDependentResources();
}

void DX11UWAMain::GetKeyboardButtons(const char* buttons)
{
	memcpy_s(main_kbuttons, sizeof(main_kbuttons), buttons, sizeof(main_kbuttons));
}

void DX11UWAMain::GetMousePos(const Windows::UI::Input::PointerPoint^ pos)
{
	main_currentpos = const_cast< Windows::UI::Input::PointerPoint^>(pos);
}
