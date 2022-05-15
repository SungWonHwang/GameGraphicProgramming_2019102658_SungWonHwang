#include "Renderer/Renderer.h"

namespace library
{
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Renderer

	  Summary:  Constructor

	  Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
				 m_immediateContext, m_immediateContext1, m_swapChain,
				 m_swapChain1, m_renderTargetView, m_depthStencil,
				 m_depthStencilView, m_cbChangeOnResize, m_camera,
				 m_projection, m_renderables, m_vertexShaders,
				 m_pixelShaders].

	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	Renderer::Renderer() :
		m_driverType(D3D_DRIVER_TYPE_HARDWARE),
		m_featureLevel(D3D_FEATURE_LEVEL_11_1),
		m_d3dDevice(),
		m_d3dDevice1(),
		m_immediateContext(),
		m_immediateContext1(),
		m_swapChain(),
		m_swapChain1(),
		m_renderTargetView(),
		m_depthStencil(),
		m_depthStencilView(),
		m_cbChangeOnResize(),
		m_cbLights(),
		m_pszMainSceneName(),
		m_camera(XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f)),
		m_projection(),

		m_renderables(),
		m_aPointLights(),
		m_vertexShaders(),
		m_pixelShaders(),
		m_scenes()
	{}



	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Initialize
	  Summary:  Creates Direct3D device and swap chain
	  Args:     HWND hWnd
				  Handle to the window

	 Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
				 m_d3dDevice1, m_immediateContext1, m_swapChain1,
				 m_swapChain, m_renderTargetView, m_cbChangeOnResize,
				 m_projection, m_cbLights, m_camera, m_vertexShaders,
				 m_pixelShaders, m_renderables].
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

		// Create the depth stencil OMSetRenderTargets
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


		//**************************change*********************
		//about lights
		D3D11_BUFFER_DESC bdLight = {};
		bdLight.ByteWidth = sizeof(CBLights);
		bdLight.Usage = D3D11_USAGE_DEFAULT;
		bdLight.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bdLight.CPUAccessFlags = 0u;
		if (FAILED(m_d3dDevice->CreateBuffer(&bdLight, nullptr, m_cbLights.GetAddressOf())))
		{
		}

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

		//**************************change*********************
		std::unordered_map<std::wstring, std::shared_ptr<Scene>>::iterator scene;
		for (scene = m_scenes.begin(); scene != m_scenes.end(); scene++)
		{
			for (UINT i = 0; i < scene->second->GetVoxels().size(); i++)
			{
				scene->second->GetVoxels()[i]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
			}
		}

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	   Method:   Renderer::AddRenderable

      Summary:  Add a renderable object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Shared pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
	{
		if (m_renderables.count(pszRenderableName) > 0) return E_FAIL;
		m_renderables.insert({ pszRenderableName, renderable });

		return S_OK;
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


	HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
	{
		if (m_vertexShaders.count(pszVertexShaderName) > 0) return E_FAIL;
		m_vertexShaders.insert({ pszVertexShaderName, vertexShader });

		return S_OK;
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

	HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
	{
		if (m_pixelShaders.count(pszPixelShaderName) > 0) return E_FAIL;
		m_pixelShaders.insert({ pszPixelShaderName, pixelShader });

		return S_OK;
	}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Method:   Renderer::AddPointLight

	 Summary:  Add a point light

	 Args:     size_t index
			  Index of the point light
			const std::shared_ptr<PointLight>& pointLight
			  Shared pointer to the point light object

	Modifies: [m_aPointLights].
	m_aPointLights
	 Returns:  HRESULT
			  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	/*--------------------------------------------------------------------
	 TODO: Renderer::AddPointLight definition (remove the comment)
	 When the index exceeds the size of possible lights, it returns E_FAIL
	 else, add the light to designated index
	--------------------------------------------------------------------*/

	HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
	{
		if (index >= NUM_LIGHTS)
		{
			return E_FAIL;
		}

		m_aPointLights[index] = pPointLight;

		return S_OK;
	}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddScene

	  Summary:  Add a scene

	  Args:     PCWSTR pszSceneName
				  Key of a scene
				const std::filesystem::path& sceneFilePath
				  File path to initialize a scene

	  Modifies: [m_scenes].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	/*--------------------------------------------------------------------
	  TODO: Renderer::AddScene definition (remove the comment)
	--------------------------------------------------------------------*/
	HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath)
	{
		if (m_scenes.find(pszSceneName) != m_scenes.end())
		{
			return E_FAIL;
		}

		m_scenes.insert(std::make_pair(pszSceneName, std::make_shared<Scene>(sceneFilePath)));

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetMainScene

	  Summary:  Set the main scene

	  Args:     PCWSTR pszSceneName
				  Name of the scene to set as the main scene

	  Modifies: [m_pszMainSceneName].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	/*--------------------------------------------------------------------
	  TODO: Renderer::SetMainScene definition (remove the comment)
	--------------------------------------------------------------------*/
	HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
	{
		if (m_scenes.find(pszSceneName) == m_scenes.end())
		{
			return E_FAIL;
		}

		m_pszMainSceneName = pszSceneName;

		return S_OK;
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


	void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
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

	void Renderer::Update(_In_ FLOAT deltaTime)
	{
		for (auto& renderable : m_renderables)
		{
			renderable.second->Update(deltaTime);
		}

		for (auto& light : m_aPointLights)
		{
			light->Update(deltaTime);
		}
		//********************************change****************
		for (UINT i = 0u; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); ++i)
		{
			m_scenes[m_pszMainSceneName]->GetVoxels()[i]->Update(deltaTime);
		}

		m_camera.Update(deltaTime);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Render
	  Summary:  Render the frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/


	void Renderer::Render()
	{

		// Clear the backbuffer
		const float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
		m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), ClearColor);

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
		{
		}

		//about lights
		D3D11_BUFFER_DESC bdLight = {};
		bdLight.ByteWidth = sizeof(CBLights);
		bdLight.Usage = D3D11_USAGE_DEFAULT;
		bdLight.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bdLight.CPUAccessFlags = 0;
		bdLight.MiscFlags = 0;
		bdLight.StructureByteStride = 0;

		if (FAILED(m_d3dDevice->CreateBuffer(&bdLight, nullptr, m_cbLights.GetAddressOf())))
		{
		}

		CBLights cbLights;
		for (int j = 0; j < NUM_LIGHTS; j++)
		{
			cbLights.LightPositions[j] = m_aPointLights[j]->GetPosition();
			cbLights.LightColors[j] = m_aPointLights[j]->GetColor();
			//MessageBox(nullptr, L"Fail", L"gg", NULL);
		}
		m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0u, 0u);



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
			{
			}

			CBChangesEveryFrame cbChangesEveryFrame =
			{
				.World = XMMatrixTranspose(itRender->second->GetWorldMatrix()),
				.OutputColor = itRender->second->GetOutputColor()
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
			m_immediateContext->VSSetConstantBuffers(
				3,
				1,
				m_cbLights.GetAddressOf()
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
			m_immediateContext->PSSetConstantBuffers(
				3,
				1,
				m_cbLights.GetAddressOf()
			);

			/*
			//AboutTexture

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
			*/

			
			/*
			if (itRender->second->HasTexture())
			{
				for (UINT i = 0u; i < itRender->second->GetNumMeshes(); ++i)
				{
					const UINT materialIndex = itRender->second->GetMesh(i).uMaterialIndex;
					if (itRender->second->GetMaterial(materialIndex).pDiffuse)
					{
						m_immediateContext->PSSetShaderResources(0u, 1u, itRender->second->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());
						m_immediateContext->PSSetSamplers(0u, 1u, itRender->second->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());
					}
					m_immediateContext->DrawIndexed(
						itRender->second->GetMesh(i).uNumIndices,
						itRender->second->GetMesh(i).uBaseIndex,
						itRender->second->GetMesh(i).uBaseVertex);
				}
			}
			else
			{
				m_immediateContext->DrawIndexed(itRender->second->GetNumIndices(), 0u, 0);
			}
			*/
			
			
		}
		//*********************************PLUS*****************************
		// voxels 
		std::vector<std::shared_ptr<Voxel>>::iterator voxel;
		for (voxel = m_scenes[m_pszMainSceneName]->GetVoxels().begin(); voxel != m_scenes[m_pszMainSceneName]->GetVoxels().end(); ++voxel)
		{
			// vertex buffer
			UINT uStride = sizeof(SimpleVertex);
			UINT uOffset = 0;
			m_immediateContext->IASetVertexBuffers(
				0,
				1,
				voxel->get()->GetVertexBuffer().GetAddressOf(),
				&uStride,
				&uOffset
			);

			// instance buffer
			uStride = sizeof(InstanceData);
			m_immediateContext->IASetVertexBuffers(
				1,
				1,
				voxel->get()->GetInstanceBuffer().GetAddressOf(),
				&uStride,
				&uOffset
			);

			// index buffer
			m_immediateContext->IASetIndexBuffer(
				voxel->get()->GetIndexBuffer().Get(),
				DXGI_FORMAT_R16_UINT, 0u
			);

			// Input Layout
			m_immediateContext->IASetInputLayout(
				voxel->get()->GetVertexLayout().Get()
			);

			//Set Primitive Topology
			m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Update
			CBChangesEveryFrame cbChangesEveryFrame =
			{
				.World = XMMatrixTranspose(voxel->get()->GetWorldMatrix()),
				.OutputColor = voxel->get()->GetOutputColor()
			};
			m_immediateContext->UpdateSubresource(voxel->get()->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

			//vertex
			m_immediateContext->VSSetShader(
				voxel->get()->GetVertexShader().Get(),
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
				voxel->get()->GetConstantBuffer().GetAddressOf()
			);

			//pixel
			m_immediateContext->PSSetShader(
				voxel->get()->GetPixelShader().Get(),
				nullptr, 
				0
			);
			m_immediateContext->PSSetConstantBuffers(
				0, 
				1, 
				m_camera.GetConstantBuffer().GetAddressOf()
			);
			m_immediateContext->PSSetConstantBuffers(
				2, 
				1, 
				voxel->get()->GetConstantBuffer().GetAddressOf()
			);
			m_immediateContext->PSSetConstantBuffers(
				3,
				1, 
				m_cbLights.GetAddressOf()
			);

			//triangle
			m_immediateContext->DrawIndexedInstanced(
				voxel->get()->GetNumIndices(),
				voxel->get()->GetNumInstances(),
				0,
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
	  Method:   Renderer::SetVertexShaderOfScene

	  Summary:  Sets the vertex shader for the voxels in a scene

	  Args:     PCWSTR pszSceneName
				  Key of the scene
				PCWSTR pszVertexShaderName
				  Key of the vertex shader

	  Modifies: [m_scenes].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	/*--------------------------------------------------------------------
	  TODO: Renderer::SetVertexShaderOfScene definition (remove the comment)
	--------------------------------------------------------------------*/
	HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
	{
		if (m_scenes.find(pszSceneName) == m_scenes.end())
		{
			return E_FAIL;
		}
		if (m_vertexShaders.find(pszVertexShaderName) == m_vertexShaders.end())
		{
			return E_FAIL;
		}

		for (UINT i = 0u; i < m_scenes[pszSceneName]->GetVoxels().size(); ++i)
		{
			m_scenes[pszSceneName]->GetVoxels()[i]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
		}

		return S_OK;
	}
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetPixelShaderOfScene

	  Summary:  Sets the pixel shader for the voxels in a scene

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszPixelShaderName
				  Key of the pixel shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	/*--------------------------------------------------------------------
	  TODO: Renderer::SetPixelShaderOfScene definition (remove the comment)
	--------------------------------------------------------------------*/
	HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
	{
		if (m_scenes.find(pszSceneName) == m_scenes.end())
		{
			return E_FAIL;
		}
		if (m_pixelShaders.find(pszPixelShaderName) == m_pixelShaders.end())
		{
			return E_FAIL;
		}

		for (UINT i = 0u; i < m_scenes[pszSceneName]->GetVoxels().size(); ++i)
		{
			m_scenes[pszSceneName]->GetVoxels()[i]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
		}

		return S_OK;
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