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

                ********change*****
                
    Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,

                  m_swapChain1, m_renderTargetView, m_depthStencil,
                  m_depthStencilView, m_cbChangeOnResize, m_cbShadowMatrix,
                  m_pszMainSceneName, m_camera, m_projection, m_scenes
                  m_invalidTexture, m_shadowMapTexture, m_shadowVertexShader,
                  m_shadowPixelShader].
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer() :
        m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_depthStencil(nullptr)
        , m_depthStencilView(nullptr)
        , m_cbChangeOnResize(nullptr)
        , m_cbLights(nullptr)
        , m_pszMainSceneName(L"Default")
        , m_padding()
        , m_camera(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f))
        , m_projection(XMMatrixIdentity())
        //, m_renderables(std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>())
        //, m_models(std::unordered_map<PCWSTR, std::shared_ptr<Model>>())
        //, m_aPointLights()
        //, m_vertexShaders(std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>())
        //, m_pixelShaders(std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>())
        , m_scenes(std::unordered_map<std::wstring, std::shared_ptr<Scene>>())
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
        , m_cbShadowMatrix(nullptr)
        , m_shadowMapTexture()
        , m_shadowVertexShader()
        , m_shadowPixelShader()
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


                  ********change*****
                  
      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView, m_vertexShader,
                  m_vertexLayout, m_pixelShader, m_vertexBuffer
                  m_cbShadowMatrix].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - static_cast<UINT>(rc.left);
        UINT height = rc.bottom - static_cast<UINT>(rc.top);


        POINT p1, p2;
        p1.x = rc.left;
        p1.y = rc.top;
        p2.x = rc.right;
        p2.y = rc.bottom;

        ClientToScreen(hWnd, &p1);
        ClientToScreen(hWnd, &p2);

        rc.left = p1.x;
        rc.top = p1.y;
        rc.right = p2.x;
        rc.bottom = p2.y;

        ClipCursor(&rc);

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
        {
            return hr;
        }

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

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc =
                {
                    .Width = width,
                    .Height = height,
                    .RefreshRate = {.Numerator = 60, .Denominator = 1 },
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                },
                .SampleDesc = {.Count = 1, .Quality = 1 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = hWnd,
                .Windowed = TRUE,
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }


        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D>           pBackBuffer(nullptr);

        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
        };

        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0, }
        };

        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)width,
            .Height = (FLOAT)height,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };


        m_immediateContext->RSSetViewports(1, &vp);



        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(width) / static_cast<FLOAT>(height), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_camera.Initialize(m_d3dDevice.Get());
        
        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }
        

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create shadow matrix constant buffer
        bd.ByteWidth = sizeof(CBShadowMatrix);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbShadowMatrix.GetAddressOf());
        if (FAILED(hr))
        {

            return hr;
        }

        // Initialize the shadow map texture
        m_shadowMapTexture = std::make_shared<RenderTexture>(width, height);
        m_shadowMapTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

        // Initialize the point lights of main scene
        for (UINT i = 0; i < NUM_LIGHTS; ++i)
        {
            m_scenes[m_pszMainSceneName]->GetPointLight(i)->Initialize(width, height);
        }

        return hr;
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
    /*
    HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
    {
        if (m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }
        else
        {
            m_renderables[pszRenderableName] = renderable;
            return S_OK;
        }
    }
    */

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

    /*
    HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        if (m_vertexShaders.contains(pszVertexShaderName))
        {
            return E_FAIL;
        }
        else
        {
            m_vertexShaders[pszVertexShaderName] = vertexShader;
            return S_OK;
        }
    }
    */

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
    /*
    HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        if (m_pixelShaders.contains(pszPixelShaderName))
        {
            return E_FAIL;
        }
        else
        {
            m_pixelShaders[pszPixelShaderName] = pixelShader;
            return S_OK;
        }
    }
    */



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
    /*
    HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pointLight)
    {
        if (index >= NUM_LIGHTS)
        {
            return E_FAIL;
        }
        else
        {
            m_aPointLights[index] = pointLight;
            return S_OK;
        }
    }
    */

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
     Method:   Renderer::AddScene

     Summary:  Add scene to renderer

     Args:     PCWSTR pszSceneName
                 The name of the scene
               const std::shared_ptr<Scene>&
                 The shared pointer to Scene

     Modifies: [m_scenes].

     Returns:  HRESULT
                 Status code
   M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddModel
      Summary:  Add a model object
      Args:     PCWSTR pszModelName
                  Key of the model object
                const std::shared_ptr<Model>& pModel
                  Shared pointer to the model object
      Modifies: [m_models].
      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddModel definition (remove the comment)
    --------------------------------------------------------------------*/
    /*
    HRESULT Renderer::AddModel(_In_ PCWSTR pszModelName, _In_ const std::shared_ptr<Model>& pModel)
    {
        if (m_models.contains(pszModelName))
        {
            return E_FAIL;
        }
        else
        {
            m_models[pszModelName] = pModel;
            return S_OK;
        }
    }
    */

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull

      Summary:  Return scene with the given name or null

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }
        else
        {
            m_pszMainSceneName = pszSceneName;
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetShadowMapShaders

      Summary:  Set shaders for the shadow mapping

      Args:     std::shared_ptr<ShadowVertexShader>
                  vertex shader
                std::shared_ptr<PixelShader>
                  pixel shader

      Modifies: [m_shadowVertexShader, m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::SetShadowMapShaders(_In_ std::shared_ptr<ShadowVertexShader> vertexShader, _In_ std::shared_ptr<PixelShader> pixelShader)
    {
        m_shadowVertexShader = move(vertexShader);
        m_shadowPixelShader = move(pixelShader);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Handle user mouse input

      Args:     DirectionsInput& directions
                MouseRelativeMovement& mouseRelativeMovement
                FLOAT deltaTime
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(
        _In_ const DirectionsInput& directions,
        _In_ const MouseRelativeMovement& mouseRelativeMovement,
        _In_ FLOAT deltaTime)
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
        /*
        for (auto& renderable : m_renderables)
        {
            renderable.second->Update(deltaTime);
        }

        for (auto& light : m_aPointLights)
        {
            light->Update(deltaTime);
        }
        //********************************SCENE****************
        for (UINT i = 0u; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); ++i)
        {
            m_scenes[m_pszMainSceneName]->GetVoxels()[i]->Update(deltaTime);
        }

        // Update the voxels
        for (auto voxelElem = m_scenes.begin(); voxelElem != m_scenes.end(); ++voxelElem)
        {
            for (int i = 0; i < voxelElem->second->GetVoxels().size(); ++i)
            {
                voxelElem->second->GetVoxels()[i]->Update(deltaTime);
            }
        }

        //********************************MODEL****************

        for (auto it : m_models) {
            it.second->Update(deltaTime);
        }
        */
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_camera.Update(deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Render definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::Render()
	{

        RenderSceneToTexture();


		// Clear the backbuffer
		m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

		// Clear the depth buffer to 1.0 (max depth)
		m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        // create camera constant buffer and update
        XMFLOAT4 camPosition;
        XMStoreFloat4(&camPosition, m_camera.GetEye());
        CBChangeOnCameraMovement cbView = {
                .View = XMMatrixTranspose(m_camera.GetView()),
                .CameraPosition = camPosition
        };
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbView, 0, 0);


        // update the lights constant buffer
        // modify
        CBLights cbLight = {
            .LightPositions = {},
            .LightColors = {},
            .LightViews = {},
            .LightProjections = {}
        };
        for (int lightIndex = 0; lightIndex < NUM_LIGHTS; lightIndex++)
        {
            cbLight.LightPositions[lightIndex] = m_scenes[m_pszMainSceneName]->GetPointLight(lightIndex)->GetPosition();
            cbLight.LightColors[lightIndex] = m_scenes[m_pszMainSceneName]->GetPointLight(lightIndex)->GetColor();
            cbLight.LightViews[lightIndex] = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(lightIndex)->GetViewMatrix());
            cbLight.LightProjections[lightIndex] = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(lightIndex)->GetProjectionMatrix());
        }
        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLight, 0, 0);



        // for each renderable
        for (auto renderablesElem : m_scenes[m_pszMainSceneName]->GetRenderables())
        {
            // set the buffers
            UINT aStrides[2] = {
                static_cast<UINT>(sizeof(SimpleVertex)),
                static_cast<UINT>(sizeof(NormalData))
            };
            UINT aOffsets[2] = { 0u,0u };
            ComPtr<ID3D11Buffer> aBuffers[2]{
                renderablesElem.second->GetVertexBuffer().Get(),
                renderablesElem.second->GetNormalBuffer().Get()
            };
            m_immediateContext->IASetVertexBuffers(0, 2, aBuffers->GetAddressOf(), aStrides, aOffsets);

            // index buffer
            m_immediateContext->IASetIndexBuffer(renderablesElem.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

            // set the input layout
            m_immediateContext->IASetInputLayout(renderablesElem.second->GetVertexLayout().Get());

            // update the constant buffers
            CBChangesEveryFrame cbWorld = {
                .World = XMMatrixTranspose(renderablesElem.second->GetWorldMatrix()),
                .OutputColor = renderablesElem.second->GetOutputColor(),
                .HasNormalMap = renderablesElem.second->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(renderablesElem.second->GetConstantBuffer().Get(), 0, nullptr, &cbWorld, 0, 0);

            // set the shaders and their input
            m_immediateContext->VSSetShader(renderablesElem.second->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, renderablesElem.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, renderablesElem.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetShader(renderablesElem.second->GetPixelShader().Get(), nullptr, 0);
            m_immediateContext->PSSetShaderResources(2, 1, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
            m_immediateContext->PSSetSamplers(2, 1, m_shadowMapTexture->GetSamplerState().GetAddressOf());

            if (renderablesElem.second->HasTexture())
            {
                for (UINT i = 0u; i < renderablesElem.second->GetNumMeshes(); ++i)
                {
                    UINT iMaterial = renderablesElem.second->GetMesh(i).uMaterialIndex;

                    // sest pixel shader resources and sampler states
                    /*
                    ComPtr<ID3D11ShaderResourceView> textures[2] = {
                        renderablesElem.second->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().Get(),
                        renderablesElem.second->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().Get()
                    };
                    ComPtr<ID3D11SamplerState> samplerStates[2] = {
                        renderablesElem.second->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().Get(),
                        renderablesElem.second->GetMaterial(iMaterial)->pNormal->GetSamplerState().Get()
                    };
                    m_immediateContext->PSSetShaderResources(0, 2, textures->GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                    */
                    // set pixel shader resources and sampler states
                    if (renderablesElem.second->GetMaterial(iMaterial)->pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(0u, 1u, renderablesElem.second->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(0u, 1u, renderablesElem.second->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().GetAddressOf());
                    }
                    if (renderablesElem.second->GetMaterial(iMaterial)->pNormal)
                    {
                        m_immediateContext->PSSetShaderResources(1u, 1u, renderablesElem.second->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(1u, 1u, renderablesElem.second->GetMaterial(iMaterial)->pNormal->GetSamplerState().GetAddressOf());
                    }

                    // draw
                    m_immediateContext->DrawIndexed(renderablesElem.second->GetMesh(i).uNumIndices, renderablesElem.second->GetMesh(i).uBaseIndex, renderablesElem.second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(renderablesElem.second->GetNumIndices(), 0, 0);
            }

        }
        // for each voxel of main scene
        for (auto voxelElem : m_scenes[m_pszMainSceneName]->GetVoxels())
        {
            // set the buffers
            // 
            // vertex buffer with normal data and instance data

            UINT aStrides[3] = {
                static_cast<UINT>(sizeof(SimpleVertex)),
                static_cast<UINT>(sizeof(NormalData)),
                static_cast<UINT>(sizeof(InstanceData))
            };
            UINT aOffsets[3] = { 0u,0u,0u };
            ComPtr<ID3D11Buffer> aBuffers[3]{
                voxelElem->GetVertexBuffer().Get(),
                voxelElem->GetNormalBuffer().Get(),
                voxelElem->GetInstanceBuffer().Get()
            };
            m_immediateContext->IASetVertexBuffers(0, 3, aBuffers->GetAddressOf(), aStrides, aOffsets);

            // index buffer
            m_immediateContext->IASetIndexBuffer(voxelElem->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

            // set the input layout
            m_immediateContext->IASetInputLayout(voxelElem->GetVertexLayout().Get());

            // update the constant buffer

            CBChangesEveryFrame cbWorld = {
                .World = XMMatrixTranspose(voxelElem->GetWorldMatrix()),
                .OutputColor = voxelElem->GetOutputColor(),
                .HasNormalMap = voxelElem->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(voxelElem->GetConstantBuffer().Get(), 0, nullptr, &cbWorld, 0, 0);


            // set the shaders and their input
            m_immediateContext->VSSetShader(voxelElem->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, voxelElem->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, voxelElem->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetShader(voxelElem->GetPixelShader().Get(), nullptr, 0);
            m_immediateContext->PSSetShaderResources(2, 1, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
            m_immediateContext->PSSetSamplers(2, 1, m_shadowMapTexture->GetSamplerState().GetAddressOf());


            // draw 
            if (voxelElem->HasTexture())
            {
                for (UINT i = 0u; i < voxelElem->GetNumMeshes(); ++i)
                {
                    UINT iMaterial = voxelElem->GetMesh(i).uMaterialIndex;

                    // sest pixel shader resources and sampler states
                    /*
                    ComPtr<ID3D11ShaderResourceView> textures[2] = {
                        voxelElem->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().Get(),
                        voxelElem->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().Get()
                    };
                    ComPtr<ID3D11SamplerState> samplerStates[2] = {
                        voxelElem->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().Get(),
                        voxelElem->GetMaterial(iMaterial)->pNormal->GetSamplerState().Get()
                    };
                    m_immediateContext->PSSetShaderResources(0, 2, textures->GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                    */
                    // set pixel shader resources and sampler states
                    if (voxelElem->GetMaterial(iMaterial)->pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(0u, 1u, voxelElem->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(0u, 1u, voxelElem->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().GetAddressOf());
                    }
                    if (voxelElem->GetMaterial(iMaterial)->pNormal)
                    {
                        m_immediateContext->PSSetShaderResources(1u, 1u, voxelElem->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(1u, 1u, voxelElem->GetMaterial(iMaterial)->pNormal->GetSamplerState().GetAddressOf());
                    }

                    // draw
                    //m_immediateContext->DrawIndexedInstanced(voxelElem->GetMesh(i).uNumIndices, voxelElem->GetNumInstances(), 0, 0, 0);
                    for (UINT i = 0u; i < voxelElem->GetNumMeshes(); ++i)
                    {
                        m_immediateContext->DrawIndexed(voxelElem->GetMesh(i).uNumIndices, voxelElem->GetMesh(i).uBaseIndex, voxelElem->GetMesh(i).uBaseVertex);
                    }
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(voxelElem->GetNumIndices(), 0, 0);
            }
        }





        // for each model
        for (auto modelElem  : m_scenes[m_pszMainSceneName]->GetModels())
        {
            // Set the vertex buffer
            UINT aStrides[2] =
            {
                static_cast<UINT>(sizeof(SimpleVertex)),
                static_cast<UINT>(sizeof(AnimationData)),
            };

            UINT aOffsets[2] = { 0u, 0u };


            ComPtr<ID3D11Buffer> aBuffers[2]{
                modelElem.second->GetVertexBuffer().Get(),
                modelElem.second->GetNormalBuffer().Get()
            };


            m_immediateContext->IASetVertexBuffers(0, 2, aBuffers->GetAddressOf(), aStrides, aOffsets);


            // Set the index buffer 
            m_immediateContext->IASetIndexBuffer(modelElem.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);



            // Set the input layout
            m_immediateContext->IASetInputLayout(modelElem.second->GetVertexLayout().Get());


            // update the constant buffers
            CBChangesEveryFrame cbWorld = {
                .World = XMMatrixTranspose(modelElem.second->GetWorldMatrix()),
                .OutputColor = modelElem.second->GetOutputColor(),
                .HasNormalMap = modelElem.second->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(modelElem.second->GetConstantBuffer().Get(), 0, nullptr, &cbWorld, 0, 0);


            // set the shaders and their input
            m_immediateContext->VSSetShader(modelElem.second->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, modelElem.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, modelElem.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetShader(modelElem.second->GetPixelShader().Get(), nullptr, 0);
            m_immediateContext->PSSetShaderResources(2, 1, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
            m_immediateContext->PSSetSamplers(2, 1, m_shadowMapTexture->GetSamplerState().GetAddressOf());

            if (modelElem.second->HasTexture())
            {
                for (UINT i = 0u; i < modelElem.second->GetNumMeshes(); ++i)
                {
                    UINT iMaterial = modelElem.second->GetMesh(i).uMaterialIndex;
                    // sest pixel shader resources and sampler states
                    /*
                    ComPtr<ID3D11ShaderResourceView> textures[2] = {
                        modelElem.second->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().Get(),
                        modelElem.second->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().Get()
                    };
                    ComPtr<ID3D11SamplerState> samplerStates[2] = {
                        modelElem.second->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().Get(),
                        modelElem.second->GetMaterial(iMaterial)->pNormal->GetSamplerState().Get()
                    };
                    m_immediateContext->PSSetShaderResources(0, 2, textures->GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                    */
                    // set pixel shader resources and sampler states
                    if (modelElem.second->GetMaterial(iMaterial)->pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(0u, 1u, modelElem.second->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(0u, 1u, modelElem.second->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().GetAddressOf());
                    }
                    if (modelElem.second->GetMaterial(iMaterial)->pNormal)
                    {
                        m_immediateContext->PSSetShaderResources(1u, 1u, modelElem.second->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(1u, 1u, modelElem.second->GetMaterial(iMaterial)->pNormal->GetSamplerState().GetAddressOf());
                    }

                    // draw
                    m_immediateContext->DrawIndexed(modelElem.second->GetMesh(i).uNumIndices, modelElem.second->GetMesh(i).uBaseIndex, modelElem.second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                // Draw
                m_immediateContext->DrawIndexed(modelElem.second->GetNumIndices(), 0, 0);
            }
        }


        // present the information rendered to the back buffer to the front buffer

        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetVertexShaderOfModel definition (remove the comment)
    --------------------------------------------------------------------*/
    /*
    HRESULT Renderer::SetVertexShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszVertexShaderName)
    {
        HRESULT hr = S_OK;

        hr = AddVertexShader(pszModelName, m_vertexShaders[pszVertexShaderName]);
        if (FAILED(hr))
            return hr;

        m_models[pszModelName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

        return S_OK;

    }
    */

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetPixelShaderOfModel definition (remove the comment)
    --------------------------------------------------------------------*/
    /*
    HRESULT Renderer::SetPixelShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszPixelShaderName)
    {
        HRESULT hr = S_OK;

        hr = AddPixelShader(pszModelName, m_pixelShaders[pszPixelShaderName]);
        if (FAILED(hr))
            return hr;

        m_models[pszModelName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

        return S_OK;
    }
    */

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
    /*
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        HRESULT hr = S_OK;

        hr = AddVertexShader(pszRenderableName, m_vertexShaders[pszVertexShaderName]);
        if (FAILED(hr))
            return hr;

        m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

        return S_OK;
    }
    */
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
    /*
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
    {
        HRESULT hr = S_OK;

        hr = AddPixelShader(pszRenderableName, m_pixelShaders[pszPixelShaderName]);
        if (FAILED(hr))
            return hr;

        m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

        return S_OK;
    }
    */

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
/*
    HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
    {
        HRESULT hr = S_OK;

        hr = AddVertexShader(pszSceneName, m_vertexShaders[pszVertexShaderName]);
        if (FAILED(hr))
            return hr;

        for (int i = 0; i < m_scenes[pszSceneName]->GetVoxels().size(); ++i)
        {
            m_scenes[pszSceneName]->GetVoxels()[i]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
        }

        return S_OK;
    }
    */

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
/*
    HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
    {
        HRESULT hr = S_OK;

        hr = AddPixelShader(pszSceneName, m_pixelShaders[pszPixelShaderName]);
        if (FAILED(hr))
            return hr;

        for (int i = 0; i < m_scenes[pszSceneName]->GetVoxels().size(); ++i)
        {
            m_scenes[pszSceneName]->GetVoxels()[i]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
        }

        return S_OK;
    }
    */
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

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::RenderSceneToTexture

      Summary:  Render scene to the texture
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::RenderSceneToTexture definition (remove the comment)
    --------------------------------------------------------------------*/

    void Renderer::RenderSceneToTexture()
    {
        //Unbind current pixel shader resources
        ID3D11ShaderResourceView* const pSRV[1] = { NULL };
        m_immediateContext->PSSetShaderResources(0, 1, pSRV);
        m_immediateContext->PSSetShaderResources(1, 1, pSRV);
        m_immediateContext->PSSetShaderResources(2, 1, pSRV);

        // Change render target to the shadow map texture
        m_immediateContext->OMSetRenderTargets(1u, m_shadowMapTexture->GetRenderTargetView().GetAddressOf(), m_depthStencilView.Get());

        // Clear render target view with white color
        m_immediateContext->ClearRenderTargetView(m_shadowMapTexture->GetRenderTargetView().Get(), Colors::White);

        // Clear depth stencil view
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

        // For all renderables
        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::iterator renderable;
        for (renderable = m_scenes[m_pszMainSceneName]->GetRenderables().begin(); renderable != m_scenes[m_pszMainSceneName]->GetRenderables().end(); ++renderable)
        {
            // Bind vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, renderable->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

            // Bind index buffer
            m_immediateContext->IASetIndexBuffer(renderable->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Bind input layout
            m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

            // Update shadow matrix constant buffer
            CBShadowMatrix cbShadowMatrix =
            {
                .World = XMMatrixTranspose(renderable->second->GetWorldMatrix()),
                .View = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0ull)->GetViewMatrix()),
                .Projection = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0ull)->GetProjectionMatrix()),
                .IsVoxel = FALSE
            };
            m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cbShadowMatrix, 0u, 0u);

            // Bind vertex shader and constant buffer
            m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            // Bind pixel shader
            m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

            // Render the triangles
            if (renderable->second->HasTexture())
            {
                for (UINT i = 0; i < renderable->second->GetNumMeshes(); ++i)
                {
                    m_immediateContext->DrawIndexed(renderable->second->GetMesh(i).uNumIndices,
                        renderable->second->GetMesh(i).uBaseIndex,
                        renderable->second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(renderable->second->GetNumIndices(), 0u, 0);
            }
        }

        // For all voxels in main scene
        std::vector<std::shared_ptr<Voxel>>::iterator voxel;
        for (voxel = m_scenes[m_pszMainSceneName]->GetVoxels().begin(); voxel != m_scenes[m_pszMainSceneName]->GetVoxels().end(); ++voxel)
        {
            // Bind vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, voxel->get()->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

            // Bind instance buffer
            uStride = sizeof(InstanceData);
            m_immediateContext->IASetVertexBuffers(2u, 1u, voxel->get()->GetInstanceBuffer().GetAddressOf(), &uStride, &uOffset);

            // Bind index buffer
            m_immediateContext->IASetIndexBuffer(voxel->get()->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Bind input layout
            m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

            // Update shadow matrix constant buffer
            CBShadowMatrix cbShadowMatrix =
            {
                .World = XMMatrixTranspose(voxel->get()->GetWorldMatrix()),
                .View = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0ull)->GetViewMatrix()),
                .Projection = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0ull)->GetProjectionMatrix()),
                .IsVoxel = TRUE
            };
            m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cbShadowMatrix, 0u, 0u);

            // Bind vertex shader and constant buffer
            m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            // Bind pixel shader
            m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

            // Render the triangles
            if (voxel->get()->HasTexture())
            {
                for (UINT i = 0; i < voxel->get()->GetNumMeshes(); ++i)
                {
                    m_immediateContext->DrawIndexedInstanced(voxel->get()->GetMesh(i).uNumIndices,
                        voxel->get()->GetNumInstances(),
                        voxel->get()->GetMesh(i).uBaseIndex,
                        voxel->get()->GetMesh(i).uBaseVertex,
                        0u);
                }
            }
            else
            {
                m_immediateContext->DrawIndexedInstanced(voxel->get()->GetNumIndices(), voxel->get()->GetNumInstances(), 0u, 0, 0u);
            }
        }

        // For all models
        std::unordered_map<std::wstring, std::shared_ptr<Model>>::iterator model;
        for (model = m_scenes[m_pszMainSceneName]->GetModels().begin(); model != m_scenes[m_pszMainSceneName]->GetModels().end(); ++model)
        {
            // Bind vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, model->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

            // Bind index buffer
            m_immediateContext->IASetIndexBuffer(model->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Bind input layout
            m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

            // Update shadow matrix constant buffer
            CBShadowMatrix cbShadowMatrix =
            {
                .World = XMMatrixTranspose(model->second->GetWorldMatrix()),
                .View = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0ull)->GetViewMatrix()),
                .Projection = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0ull)->GetProjectionMatrix()),
                .IsVoxel = FALSE
            };
            m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cbShadowMatrix, 0u, 0u);

            // Bind vertex shader and constant buffer
            m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            // Bind pixel shader
            m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

            // Render the triangles
            if (model->second->HasTexture())
            {
                for (UINT i = 0; i < model->second->GetNumMeshes(); ++i)
                {
                    m_immediateContext->DrawIndexed(model->second->GetMesh(i).uNumIndices,
                        model->second->GetMesh(i).uBaseIndex,
                        model->second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(model->second->GetNumIndices(), 0u, 0);
            }
        }

        // Reset the render target to the original back buffer
        m_immediateContext->OMSetRenderTargets(1u, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    }
}