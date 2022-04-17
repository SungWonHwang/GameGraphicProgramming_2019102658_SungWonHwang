#include "Renderer/Renderer.h"

namespace library
{
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Renderer
	  Summary:  Constructor
	  Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
				  m_immediateContext, m_immediateContext1, m_swapChain,
				  m_swapChain1, m_renderTargetView, m_vertexShader,
				  m_pixelShader, m_vertexLayout, m_vertexBuffer].

	Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
				 m_immediateContext, m_immediateContext1, m_swapChain,
				 m_swapChain1, m_renderTargetView, m_depthStencil,
				 m_depthStencilView, m_cbChangeOnResize, m_camera,
				 m_projection, m_renderables, m_vertexShaders,
				 m_pixelShaders].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	Renderer::Renderer()
		: m_driverType(D3D_DRIVER_TYPE_NULL),
		m_featureLevel(D3D_FEATURE_LEVEL_11_0),
		m_d3dDevice(nullptr),
		m_d3dDevice1(nullptr),
		m_immediateContext(nullptr),
		m_immediateContext1(nullptr),
		m_swapChain(nullptr),
		m_swapChain1(nullptr),
		m_renderTargetView(nullptr),

		m_depthStencil(nullptr),
		m_depthStencilView(nullptr),

		m_cbChangeOnResize(nullptr),
		m_padding(),
		//m_view(XMMATRIX()),
		m_camera(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)),
		m_projection(XMMATRIX()),

		m_renderables(std::unordered_map<std::wstring, std::shared_ptr<Renderable>>()),
		m_vertexShaders(std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>()),
		m_pixelShaders(std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>())
	{
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Initialize
	  Summary:  Creates Direct3D device and swap chain
	  Args:     HWND hWnd
				  Handle to the window
	  Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
				  m_d3dDevice1, m_immediateContext1, m_swapChain1,
				  m_swapChain, m_renderTargetView, m_vertexShader,
				  m_vertexLayout, m_pixelShader, m_vertexBuffer].
	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	HRESULT Renderer::Initialize(_In_ HWND hWnd)
	{
		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect(hWnd, &rc);
		UINT width = rc.right - static_cast<UINT>(rc.left);
		UINT height = rc.bottom - static_cast<UINT>(rc.top);

		UINT createDeviceFlags = 0;

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

			if (hr == E_INVALIDARG)
			{
				// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
				hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
					D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
			}

			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return hr;

		// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
		ComPtr<IDXGIFactory1>           dxgiFactory(nullptr);
		{
			ComPtr<IDXGIDevice>           dxgiDevice(nullptr);
			hr = m_d3dDevice.As(&dxgiDevice);
			if (SUCCEEDED(hr))
			{
				ComPtr<IDXGIAdapter>           adapter(nullptr);

				hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
				if (SUCCEEDED(hr))
				{
					hr = adapter->GetParent(__uuidof(IDXGIFactory1), (&dxgiFactory));
				}
			}
		}
		if (FAILED(hr))
			return hr;

		// Create swap chain
		ComPtr<IDXGIFactory2>           dxgiFactory2(nullptr);
		hr = dxgiFactory.As(&dxgiFactory2);
		if (dxgiFactory2)
		{
			// DirectX 11.1 or later
			hr = m_d3dDevice.As(&m_d3dDevice1);
			if (SUCCEEDED(hr))
			{
				hr = m_immediateContext.As(&m_immediateContext1);
			}

			DXGI_SWAP_CHAIN_DESC1 sd = {};
			sd.Width = width;
			sd.Height = height;
			sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 1;

			hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
			if (SUCCEEDED(hr))
			{
				hr = m_swapChain1.As(&m_swapChain);
			}
		}
		else
		{
			// DirectX 11.0 systems
			DXGI_SWAP_CHAIN_DESC sd = {};
			sd.BufferCount = 1;
			sd.BufferDesc.Width = width;
			sd.BufferDesc.Height = height;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hWnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;

			hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
		}

		// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
		dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);


		if (FAILED(hr))
			return hr;

		// Create a render target view
		ComPtr<ID3D11Texture2D>           pBackBuffer(nullptr);

		hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (&pBackBuffer));
		if (FAILED(hr))
			return hr;

		hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
		if (FAILED(hr))
			return hr;

		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_immediateContext->RSSetViewports(1, &vp);


		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;


		hr = m_d3dDevice->CreateTexture2D(&descDepth, NULL, m_depthStencil.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		// Initialize the view matrix
		XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
		XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		//m_view = XMMatrixLookAtLH(Eye, At, Up);
		//m_camera = XMMatrixLookAtLH(Eye, At, Up);
		


		// Initializing Objects
		std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::iterator itRender;
		for (itRender = m_renderables.begin(); itRender != m_renderables.end(); itRender++)
		{
			itRender->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
		}

		std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>::iterator itVertex;
		for (itVertex = m_vertexShaders.begin(); itVertex != m_vertexShaders.end(); itVertex++)
		{
			itVertex->second->Initialize(m_d3dDevice.Get());
		}

		std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>::iterator itPixel;
		for (itPixel = m_pixelShaders.begin(); itPixel != m_pixelShaders.end(); itPixel++)
		{
			itPixel->second->Initialize(m_d3dDevice.Get());
		}


		float NearZ = 0.01f;
		float FarZ = 100.0f;

		m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, NearZ, FarZ);

		//**************************change*********************

		// Create the projection matrix
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(CBChangeOnResize);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0u;

		hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		CBChangeOnResize cbChangesOnResize;
		cbChangesOnResize.Projection = XMMatrixTranspose(m_projection);
		m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cbChangesOnResize, 0, 0);



		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddRenderable
	  Summary:  Add a renderable object and initialize the object
	  Args:     PCWSTR pszRenderableName
				  Key of the renderable object
				const std::shared_ptr<Renderable>& renderable
				  Unique pointer to the renderable object
	  Modifies: [m_renderables].
	  Returns:  HRESULT
				  Status code.

				  **********change**********

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable object
				const std::shared_ptr<Renderable>& renderable
				  Unique pointer to the renderable object

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	HRESULT Renderer::AddRenderable(PCWSTR pszRenderableName, const std::shared_ptr<Renderable>& renderable)
	{
		if (m_renderables.contains(pszRenderableName))
		{
			return E_FAIL;
		}
		else
		{
			m_renderables.insert(std::make_pair(pszRenderableName, renderable));

			return S_OK;
		}
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddVertexShader
	  Summary:  Add the vertex shader into the renderer
	  Args:     PCWSTR pszVertexShaderName
				  Key of the vertex shader
				const std::shared_ptr<VertexShader>&
				  Vertex shader to add
	  Modifies: [m_vertexShaders].
	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	HRESULT Renderer::AddVertexShader(PCWSTR pszVertexShaderName, const std::shared_ptr<VertexShader>& vertexShader)
	{
		if (m_vertexShaders.contains(pszVertexShaderName))
		{
			return E_FAIL;
		}
		else
		{
			m_vertexShaders.insert(std::make_pair(pszVertexShaderName, vertexShader));

			return S_OK;
		}
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddPixelShader
	  Summary:  Add the pixel shader into the renderer
	  Args:     PCWSTR pszPixelShaderName
				  Key of the pixel shader
				const std::shared_ptr<PixelShader>&
				  Pixel shader to add
	  Modifies: [m_pixelShaders].
	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	HRESULT Renderer::AddPixelShader(PCWSTR pszPixelShaderName, const std::shared_ptr<PixelShader>& pixelShader)
	{
		if (m_pixelShaders.contains(pszPixelShaderName))
		{
			return E_FAIL;
		}
		else
		{
			m_pixelShaders.insert(std::make_pair(pszPixelShaderName, pixelShader));

			return S_OK;
		}
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::HandleInput

	  Summary:  Add the pixel shader into the renderer and initialize it

	  Args:     const DirectionsInput& directions
				  Data structure containing keyboard input data
				const MouseRelativeMovement& mouseRelativeMovement
				  Data structure containing mouse relative input data

	  Modifies: [m_camera].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	/*--------------------------------------------------------------------
	  TODO: Renderer::HandleInput definition (remove the comment)
	--------------------------------------------------------------------*/

	void Renderer::HandleInput(
		_In_ const DirectionsInput& directions,
		_In_ const MouseRelativeMovement& mouseRelativeMovement,
		_In_ FLOAT deltaTime
	)
	{
		m_camera.HandleInput(
			directions,
			mouseRelativeMovement,
			deltaTime
		);
	}





	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Update
	  Summary:  Update the renderables each frame
	  Args:     FLOAT deltaTime
				  Time difference of a frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	void Renderer::Update(FLOAT deltaTime)
	{
		std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::iterator it;
		for (it = m_renderables.begin(); it != m_renderables.end(); it++)
		{
			it->second->Update(deltaTime);
		}
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Render
	  Summary:  Render the frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	
	void Renderer::Render()
	{
		// Just clear the backbuffer
		m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

		// Clear depth stencil view
		m_immediateContext->ClearDepthStencilView(
			m_depthStencilView.Get(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0
		);

		D3D11_BUFFER_DESC bd = {};
		
		bd.ByteWidth = sizeof(CBChangeOnCameraMovement);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0u;

		if (FAILED(m_d3dDevice->CreateBuffer(&bd, nullptr, m_camera.GetConstantBuffer().GetAddressOf())))
		{}

		// Create camera constant buffer and update
		CBChangeOnCameraMovement cbChangeOnCameraMovement =
		{
			.View = XMMatrixTranspose(m_camera.GetView())
		};
		m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cbChangeOnCameraMovement, 0u, 0u);


		std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::iterator itRender;
		for (itRender = m_renderables.begin(); itRender != m_renderables.end(); ++itRender)
		{
			// Create renderable constant buffer and update
			D3D11_BUFFER_DESC bd =
			{
				.ByteWidth = sizeof(CBChangesEveryFrame),
				.Usage = D3D11_USAGE_DEFAULT,
				.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
				.CPUAccessFlags = 0u
			};
			if (FAILED(m_d3dDevice->CreateBuffer(&bd, nullptr, itRender->second->GetConstantBuffer().GetAddressOf())))
			{}

			CBChangesEveryFrame cbChangesEveryFrame =
			{
				.World = XMMatrixTranspose(itRender->second->GetWorldMatrix())
			};
			m_immediateContext->UpdateSubresource(itRender->second->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

			// Set up the IA stage by setting the input topology and layout.
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;

			m_immediateContext->IASetVertexBuffers(
				0u,
				1u,
				itRender->second->GetVertexBuffer().GetAddressOf(),
				&stride,
				&offset
			);
	
			m_immediateContext->IASetIndexBuffer(
				itRender->second->GetIndexBuffer().Get(),
				DXGI_FORMAT_R16_UINT,
				0
			);

			m_immediateContext->IASetInputLayout(itRender->second->GetVertexLayout().Get());


			// Set primitive topology
			m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Render the triangles
			// Set up the vertex shader stage.
			m_immediateContext->VSSetShader(
				itRender->second->GetVertexShader().Get(),
				nullptr,
				0
			);
			m_immediateContext->VSSetConstantBuffers(
				0,
				1,
				m_camera.GetConstantBuffer().GetAddressOf()
			);

			m_immediateContext->VSSetConstantBuffers(
				1,
				1,
				m_cbChangeOnResize.GetAddressOf()
			);
			m_immediateContext->VSSetConstantBuffers(
				2,
				1,
				itRender->second->GetConstantBuffer().GetAddressOf()
			);

			// Set up the pixel shader stage.

			m_immediateContext->PSSetShader(
				itRender->second->GetPixelShader().Get(),
				nullptr,
				0
			);
			m_immediateContext->PSSetConstantBuffers(
				2, 
				1, 
				itRender->second->GetConstantBuffer().GetAddressOf()
			);
			m_immediateContext->PSSetShaderResources(
				0,
				1,
				itRender->second->GetTextureResourceView().GetAddressOf()
			);
			m_immediateContext->PSSetSamplers(
				0, 
				1, 
				itRender->second->GetSamplerState().GetAddressOf()
			);

			// Calling Draw tells Direct3D to start sending commands to the graphics device.
			m_immediateContext->DrawIndexed(
				itRender->second->GetNumIndices(),
				0, 
				0
			);
		}


		m_swapChain->Present(0, 0);
	}

	

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetVertexShaderOfRenderable
	  Summary:  Sets the vertex shader for a renderable
	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszVertexShaderName
				  Key of the vertex shader
	  Modifies: [m_renderables].
	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	HRESULT Renderer::SetVertexShaderOfRenderable(PCWSTR pszRenderableName, PCWSTR pszVertexShaderName)
	{
		if (!m_renderables.contains(pszRenderableName))
		{
			return E_FAIL;
		}
		else
		{
			if (m_vertexShaders.contains(pszVertexShaderName))
			{
				m_renderables.find(pszRenderableName)->second->SetVertexShader(m_vertexShaders.find(pszVertexShaderName)->second);
				return S_OK;
			}

			return E_FAIL;

		}
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetPixelShaderOfRenderable
	  Summary:  Sets the pixel shader for a renderable
	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszPixelShaderName
				  Key of the pixel shader
	  Modifies: [m_renderables].
	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	HRESULT Renderer::SetPixelShaderOfRenderable(PCWSTR pszRenderableName, PCWSTR pszPixelShaderName)
	{
		if (!m_renderables.contains(pszRenderableName))
		{
			return E_FAIL;
		}
		else
		{
			if (m_vertexShaders.contains(pszPixelShaderName))
			{
				m_renderables.find(pszRenderableName)->second->SetPixelShader(m_pixelShaders.find(pszPixelShaderName)->second);
				return S_OK;
			}

			return E_FAIL;

		}
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::GetDriverType
	  Summary:  Returns the Direct3D driver type
	  Returns:  D3D_DRIVER_TYPE
				  The Direct3D driver type used
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	D3D_DRIVER_TYPE Renderer::GetDriverType() const
	{
		return m_driverType;
	}
}