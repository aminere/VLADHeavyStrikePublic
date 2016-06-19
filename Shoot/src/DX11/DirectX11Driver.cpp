/* 

Amine Rehioui
Created: July 27th 2013

*/

#include "Shoot.h"

#include "App.h"
#include "DirectX11Driver.h"

#include "VertexBufferDX11.h"
#include "TextureDX11.h"
#include "ShaderDX11.h"

#include "FakeCubeMapTexture.h"
#include "SkyBoxVertexBufferNoCubemapDX11.h"

#if SHOOT_PLATFORM == SHOOT_PLATFORM_WP8
extern HRESULT ShootCreateSwapChainForCoreWindow(IDXGIFactory2* pFactory, ID3D11Device1* pDevice, DXGI_SWAP_CHAIN_DESC1* pSwapChainDesc, IDXGISwapChain1** ppSwapChain);
#endif

namespace shoot
{
	//! constructor
	DirectX11Driver::DirectX11Driver()
		: m_FeatureLevel(D3D_FEATURE_LEVEL_9_3)
		, m_ProjectionViewMatrix(Matrix44::Identity)
	{
	}

	//! destructor
	DirectX11Driver::~DirectX11Driver()
	{
	}

	//! driver initialization
	void DirectX11Driver::Init()
	{
		super::Init();

		UINT creationFlags = 0;

#if defined(_DEBUG)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevels[] = 
		{
			//D3D_FEATURE_LEVEL_11_1,
			//D3D_FEATURE_LEVEL_11_0,
			//D3D_FEATURE_LEVEL_10_1,
			//D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3 // max compatibility with Windows Phone 8
		};
		
		ID3D11Device* pDevice = NULL;
		ID3D11DeviceContext* pContext = NULL;
		DX_ASSERT(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, featureLevels, ARRAYSIZE(featureLevels),
									D3D11_SDK_VERSION, &pDevice, &m_FeatureLevel, &pContext));
		
		ID3D11Device1* pDevice1 = NULL;
		ID3D11DeviceContext1* pContext1 = NULL;
		DX_ASSERT(pDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&pDevice1));
		DX_ASSERT(pContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&pContext1));
		m_Device = pDevice1;
		m_ImmediateContext = pContext1;
		pDevice->Release();
		pContext->Release();

#if SHOOT_PLATFORM != SHOOT_PLATFORM_WP8
		if(!m_RenderTargetView.Get())
		{
			ResizeScreen(Size(s32(m_ViewPortArea.Size().X), s32(m_ViewPortArea.Size().Y)));
		}
#endif
	}

	//! begins taking graphic commands
	void DirectX11Driver::Begin(bool bClearBackBuffer /*= true*/, bool bClearDepthBuffer /*= true*/, Color clearColor /*= Color()*/)
	{
		m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView, clearColor.v);
		if(bClearDepthBuffer)
		{
			m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		}

		ID3D11Buffer* pMVPBuffer = m_MVPBuffer;
		m_ImmediateContext->VSSetConstantBuffers(0, 1, &pMVPBuffer);
		m_MVPBuffer = pMVPBuffer;

		ID3D11Buffer* pTextureTransformBuffer = m_TextureTransformBuffer;
		m_ImmediateContext->VSSetConstantBuffers(1, 1, &pTextureTransformBuffer);
		m_TextureTransformBuffer = pTextureTransformBuffer;

		ID3D11Buffer* pMaterialBuffer = m_MaterialBuffer;
		m_ImmediateContext->VSSetConstantBuffers(2, 1, &pMaterialBuffer);
		m_MaterialBuffer = pMaterialBuffer;

		ID3D11SamplerState* pSampler = m_TextureSamplerLinear;
		m_ImmediateContext->PSSetSamplers(0, 1, &pSampler);
		m_TextureSamplerLinear = pSampler;
		
	}

	//! create a render target texture
	Texture* DirectX11Driver::CreateRenderTarget(const Vector2& vSize)
	{
		return NULL; //snew OpenGLRenderTarget(vSize);
	}

	//! create a shadow map texture
	Texture* DirectX11Driver::CreateShadowMap(const Vector2& vSize)
	{
		return NULL; //snew OpenGLShadowMap(vSize);
	}

	//! sets a render target
	/** \param pTarget: render target texture. Pass NULL to select the regular back buffer */
	void DirectX11Driver::SetRenderTarget(Texture* pTarget)
	{
		//if(pTarget)
		//{
		//	GL_CHECK(glViewport(0, 0, u32(pTarget->GetSize().X), u32(pTarget->GetSize().Y)));
		//	switch(pTarget->GetType())
		//	{
		//	case Texture::T_RenderTarget:
		//		OpenGLExtensionHandler::Instance()->extGlBindFramebuffer(GL_FRAMEBUFFER_EXT, static_cast<OpenGLRenderTarget*>(pTarget)->GetFrameBufferID());
		//		break;

		//	case Texture::T_ShadowMap:
		//		OpenGLExtensionHandler::Instance()->extGlBindFramebuffer(GL_FRAMEBUFFER_EXT, static_cast<OpenGLShadowMap*>(pTarget)->GetFrameBufferID());
		//		break;

		//	default:
		//		SHOOT_ASSERT(false, "Render target has wrong type");
		//	}			
		//}
		//else
		//{
		//	GraphicsDriver::Instance()->SetViewPort(GraphicsDriver::Instance()->GetViewPort());
		//	OpenGLExtensionHandler::Instance()->extGlBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		//}

		//ClearBuffers(BF_BackBuffer | BF_DepthBuffer);
	}

	//! draws a list of primitives from batch of vertices
	/** \param eType: primitive type 
		\param vertexFlags: a combination of E_VertexFlag 
		\param numVertices: number of vertices provided 
		\param pVertices: vertex array */
	void DirectX11Driver::DrawPrimitiveList(E_PrimitiveType eType, u32 vertexFlags, u32 numVertices, const Vertex3D* pVertices)
	{
		//static s32 glPrimitives[] = { GL_POINTS,
		//							  GL_LINES,
		//							  GL_LINE_LOOP,
		//							  GL_LINE_STRIP,
		//							  GL_TRIANGLES,
		//							  GL_TRIANGLE_STRIP,
		//							  GL_TRIANGLE_FAN };

		//const bool bPos = ((vertexFlags & Vertex3D::VF_Pos) != 0);
		//const bool bUV = ((vertexFlags & Vertex3D::VF_UV) != 0);
		//const bool bNormal = ((vertexFlags & Vertex3D::VF_Normal) != 0);
		//const bool bColor = ((vertexFlags & Vertex3D::VF_Color) != 0);
		//const bool bUVW = ((vertexFlags & Vertex3D::VF_UVW) != 0);

		//if(bPos)
		//{
		//	GL_CHECK(glEnableClientState(GL_VERTEX_ARRAY));
		//	GL_CHECK(glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].Pos)));
		//}
		//else
		//{
		//	GL_CHECK(glDisableClientState(GL_VERTEX_ARRAY));
		//}

		//if(bUV || bUVW)
		//{
		//	GL_CHECK(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

		//	if(bUV)
		//	{
		//		GL_CHECK(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].UV)));
		//	}
		//	else			
		//	{
		//		GL_CHECK(glTexCoordPointer(3, GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].UVW)));
		//	}
		//}
		//else
		//{
		//	GL_CHECK(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
		//}
		//
		//if(bNormal)
		//{
		//	GL_CHECK(glEnableClientState(GL_NORMAL_ARRAY));
		//	GL_CHECK(glNormalPointer(GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].Normal)));
		//}
		//else
		//{
		//	GL_CHECK(glDisableClientState(GL_NORMAL_ARRAY));
		//}

		//if(bColor)
		//{
		//	GL_CHECK(glEnableClientState(GL_COLOR_ARRAY));
		//	GL_CHECK(glColorPointer(4, GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].color)));
		//}
		//else
		//{
		//	GL_CHECK(glDisableClientState(GL_COLOR_ARRAY));
		//}

		//GL_CHECK(glDrawArrays(glPrimitives[eType], 0, numVertices));

		//bPos ? glDisableClientState(GL_VERTEX_ARRAY) : void();
		//(bUV || bUVW) ? glDisableClientState(GL_TEXTURE_COORD_ARRAY) : void();
		//bNormal ? glDisableClientState(GL_NORMAL_ARRAY) : void();
		//bColor ? glDisableClientState(GL_COLOR_ARRAY) : void();
	}

	//! draws a list of primitives from batch of vertices
	/** \param eType: primitive type 
		\param pVertexBuffer: vertex buffer to use */
	void DirectX11Driver::DrawPrimitiveList(E_PrimitiveType eType, VertexBuffer* pVertexBuffer)
	{
		DrawPrimitiveList(eType, pVertexBuffer->GetVertexFlags(), pVertexBuffer->GetNumVertices(), pVertexBuffer->GetVertices());
	}

	//! draws a list of primitives from batch of vertices and indices
	/** \param eType: primitive type 
		\param vertexFlags: a combination of E_VertexFlag		
		\param pVertices: vertex array
		\param pIndices: index array 
		\param numIndices: number of indices provided */
	void DirectX11Driver::DrawIndexedPrimitiveList(E_PrimitiveType eType, u32 vertexFlags, Vertex3D* pVertices, u32 numIndices, u16* pIndices)
	{
		//static s32 glPrimitives[] = { GL_POINTS,
		//							  GL_LINES,
		//							  GL_LINE_LOOP,
		//							  GL_LINE_STRIP,
		//							  GL_TRIANGLES,
		//							  GL_TRIANGLE_STRIP,
		//							  GL_TRIANGLE_FAN };

		//const bool bPos = ((vertexFlags & Vertex3D::VF_Pos) != 0);
		//const bool bUV = ((vertexFlags & Vertex3D::VF_UV) != 0);
		//const bool bNormal = ((vertexFlags & Vertex3D::VF_Normal) != 0);
		//const bool bColor = ((vertexFlags & Vertex3D::VF_Color) != 0);
		//const bool bUVW = ((vertexFlags & Vertex3D::VF_UVW) != 0);

		//if(bPos)
		//{
		//	GL_CHECK(glEnableClientState(GL_VERTEX_ARRAY));
		//	GL_CHECK(glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].Pos)));
		//}
		//else
		//{
		//	GL_CHECK(glDisableClientState(GL_VERTEX_ARRAY));
		//}

		//if(bUV || bUVW)
		//{
		//	GL_CHECK(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

		//	if(bUV)
		//	{
		//		GL_CHECK(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].UV)));
		//	}
		//	else			
		//	{
		//		GL_CHECK(glTexCoordPointer(3, GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].UVW)));
		//	}
		//}
		//else
		//{
		//	GL_CHECK(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
		//}
		//
		//if(bNormal)
		//{
		//	GL_CHECK(glEnableClientState(GL_NORMAL_ARRAY));
		//	GL_CHECK(glNormalPointer(GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].Normal)));
		//}
		//else
		//{
		//	GL_CHECK(glDisableClientState(GL_NORMAL_ARRAY));
		//}

		//if(bColor)
		//{
		//	GL_CHECK(glEnableClientState(GL_COLOR_ARRAY));
		//	GL_CHECK(glColorPointer(4, GL_FLOAT, sizeof(Vertex3D), &(pVertices[0].color)));
		//}
		//else
		//{
		//	GL_CHECK(glDisableClientState(GL_COLOR_ARRAY));
		//}
		//
		//GL_CHECK(glDrawElements(glPrimitives[eType], numIndices, GL_UNSIGNED_SHORT, pIndices));
	}	

	//! renders a line
	void DirectX11Driver::DrawLine(const Vector3& vStart, const Vector3& vEnd)
	{
		//glBegin(GL_LINES);
		//glVertex3f(vStart.X, vStart.Y, vStart.Z); glVertex3f(vEnd.X, vEnd.Y, vEnd.Z);			
		//glEnd();
	}

	//! renders a 2D quad
	void DirectX11Driver::Draw2DQuad(const Vector2& vOffset, const Vector2& vSize)
	{
		const Vertex3D aVertices[] = 
		{
			{ Vector3::Create(vOffset.X, vOffset.Y, 0.0f), Vector2(), Vector3::Zero, Color(), Vector3::Zero },
			{ Vector3::Create(vOffset.X+vSize.X, vOffset.Y, 0.0f), Vector2(), Vector3::Zero, Color(), Vector3::Zero },
			{ Vector3::Create(vOffset.X+vSize.X, vOffset.Y+vSize.Y, 0.0f), Vector2(), Vector3::Zero, Color(), Vector3::Zero },
			{ Vector3::Create(vOffset.X, vOffset.Y+vSize.Y, 0.0f), Vector2(), Vector3::Zero, Color(), Vector3::Zero },
			{ Vector3::Create(vOffset.X, vOffset.Y, 0.0f), Vector2(), Vector3::Zero, Color(), Vector3::Zero },
			{ Vector3::Create(vOffset.X+vSize.X, vOffset.Y+vSize.Y, 0.0f), Vector2(), Vector3::Zero, Color(), Vector3::Zero }
		};

		DrawPrimitiveList(GraphicsDriver::PT_Triangle, Vertex3D::VF_Pos, 6, aVertices);
	}

	//! renders a debug box
	void DirectX11Driver::DrawDebugBox(const AABBox3D& box)
	{
		//Vector3 vMin = box.Min();
		//Vector3 vMax = box.Max();

		//glBegin(GL_LINE_LOOP);				
		//glVertex3f(vMin.X, vMin.Y, vMin.Z); glVertex3f(vMax.X, vMin.Y, vMin.Z); glVertex3f(vMax.X, vMax.Y, vMin.Z); glVertex3f(vMin.X, vMax.Y, vMin.Z); glVertex3f(vMin.X, vMin.Y, vMin.Z);
		//glEnd();
		//glBegin(GL_LINE_LOOP);				
		//glVertex3f(vMin.X, vMin.Y, vMax.Z); glVertex3f(vMax.X, vMin.Y, vMax.Z); glVertex3f(vMax.X, vMax.Y, vMax.Z); glVertex3f(vMin.X, vMax.Y, vMax.Z); glVertex3f(vMin.X, vMin.Y, vMax.Z);
		//glEnd();
		//glBegin(GL_LINES);				
		//glVertex3f(vMin.X, vMin.Y, vMin.Z); glVertex3f(vMin.X, vMin.Y, vMax.Z);
		//glVertex3f(vMax.X, vMin.Y, vMin.Z); glVertex3f(vMax.X, vMin.Y, vMax.Z);
		//glVertex3f(vMin.X, vMax.Y, vMin.Z); glVertex3f(vMin.X, vMax.Y, vMax.Z);
		//glVertex3f(vMax.X, vMax.Y, vMin.Z); glVertex3f(vMax.X, vMax.Y, vMax.Z);
		//glEnd();
	}

	//! renders a debug box
	void DirectX11Driver::DrawDebugBox(const AABBox2D& box)
	{
		//Vector2 vMin = box.Min();
		//Vector2 vMax = box.Max();

		//glBegin(GL_LINE_LOOP);
		//glVertex3f(vMin.X, vMin.Y, 0.0f); 
		//glVertex3f(vMax.X, vMin.Y, 0.0f); 
		//glVertex3f(vMax.X, vMax.Y, 0.0f); 
		//glVertex3f(vMin.X, vMax.Y, 0.0f); 
		//glVertex3f(vMin.X, vMin.Y, 0.0f);
		//glEnd();
	}

	//! sets the viewport area
	void DirectX11Driver::SetViewPort(const AABBox2D& area)
	{
		if(m_ImmediateContext.Get())
		{
			CD3D11_VIEWPORT viewport(area.Min().X, area.Min().Y, area.Max().X, area.Max().Y);
			m_ImmediateContext->RSSetViewports(1, &viewport);
		}
		m_ViewPortArea = area;
	}

	//! resizes the screen surface
	void DirectX11Driver::ResizeScreen(const Size& newSize)
	{
		if(m_Device.Get())
		{
			ID3D11RenderTargetView* nullViews[] = { NULL };
			m_ImmediateContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, NULL);
			m_RenderTargetView = NULL;
			m_DepthStencilView = NULL;
			m_SwapChain = NULL;
			m_MVPBuffer = NULL;
			m_TextureTransformBuffer = NULL;
			m_MaterialBuffer = NULL;
			m_TextureSamplerLinear = NULL;
			for(u32 i=0; i<2; ++i)
			{
				for(u32 j=0; j<D3D11_BLEND_INV_SRC1_ALPHA+1; ++j)
				{
					for(u32 k=0; k<D3D11_BLEND_INV_SRC1_ALPHA+1; ++k)
					{
						m_BlendState[i][j][k] = NULL;
					}
				}
			}
			for(u32 i=0; i<2; ++i)
			{
				for(u32 j=0; j<2; ++j)
				{
					for(u32 k=0; k<D3D11_COMPARISON_ALWAYS+1; ++k)
					{
						m_DepthStencilState[i][j][k] = NULL;
					}
				}
			}
			for(u32 i=0; i<2; ++i)
			{
				for(u32 j=0; j<D3D11_FILL_SOLID+1; ++j)
				{
					for(u32 k=0; k<D3D11_CULL_BACK+1; ++k)
					{
						m_RasterizerState[i][j][k] = NULL;
					}
				}
			}
			m_ImmediateContext->Flush();

			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
			swapChainDesc.Width = newSize.Width;
			swapChainDesc.Height = newSize.Height;
			swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 1; // On phone, only single buffering is supported.
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // On phone, only stretch and aspect-ratio stretch scaling are allowed.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // On phone, no swap effects are supported.
			swapChainDesc.Flags = 0;
			
			COMReference<IDXGIDevice1> XGIDevice1;
			IDXGIDevice1* pIDXGIDevice1 = NULL;
			DX_ASSERT(m_Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pIDXGIDevice1));
			XGIDevice1 = pIDXGIDevice1;

			COMReference<IDXGIAdapter> XGIAdapter;
			IDXGIAdapter* pIDXGIAdapter = NULL;
			DX_ASSERT(XGIDevice1->GetAdapter(&pIDXGIAdapter));
			XGIAdapter = pIDXGIAdapter;

			COMReference<IDXGIFactory2> XGIFactory2;
			IDXGIFactory2* pFactory = NULL;
			DX_ASSERT(XGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pFactory));
			XGIFactory2 = pFactory;

			// create swap chain
			IDXGISwapChain1* pSwapChain = NULL;

#if SHOOT_PLATFORM == SHOOT_PLATFORM_WP8
			DX_ASSERT(ShootCreateSwapChainForCoreWindow(XGIFactory2, m_Device, &swapChainDesc, &pSwapChain));
#else
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
			fullScreenDesc.RefreshRate.Numerator = 60;
			fullScreenDesc.RefreshRate.Denominator = 1;
			fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			fullScreenDesc.Windowed = !App::Instance()->IsFullScreen();			
			DX_ASSERT(XGIFactory2->CreateSwapChainForHwnd(m_Device, m_hWindow, &swapChainDesc, &fullScreenDesc, NULL, &pSwapChain));			
#endif

			m_SwapChain = pSwapChain;

			// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
			// ensures that the application will only render after each VSync, minimizing power consumption.
			DX_ASSERT(XGIDevice1->SetMaximumFrameLatency(1));

			// Create render target & depth stencil views
			COMReference<ID3D11Texture2D> backBuffer;
			ID3D11Texture2D* pBackBuffer = NULL;
			DX_ASSERT(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer));
			backBuffer = pBackBuffer;

			ID3D11RenderTargetView* pRenderTargetView = NULL;
			DX_ASSERT(m_Device->CreateRenderTargetView(backBuffer, NULL, &pRenderTargetView));
			m_RenderTargetView = pRenderTargetView;

			COMReference<ID3D11Texture2D> depthStencil;
			ID3D11Texture2D* pDepthStencil = NULL;
			CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, newSize.Width, newSize.Height, 1, 1, D3D11_BIND_DEPTH_STENCIL);
			DX_ASSERT(m_Device->CreateTexture2D(&depthStencilDesc, NULL, &pDepthStencil));
			depthStencil = pDepthStencil;

			CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
			ID3D11DepthStencilView* pDepthStencilView = NULL;
			DX_ASSERT(m_Device->CreateDepthStencilView(depthStencil, &depthStencilViewDesc, &pDepthStencilView));
			m_DepthStencilView = pDepthStencilView;

			m_ImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, m_DepthStencilView);
			m_RenderTargetView = pRenderTargetView;

			// create the constant buffers			
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;			
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = 0;
			bd.ByteWidth = sizeof(Matrix44);
			ID3D11Buffer* pMVP = NULL;
			DX_ASSERT(m_Device->CreateBuffer(&bd, NULL, &pMVP));
			m_MVPBuffer = pMVP;
			bd.ByteWidth = sizeof(Matrix44);
			ID3D11Buffer* pTextureTransformBuffer = NULL;
			DX_ASSERT(m_Device->CreateBuffer(&bd, NULL, &pTextureTransformBuffer));
			m_TextureTransformBuffer = pTextureTransformBuffer;
			bd.ByteWidth = sizeof(Color);
			ID3D11Buffer* pMaterialBuffer = NULL;
			DX_ASSERT(m_Device->CreateBuffer(&bd, NULL, &pMaterialBuffer));
			m_MaterialBuffer = pMaterialBuffer;

			// Create the texture samplers
			D3D11_SAMPLER_DESC sampDesc;
			ZeroMemory( &sampDesc, sizeof(sampDesc) );
			sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampDesc.MinLOD = 0;
			sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
			ID3D11SamplerState* pSampler = NULL;
			DX_ASSERT(m_Device->CreateSamplerState(&sampDesc, &pSampler));
			m_TextureSamplerLinear = pSampler;

			// Create blend state
			ZeroMemory(&m_BlendDesc, sizeof(D3D11_BLEND_DESC));	
			m_BlendDesc.RenderTarget[0].BlendEnable = false;
			m_BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			m_BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			m_BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			m_BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			m_BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			m_BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			m_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			ID3D11BlendState* pBlendState = NULL;
			DX_ASSERT(m_Device->CreateBlendState(&m_BlendDesc, &pBlendState));
			m_BlendState[m_BlendDesc.RenderTarget[0].BlendEnable][m_BlendDesc.RenderTarget[0].SrcBlend][m_BlendDesc.RenderTarget[0].DestBlend] = pBlendState;

			// Create depth stencil state
			ZeroMemory(&m_DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
			m_DSDesc.DepthEnable = true;
			m_DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			m_DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
			m_DSDesc.StencilEnable = false;
			ID3D11DepthStencilState* pDepthStencilState = NULL;
			DX_ASSERT(m_Device->CreateDepthStencilState(&m_DSDesc, &pDepthStencilState));
			m_DepthStencilState[m_DSDesc.DepthEnable][m_DSDesc.DepthWriteMask][m_DSDesc.DepthFunc] = pDepthStencilState;

			// Create the rasterizer state
			ZeroMemory(&m_RSDesc, sizeof(D3D11_RASTERIZER_DESC));
			m_RSDesc.AntialiasedLineEnable = false;
			m_RSDesc.CullMode = D3D11_CULL_BACK;
			m_RSDesc.DepthBias = 0;
			m_RSDesc.DepthBiasClamp = 0.0f;
			m_RSDesc.DepthClipEnable = true;
			m_RSDesc.FillMode = D3D11_FILL_SOLID;
			m_RSDesc.FrontCounterClockwise = false;
			m_RSDesc.MultisampleEnable = false;
			m_RSDesc.ScissorEnable = false;
			m_RSDesc.SlopeScaledDepthBias = 0.0f;
			ID3D11RasterizerState* pRasterizerState = NULL;
			DX_ASSERT(m_Device->CreateRasterizerState(&m_RSDesc, &pRasterizerState));
			m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode] = pRasterizerState;

			SetCullMode(CM_CounterClockWise);
		}

		super::ResizeScreen(newSize);
	}

	//! Sets transformation matrices
	/** override with driver specific code */
	void DirectX11Driver::SetTransform(E_TransformState eState, const Matrix44& matrix)
	{
		super::SetTransform(eState, matrix.GetTranspose());

		switch(eState)
		{
		case TS_Projection:
		case TS_View:
			m_ProjectionViewMatrix = m_StateMatrices[TS_Projection] * m_StateMatrices[TS_View];
			break;
		}
	}

	//! Sets a render state
	void DirectX11Driver::SetRenderState(E_RenderState eState, bool bEnable)
	{
		switch(eState)
		{
		case RS_DepthTesting:
			{
				bool bEnabled = (m_DSDesc.DepthEnable != 0);
				if(bEnabled != bEnable)
				{
					m_DSDesc.DepthEnable = bEnable;
					if(!m_DepthStencilState[m_DSDesc.DepthEnable][m_DSDesc.DepthWriteMask][m_DSDesc.DepthFunc].Get())
					{
						ID3D11DepthStencilState* pState = NULL;
						DX_ASSERT(m_Device->CreateDepthStencilState(&m_DSDesc, &pState));
						m_DepthStencilState[m_DSDesc.DepthEnable][m_DSDesc.DepthWriteMask][m_DSDesc.DepthFunc] = pState;
					}
				}
			}
			break;

		case RS_DepthWriting:
			{

				D3D11_DEPTH_WRITE_MASK mask = bEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
				if(mask != m_DSDesc.DepthWriteMask)
				{
					m_DSDesc.DepthWriteMask = mask;
					if(!m_DepthStencilState[m_DSDesc.DepthEnable][m_DSDesc.DepthWriteMask][m_DSDesc.DepthFunc].Get())
					{
						ID3D11DepthStencilState* pState = NULL;
						DX_ASSERT(m_Device->CreateDepthStencilState(&m_DSDesc, &pState));
						m_DepthStencilState[m_DSDesc.DepthEnable][m_DSDesc.DepthWriteMask][m_DSDesc.DepthFunc] = pState;
					}
				}
			}
			break;

		case RS_AlphaBlending:
			{
				bool bEnabled = (m_BlendDesc.RenderTarget[0].BlendEnable != 0);
				if(bEnabled != bEnable)
				{
					m_BlendDesc.RenderTarget[0].BlendEnable = bEnable;
					if(!m_BlendState[m_BlendDesc.RenderTarget[0].BlendEnable][m_BlendDesc.RenderTarget[0].SrcBlend][m_BlendDesc.RenderTarget[0].DestBlend].Get())
					{
						ID3D11BlendState* pState = NULL;
						DX_ASSERT(m_Device->CreateBlendState(&m_BlendDesc, &pState));
						m_BlendState[m_BlendDesc.RenderTarget[0].BlendEnable][m_BlendDesc.RenderTarget[0].SrcBlend][m_BlendDesc.RenderTarget[0].DestBlend] = pState;
					}
				}
			}
			break;

		case RS_2DTextureMapping:
			break;

		case RS_PointSprite:
			break;

		default:
			SHOOT_WARNING(0, "SetRenderState could not handle state");
		}
	}

	//! Sets a render state
	void DirectX11Driver::SetRenderState(E_RenderState eState, s32 iValue)
	{
		static D3D11_COMPARISON_FUNC compFuncs[] = { D3D11_COMPARISON_NEVER,
													 D3D11_COMPARISON_LESS,
													 D3D11_COMPARISON_EQUAL,
													 D3D11_COMPARISON_LESS_EQUAL,
													 D3D11_COMPARISON_GREATER,
													 D3D11_COMPARISON_NOT_EQUAL,
													 D3D11_COMPARISON_GREATER_EQUAL,
													 D3D11_COMPARISON_ALWAYS };

		static D3D11_FILL_MODE fillModes[] = { D3D11_FILL_WIREFRAME, // Point fill mode not supported in DX11
											   D3D11_FILL_WIREFRAME,
											   D3D11_FILL_SOLID };

		switch(eState)
		{
		case RS_DepthFunc:
			SHOOT_ASSERT(iValue < CF_Count, "Invalid E_ComparisonFunction");
			if(m_DSDesc.DepthFunc != compFuncs[iValue])
			{
				m_DSDesc.DepthFunc = compFuncs[iValue];
				if(!m_DepthStencilState[m_DSDesc.DepthEnable][m_DSDesc.DepthWriteMask][m_DSDesc.DepthFunc].Get())
				{
					ID3D11DepthStencilState* pState = NULL;
					DX_ASSERT(m_Device->CreateDepthStencilState(&m_DSDesc, &pState));
					m_DepthStencilState[m_DSDesc.DepthEnable][m_DSDesc.DepthWriteMask][m_DSDesc.DepthFunc] = pState;
				}
			}
			break;

		case RS_FillMode:
			SHOOT_ASSERT(iValue < FM_Count, "Invalid E_FillMode");
			if(m_RSDesc.FillMode != fillModes[iValue])
			{
				m_RSDesc.FillMode = fillModes[iValue];
				if(!m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode].Get())
				{
					ID3D11RasterizerState* pState = NULL;
					DX_ASSERT(m_Device->CreateRasterizerState(&m_RSDesc, &pState));
					m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode] = pState;
				}
			}
			break;

		default:
			SHOOT_WARNING(0, "SetRenderState could not handle state");
		}
	}

	//! Sets a render state
	void DirectX11Driver::SetRenderState(E_RenderState eState, f32 fValue)
	{
		//switch(eState)
		//{
		//case RS_PointSize:
		//	GL_CHECK(glPointSize(fValue));
		//	GL_CHECK(glLineWidth(fValue));
		//	break;

		//case RS_MinPointSize:
		//	OpenGLExtensionHandler::Instance()->extGlPointParameterf(GL_POINT_SIZE_MIN, fValue);
		//	break;

		//case RS_MaxPointSize:
		//	OpenGLExtensionHandler::Instance()->extGlPointParameterf(GL_POINT_SIZE_MAX, fValue);
		//	break;

		//default:
		//	SHOOT_WARNING(0, "SetRenderState could not handle state");
		//}
	}

	//! Sets a render state
	void DirectX11Driver::SetRenderState(E_RenderState eState, const f32* fValues)
	{
		//switch(eState)
		//{
		//case RS_PointDistanceAttenuation:
		//	OpenGLExtensionHandler::Instance()->extGlPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, fValues);
		//	break;

		//default:
		//	SHOOT_WARNING(0, "SetRenderState could not handle state");
		//}
	}
	
	//! Sets the blend function
	void DirectX11Driver::SetBlendFunc(E_BlendFactor eSrc, E_BlendFactor eDest)
	{
		static D3D11_BLEND blendFactors[] = { D3D11_BLEND_ZERO, 
											  D3D11_BLEND_ONE, 
											  D3D11_BLEND_SRC_COLOR, 
											  D3D11_BLEND_INV_SRC_COLOR, 
											  D3D11_BLEND_SRC_ALPHA, 
											  D3D11_BLEND_INV_SRC_ALPHA,
											  D3D11_BLEND_DEST_ALPHA, 
											  D3D11_BLEND_INV_DEST_ALPHA, 
											  D3D11_BLEND_DEST_COLOR, 
											  D3D11_BLEND_INV_DEST_COLOR, 
											  D3D11_BLEND_SRC_ALPHA_SAT };
		
		if(m_BlendDesc.RenderTarget[0].SrcBlend != blendFactors[eSrc]
		|| m_BlendDesc.RenderTarget[0].DestBlend != blendFactors[eDest])
		{
			m_BlendDesc.RenderTarget[0].SrcBlend = blendFactors[eSrc];
			m_BlendDesc.RenderTarget[0].DestBlend = blendFactors[eDest];
			if(!m_BlendState[m_BlendDesc.RenderTarget[0].BlendEnable][m_BlendDesc.RenderTarget[0].SrcBlend][m_BlendDesc.RenderTarget[0].DestBlend].Get())
			{
				ID3D11BlendState* pState = NULL;
				DX_ASSERT(m_Device->CreateBlendState(&m_BlendDesc, &pState));
				m_BlendState[m_BlendDesc.RenderTarget[0].BlendEnable][m_BlendDesc.RenderTarget[0].SrcBlend][m_BlendDesc.RenderTarget[0].DestBlend] = pState;
			}
		}
	}

	//! returns a render state
	void DirectX11Driver::GetRenderState(E_RenderState eState, bool& bEnabled)
	{
		//GLboolean bResult;

		//switch(eState)
		//{
		//case RS_DepthTesting:			
		//	GL_CHECK(glGetBooleanv(GL_DEPTH_TEST, &bResult));			
		//	break;

		//case RS_DepthWriting:
		//	GL_CHECK(glGetBooleanv(GL_DEPTH_WRITEMASK, &bResult));			
		//	break;

		//case RS_AlphaBlending:
		//	GL_CHECK(glGetBooleanv(GL_BLEND, &bResult));						
		//	break;

		//default:
		//	SHOOT_WARNING(0, "GetRenderState could not handle state");
		//}

		//bEnabled = (bResult != 0);
	}

	//! returns a render state
	void DirectX11Driver::GetRenderState(E_RenderState eState, s32& iValue)
	{
		//GLint iResult;

		//switch(eState)
		//{
		//case RS_DepthFunc:
		//	GL_CHECK(glGetIntegerv(GL_DEPTH_FUNC, &iResult));			
		//	break;		

		//case RS_SrcBlend:				
		//	GL_CHECK(glGetIntegerv(GL_BLEND_SRC, &iResult));					
		//	break;

		//case RS_DestBlend:			
		//	GL_CHECK(glGetIntegerv(GL_BLEND_DST, &iResult));
		//	break;

		//default:
		//	SHOOT_WARNING(0, "GetRenderState could not handle state");
		//}

		//iValue = iResult;
	}	

	//! Sets a material color		
	void DirectX11Driver::SetColor(const Color& color)
	{
		m_ImmediateContext->UpdateSubresource(m_MaterialBuffer, 0, NULL, color.v, 0, 0);		
	}

	//! creates a texture from a path
	Texture* DirectX11Driver::CreateTexture(const char* strPath)
	{
		return snew TextureDX11(strPath);
	}

	//! create a shader
	ShaderImpl* DirectX11Driver::CreateShader()
	{
		return snew ShaderDX11();
	}

	//! creates a cubemap texture
	Texture* DirectX11Driver::CreateCubeMapTexture(const char* strPath[6])
	{
		return snew FakeCubeMapTexture(strPath);
	}

	//! create a vertex buffer
	VertexBuffer* DirectX11Driver::CreateVertexBuffer()
	{
		return snew VertexBufferDX11();
	}

	//! create a skybox vertex buffer
	VertexBuffer* DirectX11Driver::CreateSkyBoxVertexBufferNoCubemap(Texture* pTexture)
	{
		return snew SkyBoxVertexBufferNoCubemapDX11(static_cast<FakeCubeMapTexture*>(pTexture));
	}

	//! draws using a vertex buffer
	void DirectX11Driver::Draw(u32 numVertices, u32 startVertex)
	{
		Matrix44 mvp = m_ProjectionViewMatrix * m_StateMatrices[TS_World];
		m_ImmediateContext->UpdateSubresource(m_MVPBuffer, 0, NULL, mvp.GetFloats(), 0, 0);
		m_ImmediateContext->UpdateSubresource(m_TextureTransformBuffer, 0, NULL, m_StateMatrices+TS_Texture, 0, 0);
		m_ImmediateContext->Draw(numVertices, startVertex);
	}

	//! draws using an index buffer
	void DirectX11Driver::DrawIndexed(u32 numIndices, u32 startIndex, u32 startVertex)
	{
		Matrix44 mvp = m_ProjectionViewMatrix * m_StateMatrices[TS_World];
		m_ImmediateContext->UpdateSubresource(m_MVPBuffer, 0, NULL, mvp.GetFloats(), 0, 0);
		m_ImmediateContext->UpdateSubresource(m_TextureTransformBuffer, 0, NULL, m_StateMatrices+TS_Texture, 0, 0);
		m_ImmediateContext->DrawIndexed(numIndices, startIndex, startVertex);
	}

	//! clears graphic buffers
	/** \param mask: a combination of E_BufferFlag flags */
	void DirectX11Driver::ClearBuffers(u32 mask, Color clearColor /*= Color()*/)
	{
		if((mask & BF_BackBuffer) != 0)
		{
			m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView, clearColor.v);
		}
		
		u32 depthStencilFlags = 0;

		if((mask & BF_DepthBuffer) != 0)
		{			
			depthStencilFlags |= D3D11_CLEAR_DEPTH;
		}

		if((mask & BF_StencilBuffer) != 0)
		{
			depthStencilFlags |= D3D11_CLEAR_STENCIL;
		}

		if(depthStencilFlags)
		{
			m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView, depthStencilFlags, 1.0f, 0);
		}
	}

	//! sets the culling mode
	void DirectX11Driver::SetCullMode(E_CullMode eMode)
	{
		switch(eMode)
		{
		case CM_ClockWise:
			if(!m_RSDesc.FrontCounterClockwise || m_RSDesc.CullMode != D3D11_CULL_BACK)
			{
				m_RSDesc.FrontCounterClockwise = true;
				m_RSDesc.CullMode = D3D11_CULL_BACK;
				if(!m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode].Get())
				{
					ID3D11RasterizerState* pState = NULL;
					DX_ASSERT(m_Device->CreateRasterizerState(&m_RSDesc, &pState));
					m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode] = pState;
				}
			}
			break;

		case CM_CounterClockWise:
			if(m_RSDesc.FrontCounterClockwise || m_RSDesc.CullMode != D3D11_CULL_BACK)
			{
				m_RSDesc.FrontCounterClockwise = false;
				m_RSDesc.CullMode = D3D11_CULL_BACK;
				if(!m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode].Get())
				{
					ID3D11RasterizerState* pState = NULL;
					DX_ASSERT(m_Device->CreateRasterizerState(&m_RSDesc, &pState));
					m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode] = pState;
				}
			}
			break;

		case CM_None:
			if(m_RSDesc.CullMode != D3D11_CULL_NONE)
			{
				m_RSDesc.CullMode = D3D11_CULL_NONE;
				if(!m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode].Get())
				{
					ID3D11RasterizerState* pState = NULL;
					DX_ASSERT(m_Device->CreateRasterizerState(&m_RSDesc, &pState));
					m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode] = pState;
				}
			}
			break;

		default:
			SHOOT_WARNING(0, "SetCullMode could not handle mode");
		}
	}

	//! presents the graphics
	void DirectX11Driver::Present()
	{
		HRESULT hr = m_SwapChain->Present(1, 0);
		m_ImmediateContext->DiscardView(m_RenderTargetView);
		m_ImmediateContext->DiscardView(m_DepthStencilView);
		if(hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			// device lost
			m_SwapChain = NULL;
			Init();
			ResizeScreen(Size(s32(m_ViewPortArea.Size().X), s32(m_ViewPortArea.Size().Y)));
		}
		else
		{
			SHOOT_LOG_WARNING(hr == S_OK, "Present Failed: %d", hr);
		}
	}

	//! binds material states
	void DirectX11Driver::BindMaterialStates() 
	{ 
		m_ImmediateContext->OMSetBlendState(m_BlendState[m_BlendDesc.RenderTarget[0].BlendEnable][m_BlendDesc.RenderTarget[0].SrcBlend][m_BlendDesc.RenderTarget[0].DestBlend], 0, 0xffffffff);
		m_ImmediateContext->OMSetDepthStencilState(m_DepthStencilState[m_DSDesc.DepthEnable][m_DSDesc.DepthWriteMask][m_DSDesc.DepthFunc], 1);
		m_ImmediateContext->RSSetState(m_RasterizerState[m_RSDesc.FrontCounterClockwise][m_RSDesc.FillMode][m_RSDesc.CullMode]);
	}

	//! registers a COM reference
	void DirectX11Driver::RegisterReference(IUnknown* pObject)
	{
		if(m_COMReferences.find(pObject) != m_COMReferences.end())
		{
			m_COMReferences[pObject] = m_COMReferences[pObject] + 1;
		}
		else
		{
			m_COMReferences[pObject] = 1;
		}
	}

	//! unregisters a COM reference
	void DirectX11Driver::UnregisterReference(IUnknown* pObject)
	{
		SHOOT_ASSERT(m_COMReferences.find(pObject) != m_COMReferences.end(), "Unregistering unknown COM object");		
		
		pObject->Release();

		s32& refCount = m_COMReferences[pObject];
		--refCount;
		SHOOT_ASSERT(refCount >= 0, "Negative RefCount detected");

		if(refCount == 0)
		{
			m_COMReferences.erase(pObject);
		}
	}
}

