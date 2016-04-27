//********************************************//
//*********** INCLUDES & DEFINES *************//
//********************************************//

#include <iostream>
#include <ctime>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "Define.h"
#include "Trivial_VS.csh"
#include "Trivial_PS.csh"
#include "Trivial_PS_Texture.csh"
#include "Trivial_PS_For_Skybox.csh"
#include "Trivial_GS.csh"
#include "Trivial_VS_ForGeometry.csh"
#include "Trivial_PS_ForGeometry.csh"
#include "LoadModel.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600

//*******************************************//
//********* SIMPLE WINDOW APP CLASS ********//
//*****************************************//

class D3D_DEMO
{

	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct SIMPLE_VERTEX
	{
		XMFLOAT4 position;
		XMFLOAT4 color;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};
	struct SEND_TO_OBJECT
	{
		XMMATRIX worldMatrix;
	};
	struct SEND_TO_SCENE
	{
		XMMATRIX viewMatrix;
		XMMATRIX proejctionMatrix;
	};
	struct LIGHT_VERTEX
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;
	};

	struct POINT_LIGHT_VERTEX
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightPos;
		float m_radius;
	};

	struct SPOT_LIGHT_VERTEX
	{
		float ratio;
		XMFLOAT3 padding;

		XMFLOAT3 lightPosition;
		float padding2;

		XMFLOAT3 lightDirection;
		float padding3;

		XMFLOAT4 diffuseColor;
	};

	struct GEO_VERTEX
	{
		XMFLOAT4 m_position;
		XMFLOAT4 m_color;
	};

	IDXGISwapChain* m_SwapChain;
	ID3D11Device *m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11RasterizerState* m_rasterState;
	ID3D11RasterizerState* m_rasterForSkybox;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cBuffer[2];
	ID3D11Buffer* m_cBufferForLight;
	ID3D11Buffer* m_cBufferForPointLight;
	ID3D11Buffer* m_cBufferForSpotLight;
	ID3D11Buffer* m_iBuffer;
	ID3D11Texture2D* z_buffer;
	ID3D11VertexShader* m_vertexShader;
	ID3D11VertexShader* m_vertexShaderForGeometry;
	ID3D11PixelShader* m_pixelShaderForGeometry;
	ID3D11PixelShader* m_pixelShader;
	ID3D11PixelShader* m_pixelShaderForTexture;
	ID3D11PixelShader* m_pixelShaderForSkybox;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11InputLayout* m_layout;
	ID3D11InputLayout* m_layoutForGeometryShader;
	D3D11_VIEWPORT m_viewport;
	D3D_FEATURE_LEVEL m_featureLevel;
	unsigned int num_of_vertices;
	SEND_TO_OBJECT toObject;
	SEND_TO_SCENE toScene;
	MatrixBufferType temp;
	SIMPLE_VERTEX* m_model;
	unsigned int *vert_indices, *text_indices, *norm_indices;
	LoadMesh loadObj;
	unsigned int modelSize;
	ID3D11Buffer *m_modelVertexBuffer, *m_modelIndexBuffer;
	SIMPLE_VERTEX* m_skyModel;
	unsigned int *m_vert, *m_text, *m_norm;
	unsigned int modelSizeForSkybox;
	ID3D11Buffer * m_skyboxVertexBuffer, *m_skyboxIndexBuffer;
	SIMPLE_VERTEX* m_planeModel;
	unsigned int *m_vertForPlane, *m_textForPlane, *m_normForPlane;
	unsigned int modelSizeForPlane;
	ID3D11Buffer *m_planeVertexBuffer, *m_planeIndexBuffer;
	ID3D11Texture2D *m_texture;
	ID3D11ShaderResourceView *m_textureView;
	ID3D11ShaderResourceView *m_PlaneShaderView;
	ID3D11ShaderResourceView *m_SkyBoxShaderView;
	ID3D11ShaderResourceView *m_NullShaderView = NULL;
	ID3D11Texture2D *m_secondTexture;
	ID3D11ShaderResourceView *m_SecondPlaneShaderView;
	ID3D11SamplerState *m_sampleTexture;
	XMMATRIX skyboxWorld;
	ID3D11Buffer *m_lightBuffer;
	ID3D11Buffer * m_PointLightBuffer;
	ID3D11Buffer * m_SpotLightBuffer;
	LIGHT_VERTEX m_light;
	POINT_LIGHT_VERTEX m_pointLight;
	SPOT_LIGHT_VERTEX m_spotLight;
	SIMPLE_VERTEX m_ForQuad;
	ID3D11Buffer *m_vertexBufferForGeometry;
	GEO_VERTEX m_geoVertex;

	unsigned int indices[60] = { 0,1,10,1,2,10,2,3,10,3,4,10,4,5,10,5,6,10,6,7,10,7,8,10,8,9,10,9,0,10,
		0,9,11,9,8,11,8,7,11,7,6,11,6,5,11,5,4,11,4,3,11,3,2,11,2,1,11,1,0,11 };

public:
	D3D_DEMO(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

D3D_DEMO::D3D_DEMO(HINSTANCE hinst, WNDPROC proc)
{
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"Lab 1a Line Land", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	DXGI_RATIONAL m_rational;
	ID3D11Texture2D* ptrToBackBuffer;
	D3D11_BUFFER_DESC m_vertexBuffer = {};
	D3D11_SUBRESOURCE_DATA m_vertexData = {};
	D3D11_BUFFER_DESC m_constantBuffer = {};
	D3D11_SUBRESOURCE_DATA m_constantData = {};
	D3D11_BUFFER_DESC m_indexBuffer = {};
	D3D11_SUBRESOURCE_DATA m_indexData = {};
	D3D11_TEXTURE2D_DESC m_texture2D = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC d_stencil = {};
	D3D11_RASTERIZER_DESC m_RasterDesc = {};
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC texViewDesc;
	D3D11_TEXTURE2D_DESC secondTextureDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC secondTexViewDesc;
	
	D3D11_SAMPLER_DESC m_texture;
	D3D11_BUFFER_DESC lightBufferDesc = {};
	D3D11_BUFFER_DESC point_lightBufferDesc = {};
	D3D11_BUFFER_DESC spotLightBufferDesc = {};
	D3D11_BUFFER_DESC m_constantBufferForLight = {};
	D3D11_SUBRESOURCE_DATA m_constantDataForLight = {};
	D3D11_BUFFER_DESC m_constantBufferForPointLight = {};
	D3D11_SUBRESOURCE_DATA m_constantDataForPointLight = {};
	D3D11_BUFFER_DESC m_constantBufferForSpotLight = {};
	D3D11_SUBRESOURCE_DATA m_constantDataForSpotLight = {};
	D3D11_BUFFER_DESC m_vertexBufferDescForGeometry = {};
	D3D11_SUBRESOURCE_DATA m_vertexDataForGeometry = {};

	swapChainDesc.BufferDesc.Width = BACKBUFFER_WIDTH;
	swapChainDesc.BufferDesc.Height = BACKBUFFER_HEIGHT;

	m_rational.Numerator = 0;
	m_rational.Denominator = 1;

	swapChainDesc.BufferDesc.RefreshRate = m_rational;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = NULL;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.OutputWindow = window;
	swapChainDesc.Windowed = true;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	m_viewport.Width = BACKBUFFER_WIDTH;
	m_viewport.Height = BACKBUFFER_HEIGHT;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	//********************* END WARNING ************************//
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &swapChainDesc, &m_SwapChain, &m_device, &m_featureLevel, &m_deviceContext);

	HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&ptrToBackBuffer);

	m_device->CreateRenderTargetView(ptrToBackBuffer, NULL, &m_renderTargetView);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, NULL);

	//Setting up to draw 

	num_of_vertices = 12;

	float deg = 90;
	SIMPLE_VERTEX ver_of_star[12];
	for (unsigned int i = 0; i < 10; i++)
	{
		if (i % 2 != 0)
		{
			ver_of_star[i].position.x = sin((i * 36) * XM_PI / 180);
			ver_of_star[i].position.y = cos((i * 36) * XM_PI / 180);
			ver_of_star[i].position.z = 0;
			ver_of_star[i].color = GREEN_COLOR;
		}
		else
		{
			ver_of_star[i].position.x = sin((i * 36) * XM_PI / 180) * 0.4f;
			ver_of_star[i].position.y = cos((i * 36) * XM_PI / 180) * 0.4f;
			ver_of_star[i].position.z = 0;
			ver_of_star[i].color = BLUE_COLOR;
		}
		deg -= 36;
	}

	ver_of_star[10].position.x = 0;
	ver_of_star[10].position.y = 0;
	ver_of_star[10].position.z = -0.25f;
	ver_of_star[10].color = RED_COLOR;

	ver_of_star[11].position.x = 0;
	ver_of_star[11].position.y = 0;
	ver_of_star[11].position.z = 0.25f;
	ver_of_star[11].color = RED_COLOR;

	//loading the 3D-model: Ogre
	std::vector<float3> vert_vertices, norm_vertices;
	std::vector<float2> text_vertices;
	std::vector<unsigned int> indicesForV, indicesForT, indicesForN;
	loadObj.LoadObj("OgreOBJ.obj", vert_vertices, text_vertices, norm_vertices, indicesForV, indicesForT, indicesForN);
	vert_indices = new unsigned int[indicesForV.size()];
	text_indices = new unsigned int[indicesForT.size()];
	norm_indices = new unsigned int[indicesForN.size()];

	unsigned int* model_indices = new unsigned int[indicesForV.size()];

	m_model = new SIMPLE_VERTEX[indicesForV.size()];

	modelSize = indicesForV.size();

	for (unsigned int i = 0; i < modelSize; i++)
	{
		vert_indices[i] = indicesForV[i];
		norm_indices[i] = indicesForN[i];
		text_indices[i] = indicesForT[i];
		model_indices[i] = i;
	}

	for (unsigned int j = 0; j < indicesForV.size(); j++)
	{
		m_model[j].position.x = vert_vertices[vert_indices[j]].x + 5;
		m_model[j].position.y = vert_vertices[vert_indices[j]].y - 5;
		m_model[j].position.z = vert_vertices[vert_indices[j]].z;

		m_model[j].color = { 0, 1, 0, 0 };
		m_model[j].texture.x = text_vertices[text_indices[j]].x;
		m_model[j].texture.y = text_vertices[text_indices[j]].y;
		m_model[j].normal.x = norm_vertices[norm_indices[j]].x;
		m_model[j].normal.y = norm_vertices[norm_indices[j]].y;
		m_model[j].normal.z = norm_vertices[norm_indices[j]].z;
	}

	

	//setting up model vertex buffer
	D3D11_BUFFER_DESC modelVertDesc;
	modelVertDesc.Usage = D3D11_USAGE_IMMUTABLE;
	modelVertDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	modelVertDesc.CPUAccessFlags = NULL;
	modelVertDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * indicesForV.size();
	modelVertDesc.MiscFlags = NULL;
	modelVertDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA modelData;
	modelData.pSysMem = m_model;
	modelData.SysMemPitch = 0;
	modelData.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&modelVertDesc, &modelData, &m_modelVertexBuffer);


	D3D11_BUFFER_DESC modelIndDesc;
	modelIndDesc.Usage = D3D11_USAGE_DEFAULT;
	modelIndDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	modelIndDesc.CPUAccessFlags = NULL;
	modelIndDesc.ByteWidth = sizeof(unsigned int) * indicesForV.size();
	modelIndDesc.MiscFlags = NULL;
	modelIndDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = model_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&modelIndDesc, &indexData, &m_modelIndexBuffer);

	unsigned int m_modelSize = vert_vertices.size();
	vert_vertices.clear();
	norm_vertices.clear();
	text_vertices.clear();
	indicesForV.clear();
	indicesForT.clear();
	indicesForN.clear();

	//loading the plane
	loadObj.LoadObj("Plane.obj", vert_vertices, text_vertices, norm_vertices, indicesForV, indicesForT, indicesForN);

	m_vertForPlane = new unsigned int[indicesForV.size()];
	m_textForPlane = new unsigned int[indicesForT.size()];
	m_normForPlane = new unsigned int[indicesForN.size()];

	unsigned int* plane_model_indices = new unsigned int[indicesForV.size()];

	m_planeModel = new SIMPLE_VERTEX[indicesForV.size()];

	modelSizeForPlane = indicesForV.size();

	for (unsigned int i = 0; i < modelSizeForPlane; i++)
	{
		m_vertForPlane[i] = indicesForV[i];
		m_normForPlane[i] = indicesForN[i];
		m_textForPlane[i] = indicesForT[i];
		plane_model_indices[i] = i;
	}

	for (unsigned int j = 0; j < indicesForV.size(); j++)
	{
		m_planeModel[j].position.x = vert_vertices[m_vertForPlane[j]].x;
		m_planeModel[j].position.y = vert_vertices[m_vertForPlane[j]].y - 5;
		m_planeModel[j].position.z = vert_vertices[m_vertForPlane[j]].z;

		m_planeModel[j].color = { 0, 1, 0, 0 };
		m_planeModel[j].texture.x = text_vertices[m_textForPlane[j]].x;
		m_planeModel[j].texture.y = text_vertices[m_textForPlane[j]].y;
		m_planeModel[j].normal.x = norm_vertices[m_normForPlane[j]].x;
		m_planeModel[j].normal.y = norm_vertices[m_normForPlane[j]].y;
		m_planeModel[j].normal.z = norm_vertices[m_normForPlane[j]].z;
	}

	//setting up the plane model vertex buffer
	D3D11_BUFFER_DESC modelVertDescForPlane;
	modelVertDescForPlane.Usage = D3D11_USAGE_IMMUTABLE;
	modelVertDescForPlane.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	modelVertDescForPlane.CPUAccessFlags = NULL;
	modelVertDescForPlane.ByteWidth = sizeof(SIMPLE_VERTEX) * indicesForV.size();
	modelVertDescForPlane.MiscFlags = NULL;
	modelVertDescForPlane.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA modelDataForPlane;
	modelDataForPlane.pSysMem = m_planeModel;
	modelDataForPlane.SysMemPitch = 0;
	modelDataForPlane.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&modelVertDescForPlane, &modelDataForPlane, &m_planeVertexBuffer);

	//setting up the plane model index buffer
	D3D11_BUFFER_DESC modelIndDescForPlane;
	modelIndDescForPlane.Usage = D3D11_USAGE_DEFAULT;
	modelIndDescForPlane.BindFlags = D3D11_BIND_INDEX_BUFFER;
	modelIndDescForPlane.CPUAccessFlags = NULL;
	modelIndDescForPlane.ByteWidth = sizeof(unsigned int) * indicesForV.size();
	modelIndDescForPlane.MiscFlags = NULL;
	modelIndDescForPlane.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexDataForPlane;
	indexDataForPlane.pSysMem = plane_model_indices;
	indexDataForPlane.SysMemPitch = 0;
	indexDataForPlane.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&modelIndDescForPlane, &indexDataForPlane, &m_planeIndexBuffer);

	unsigned int m_modelSizeForPlane = vert_vertices.size();
	vert_vertices.clear();
	norm_vertices.clear();
	text_vertices.clear();
	indicesForV.clear();
	indicesForT.clear();
	indicesForN.clear();

	//loading cube
	loadObj.LoadObj("Cube.obj", vert_vertices, text_vertices, norm_vertices, indicesForV, indicesForT, indicesForN);

	m_vert = new unsigned int[indicesForV.size()];
	m_text = new unsigned int[indicesForT.size()];
	m_norm = new unsigned int[indicesForN.size()];

	m_skyModel = new SIMPLE_VERTEX[vert_vertices.size()];

	modelSizeForSkybox = indicesForV.size();

	for (unsigned int i = 0; i < modelSizeForSkybox; i++)
	{
		m_vert[i] = indicesForV[i];
		m_text[i] = indicesForT[i];
		m_norm[i] = indicesForN[i];
	}

	for (unsigned int j = 0; j < vert_vertices.size(); j++)
	{
		m_skyModel[j].position.x = vert_vertices[j].x;
		m_skyModel[j].position.y = vert_vertices[j].y;
		m_skyModel[j].position.z = vert_vertices[j].z;

		m_skyModel[j].color = { 0, 1, 0, 0 };

		m_skyModel[j].texture.x = text_vertices[j].x;
		m_skyModel[j].texture.y = text_vertices[j].y;
		
		m_skyModel[j].normal.x = norm_vertices[j].x;
		m_skyModel[j].normal.y = norm_vertices[j].y;
		m_skyModel[j].normal.z = norm_vertices[j].z;
	}
	unsigned int skyboxSize = vert_vertices.size();


	//setting up skybox buffer
	D3D11_BUFFER_DESC skymodelVertDesc;
	skymodelVertDesc.Usage = D3D11_USAGE_IMMUTABLE;
	skymodelVertDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	skymodelVertDesc.CPUAccessFlags = NULL;
	skymodelVertDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * skyboxSize;
	skymodelVertDesc.MiscFlags = NULL;
	skymodelVertDesc.StructureByteStride = sizeof(float) * 2;

	D3D11_SUBRESOURCE_DATA skymodelData;
	skymodelData.pSysMem = m_skyModel;
	skymodelData.SysMemPitch = 0;
	skymodelData.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&skymodelVertDesc, &skymodelData, &m_skyboxVertexBuffer);


	//index buffer for skybox 
	D3D11_BUFFER_DESC skymodelIndDesc;
	skymodelIndDesc.Usage = D3D11_USAGE_DEFAULT;
	skymodelIndDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	skymodelIndDesc.CPUAccessFlags = NULL;
	skymodelIndDesc.ByteWidth = sizeof(unsigned int) * modelSizeForSkybox;
	skymodelIndDesc.MiscFlags = NULL;
	skymodelIndDesc.StructureByteStride = sizeof(float) * 2;

	D3D11_SUBRESOURCE_DATA skyindexData;
	skyindexData.pSysMem = m_vert;
	skyindexData.SysMemPitch = 0;
	skyindexData.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&skymodelIndDesc, &skyindexData, &m_skyboxIndexBuffer);

	//setting up the desc of the vertex buffer for star
	m_vertexBuffer.Usage = D3D11_USAGE_IMMUTABLE;
	m_vertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_vertexBuffer.CPUAccessFlags = NULL;
	m_vertexBuffer.ByteWidth = sizeof(SIMPLE_VERTEX) * num_of_vertices;

	//give the subresource structure a pointer to the vertex data
	m_vertexData.pSysMem = ver_of_star;

	m_device->CreateBuffer(&m_vertexBuffer, &m_vertexData, &m_matrixBuffer);

	m_device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &m_vertexShader);
	m_device->CreateVertexShader(Trivial_VS_ForGeometry, sizeof(Trivial_VS_ForGeometry), NULL, &m_vertexShaderForGeometry);
	m_device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &m_pixelShader);
	m_device->CreatePixelShader(Trivial_PS_Texture, sizeof(Trivial_PS_Texture), NULL, &m_pixelShaderForTexture);
	m_device->CreatePixelShader(Trivial_PS_For_Skybox, sizeof(Trivial_PS_For_Skybox), NULL, &m_pixelShaderForSkybox);
	m_device->CreatePixelShader(Trivial_PS_ForGeometry, sizeof(Trivial_PS_ForGeometry), NULL, &m_pixelShaderForGeometry);
	m_device->CreateGeometryShader(Trivial_GS, sizeof(Trivial_GS),NULL, &m_geometryShader);

	D3D11_INPUT_ELEMENT_DESC m_inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	m_device->CreateInputLayout(m_inputLayout, ARRAYSIZE(m_inputLayout), Trivial_VS, sizeof(Trivial_VS), &m_layout);

	D3D11_INPUT_ELEMENT_DESC m_inputLayoutForGS[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	m_device->CreateInputLayout(m_inputLayoutForGS, ARRAYSIZE(m_inputLayoutForGS), Trivial_VS_ForGeometry, sizeof(Trivial_VS_ForGeometry), &m_layoutForGeometryShader);
	
	//setting up the desc of the constant buffer
	m_constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	m_constantBuffer.ByteWidth = sizeof(SEND_TO_OBJECT);
	m_constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//give the subresource structure a pointer to the vertex data
	m_constantData.pSysMem = &toObject;
	m_device->CreateBuffer(&m_constantBuffer, &m_constantData, &m_cBuffer[0]);
	m_constantData.pSysMem = &toScene;
	m_constantBuffer.ByteWidth = sizeof(SEND_TO_SCENE);
	m_device->CreateBuffer(&m_constantBuffer, &m_constantData, &m_cBuffer[1]);

	//Directional Light
	m_light.diffuseColor = { 1.0f,1.0f,1.0f,1.0f };
	m_light.lightDirection = { -1.0f,-0.5f,0.0f };
	//setting up the desc of the constant buffer light
	m_constantBufferForLight.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_constantBufferForLight.Usage = D3D11_USAGE_DYNAMIC;
	m_constantBufferForLight.ByteWidth = sizeof(LIGHT_VERTEX);
	m_constantBufferForLight.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//setting up the desc for constant light data
	m_constantDataForLight.pSysMem = &m_light;
	m_device->CreateBuffer(&m_constantBufferForLight, &m_constantDataForLight, &m_cBufferForLight);

	//PointLight
	m_pointLight.diffuseColor = { 1.0f,1.0f,1.0f,1.0f };
	m_pointLight.lightPos = {0.0f, 1.0f, 0.0f};
	m_pointLight.m_radius = 10.0f;
	//setting up the desc of the constant buffer for point light
	m_constantBufferForPointLight.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_constantBufferForPointLight.Usage = D3D11_USAGE_DYNAMIC;
	m_constantBufferForPointLight.ByteWidth = sizeof(POINT_LIGHT_VERTEX);
	m_constantBufferForPointLight.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//setting up the desc for constant light data
	m_constantDataForPointLight.pSysMem = &m_pointLight;
	m_device->CreateBuffer(&m_constantBufferForPointLight, &m_constantDataForPointLight, &m_cBufferForPointLight);

	//SpotLight
	//m_spotLight.diffuseColor = { 1.0f,.4f,.4f,.4f };
	m_spotLight.diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_spotLight.lightPosition = { 0.0f, 1.0f, 0.0f };
	m_spotLight.lightDirection = { 0.0f, -1.0f, 0.0f };
	m_spotLight.ratio = 0.9f;

	//setting up the desc of the constant buffer for spot light
	m_constantBufferForSpotLight.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_constantBufferForSpotLight.Usage = D3D11_USAGE_DYNAMIC;
	m_constantBufferForSpotLight.ByteWidth = sizeof(SPOT_LIGHT_VERTEX);
	m_constantBufferForSpotLight.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	



	//setting up the desc for constant light data
	m_constantDataForSpotLight.pSysMem = &m_spotLight;
	m_device->CreateBuffer(&m_constantBufferForSpotLight, &m_constantDataForSpotLight, &m_cBufferForSpotLight);

	//setting up the view and projection
	temp.projection = XMMatrixIdentity();
	temp.view = XMMatrixIdentity();
	temp.world = XMMatrixIdentity();
	temp.view.r[3].m128_f32[2] = -5;
	skyboxWorld = XMMatrixIdentity();
	toObject.worldMatrix = XMMatrixIdentity();

	//setting up the projection for rendering 3D model
	temp.projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(90), ASPECTRATIO, SCREEN_NEAR, SCREEN_DEPTH);

	//setting up the desc of the index buffer
	m_indexBuffer.Usage = D3D11_USAGE_IMMUTABLE;
	m_indexBuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_indexBuffer.CPUAccessFlags = NULL;
	m_indexBuffer.ByteWidth = sizeof(indices);

	m_indexData.pSysMem = &indices;
	m_device->CreateBuffer(&m_indexBuffer, &m_indexData, &m_iBuffer);

	//setting up the desc of the light dynamic constant buffer 
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(m_light);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);

	//setting up the desc of the light dynamic constant buffer ->pointLight
	point_lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	point_lightBufferDesc.ByteWidth = sizeof(m_pointLight);
	point_lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	point_lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	point_lightBufferDesc.MiscFlags = 0;
	point_lightBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&point_lightBufferDesc, NULL, &m_PointLightBuffer);

	//setting up the desc of the light dynamic constant buffer -> spotpointlight 
	spotLightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	spotLightBufferDesc.ByteWidth = sizeof(m_spotLight);
	spotLightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	spotLightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	spotLightBufferDesc.MiscFlags = 0;
	spotLightBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&spotLightBufferDesc, NULL, &m_SpotLightBuffer);

	//setting up the
	m_geoVertex.m_position = { -6,0,0,1 };
	m_geoVertex.m_color = { 1,0,0,1 };
	GEO_VERTEX m_quadPos[3];
	m_quadPos[0] = m_geoVertex;

	m_geoVertex.m_position = { 6,0,0,1 };
	m_geoVertex.m_color = { 1,0,0,1 };
	m_quadPos[1] = m_geoVertex;

	m_geoVertex.m_position = { -6,0,-25,1 };
	m_geoVertex.m_color = { 1,0,0,1 };
	m_quadPos[2] = m_geoVertex;

	//setting up vertex buffer for the geometry shader
	m_vertexBufferDescForGeometry.Usage = D3D11_USAGE_IMMUTABLE;
	m_vertexBufferDescForGeometry.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_vertexBufferDescForGeometry.CPUAccessFlags = NULL;
	m_vertexBufferDescForGeometry.ByteWidth = sizeof(GEO_VERTEX)*3;

	m_vertexDataForGeometry.pSysMem = m_quadPos;	

	m_device->CreateBuffer(&m_vertexBufferDescForGeometry, &m_vertexDataForGeometry, &m_vertexBufferForGeometry);

	//texture2d setup
	m_texture2D.Width = BACKBUFFER_WIDTH;
	m_texture2D.Height = BACKBUFFER_HEIGHT;
	m_texture2D.Usage = D3D11_USAGE_DEFAULT;
	m_texture2D.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	m_texture2D.Format = DXGI_FORMAT_D32_FLOAT;
	m_texture2D.MipLevels = 1;
	m_texture2D.ArraySize = 1;
	m_texture2D.SampleDesc.Count = 1;
	m_device->CreateTexture2D(&m_texture2D, NULL, &z_buffer);

	//second texture2d setup
	ZeroMemory(&secondTextureDesc, sizeof(secondTextureDesc));
	secondTextureDesc.Width = BACKBUFFER_WIDTH;
	secondTextureDesc.Height = BACKBUFFER_HEIGHT;
	secondTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	secondTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	secondTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	secondTextureDesc.MipLevels = 1;
	secondTextureDesc.ArraySize = 1;
	secondTextureDesc.SampleDesc.Count = 1;
	secondTextureDesc.CPUAccessFlags = 0;
	secondTextureDesc.MiscFlags = 0;
	m_device->CreateTexture2D(&secondTextureDesc, NULL, &m_secondTexture);

	//for setting up the quad
	m_ForQuad.position = XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f);
	m_ForQuad.color = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);

	d_stencil.Format = DXGI_FORMAT_D32_FLOAT;
	d_stencil.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d_stencil.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(z_buffer, &d_stencil, &m_depthStencilView);

	//setting up the raster
	m_RasterDesc.FillMode = D3D11_FILL_SOLID;
	m_RasterDesc.CullMode = D3D11_CULL_BACK;
	m_RasterDesc.FrontCounterClockwise = false;
	m_RasterDesc.DepthBias = NULL;
	m_RasterDesc.SlopeScaledDepthBias = 0.0f;
	m_RasterDesc.DepthBiasClamp = 0.0f;
	m_RasterDesc.DepthClipEnable = true;
	m_RasterDesc.ScissorEnable = false;
	m_RasterDesc.MultisampleEnable = false;
	m_RasterDesc.AntialiasedLineEnable = true;

	m_device->CreateRasterizerState(&m_RasterDesc, &m_rasterState);

	//passing info inside the dds texture loader
	CreateDDSTextureFromFile(m_device, L"OgreTextures/SkinColorMostro_COLOR.dds", NULL, &m_textureView);
	//CreateDDSTextureFromFile(m_device, NULL, NULL, &m_NullShaderView);
	//passing info inside the dds texture loader for plane
	//CreateDDSTextureFromFile(m_device, L"Textures/checkerboard.dds", NULL, &m_PlaneShaderView);
	CreateDDSTextureFromFile(m_device, L"Textures/DSC_4439.dds", NULL, &m_PlaneShaderView);
	CreateDDSTextureFromFile(m_device, L"Textures/brickwall.dds", NULL, &m_SecondPlaneShaderView);
	//setting up desc for skybox
	CreateDDSTextureFromFile(m_device, L"mp_rip/OutputCube.dds", NULL, &m_SkyBoxShaderView);

	//raster desc for skybox
	m_RasterDesc.CullMode = D3D11_CULL_NONE;

	//created raster state for the skybox
	m_device->CreateRasterizerState(&m_RasterDesc, &m_rasterForSkybox);

	//setting up smaple desc
	m_texture.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	m_texture.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_texture.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	m_texture.BorderColor[0] = 0;
	m_texture.BorderColor[1] = 0;
	m_texture.BorderColor[2] = 0;
	m_texture.BorderColor[3] = 0;
	m_texture.ComparisonFunc = D3D11_COMPARISON_NEVER;
	m_texture.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	m_texture.MaxAnisotropy = 1;
	m_texture.MaxLOD = 0;
	m_texture.MinLOD = 0;
	m_texture.MipLODBias = 0;

	m_device->CreateSamplerState(&m_texture, &m_sampleTexture);
}

bool D3D_DEMO::Run()
{
	//rotation of the star
	toObject.worldMatrix = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(5.0f)), toObject.worldMatrix);
	// Camera Movement
	XMFLOAT3 vec3;


	if (GetAsyncKeyState('W') & 0x1)
	{
		temp.view.r[3].m128_f32[2] += 0.05f;	
	}

	if (GetAsyncKeyState('S') & 0x1)
	{
		temp.view.r[3].m128_f32[2] -= 0.05f;
		//m_light.lightDirection.z -= 0.02f
	}

	if (GetAsyncKeyState('A') & 0x1)
	{
		temp.view.r[3].m128_f32[0] -= 0.05f;
	}

	if (GetAsyncKeyState('D') & 0x1)
	{
		temp.view.r[3].m128_f32[0] += 0.05f;
	}

	if (GetAsyncKeyState(VK_UP) & 0x1)
	{
		temp.view.r[3].m128_f32[1] += 0.05f;
	}

	if (GetAsyncKeyState(VK_DOWN) & 0x1)
	{
		temp.view.r[3].m128_f32[1] -= 0.05f;
	}

	//setting up the camera rotation
	if (GetAsyncKeyState(VK_NUMPAD6) & 0x1)
	{
		temp.view = XMMatrixMultiply(XMMatrixRotationY(0.05f), temp.view);
	}

	if (GetAsyncKeyState(VK_NUMPAD4) & 0x1)
	{
		temp.view = XMMatrixMultiply(XMMatrixRotationY(-0.05f), temp.view);
	}

	if (GetAsyncKeyState('I') & 0x1)
	{
		//m_light.diffuseColor.z += 0.10f;
		m_light.lightDirection.z += 0.10f;
	}
	if (GetAsyncKeyState('K') & 0x1)
	{
		//m_light.diffuseColor.z -= 0.10f;
		m_light.lightDirection.z -= 0.10f;
	}
	if (GetAsyncKeyState('J') & 0x1)
	{
		//m_light.diffuseColor.x -= 0.10f;
		m_light.lightDirection.x -= 0.10f;
	}
	if (GetAsyncKeyState('L') & 0x1)
	{
		//m_light.diffuseColor.x += 0.10f;
		m_light.lightDirection.x += 0.10f;
	}
	//movement code for pointlight
	if(GetAsyncKeyState('F') & 0x1)
	{
		//m_light.diffuseColor.z += 0.10f;
		m_pointLight.lightPos.z += 0.10f;
	}
	if (GetAsyncKeyState('V') & 0x1)
	{
		//m_light.diffuseColor.z -= 0.10f;
		m_pointLight.lightPos.z -= 0.10f;
	}
	if (GetAsyncKeyState('C') & 0x1)
	{
		//m_light.diffuseColor.x -= 0.10f;
		m_pointLight.lightPos.x -= 0.10f;
	}
	if (GetAsyncKeyState('B') & 0x1)
	{
		//m_light.diffuseColor.x += 0.10f;
		m_pointLight.lightPos.x += 0.10f;
	}

	//movement code for spotlight
	if (GetAsyncKeyState('Y') & 0x1)
	{
		m_spotLight.lightPosition.z += 0.10f;
	}

	if (GetAsyncKeyState('H') & 0x1)
	{
		m_spotLight.lightPosition.z -= 0.10f;
	}

	if (GetAsyncKeyState('T') & 0x1)
	{
		m_spotLight.lightPosition.x -= 0.10f;
	}

	if (GetAsyncKeyState('U') & 0x1)
	{
		m_spotLight.lightPosition.x += 0.10f;
	}

	vec3.x = temp.view.r[3].m128_f32[0];
	vec3.y = temp.view.r[3].m128_f32[1];
	vec3.z = temp.view.r[3].m128_f32[2];

	temp.view.r[3].m128_f32[0] = 0;
	temp.view.r[3].m128_f32[1] = 0;
	temp.view.r[3].m128_f32[2] = 0;

	if (GetAsyncKeyState(VK_NUMPAD5) & 0x1)
	{
		temp.view = XMMatrixMultiply(XMMatrixRotationX(-0.02), temp.view);
	}

	if (GetAsyncKeyState(VK_NUMPAD8) & 0x1)
	{
		temp.view = XMMatrixMultiply(XMMatrixRotationX(0.02), temp.view);
	}

	temp.view.r[3].m128_f32[0] = vec3.x;
	temp.view.r[3].m128_f32[1] = vec3.y;
	temp.view.r[3].m128_f32[2] = vec3.z;

	skyboxWorld.r[3].m128_f32[0] = vec3.x;
	skyboxWorld.r[3].m128_f32[1] = vec3.y;
	skyboxWorld.r[3].m128_f32[2] = vec3.z;
	//orientation of model
	toScene.proejctionMatrix = temp.projection;
	//desc of camera
	toScene.viewMatrix = XMMatrixInverse(NULL, temp.view);

	m_deviceContext->RSSetState(m_rasterState);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	m_deviceContext->RSSetViewports(1, &m_viewport);

	float black[4] = { 0,0,0,0 };
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, black);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	
	D3D11_MAPPED_SUBRESOURCE m_mapSource;
	m_deviceContext->Map(m_cBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource);
	SEND_TO_OBJECT *m_temp = ((SEND_TO_OBJECT*)m_mapSource.pData);
	*m_temp = toObject;
	m_deviceContext->Unmap(m_cBuffer[0], 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSource2;
	m_deviceContext->Map(m_cBuffer[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource2);
	SEND_TO_SCENE *temp2 = ((SEND_TO_SCENE*)m_mapSource2.pData);
	*temp2 = toScene;
	m_deviceContext->Unmap(m_cBuffer[1], 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForLight;
	m_deviceContext->Map(m_cBufferForLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForLight);
	LIGHT_VERTEX *m_Light = ((LIGHT_VERTEX*)m_mapSourceForLight.pData);
	*m_Light = m_light;
	m_deviceContext->Unmap(m_cBufferForLight, 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForPointLight;
	m_deviceContext->Map(m_cBufferForPointLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForPointLight);
	POINT_LIGHT_VERTEX *m_PointLight = ((POINT_LIGHT_VERTEX*)m_mapSourceForPointLight.pData);
	*m_PointLight = m_pointLight;
	m_deviceContext->Unmap(m_cBufferForPointLight, 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForSpotLight;
	m_deviceContext->Map(m_cBufferForSpotLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForSpotLight);
	SPOT_LIGHT_VERTEX *m_SpotLight = ((SPOT_LIGHT_VERTEX*)m_mapSourceForSpotLight.pData);
	*m_SpotLight = m_spotLight;
	m_deviceContext->Unmap(m_cBufferForSpotLight, 0);

	unsigned int stride = sizeof(SIMPLE_VERTEX);
	unsigned int offsets = 0;

	//skybox
	m_deviceContext->RSSetState(m_rasterForSkybox);
	m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
	m_deviceContext->PSSetShader(m_pixelShaderForSkybox, NULL, NULL);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_skyboxVertexBuffer, &stride, &offsets);
	m_deviceContext->IASetIndexBuffer(m_skyboxIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->PSSetShaderResources(0, 1, &m_SkyBoxShaderView);
	m_deviceContext->PSSetSamplers(0, 1, &m_sampleTexture);
	m_deviceContext->DrawIndexed(modelSizeForSkybox, 0, 0);

	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	//for rest of the drawing
	toObject.worldMatrix = temp.world;
	m_deviceContext->Map(m_cBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource);
	m_temp = ((SEND_TO_OBJECT*)m_mapSource.pData);
	*m_temp = toObject;
	m_deviceContext->Unmap(m_cBuffer[0], 0);


	//star
	m_deviceContext->VSSetConstantBuffers(0, 2, m_cBuffer);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_matrixBuffer, &stride, &offsets);
	m_deviceContext->PSSetShader(m_pixelShader, NULL, NULL);
	m_deviceContext->GSSetShader(NULL, NULL, NULL);
	m_deviceContext->IASetIndexBuffer(m_iBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->DrawIndexed(60, 0, 0);
	
	//drawing for the quad through geometry shader
	stride = sizeof(GEO_VERTEX);
	m_deviceContext->VSSetShader(m_vertexShaderForGeometry, NULL, NULL);
	m_deviceContext->PSSetShader(m_pixelShaderForGeometry, NULL, NULL);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBufferForGeometry, &stride, &offsets);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_deviceContext->IASetInputLayout(m_layoutForGeometryShader);
	m_deviceContext->GSSetConstantBuffers(0, 2, m_cBuffer);
	m_deviceContext->GSSetShader(m_geometryShader, NULL, NULL);
	m_deviceContext->Draw(3,0);

	//directional toggle
	if (GetAsyncKeyState('2') & 0x1)
	{
		m_light.diffuseColor = { 1.0f,1.0f,1.0f,1.0f };
	}
	if (GetAsyncKeyState('1') & 0x1)
	{
		m_light.diffuseColor = { 0.4f,1.0f,0.4f,0.4f};
	}
	
	//pointlight toggle
	if (GetAsyncKeyState('3') & 0x1)
	{
		m_pointLight.diffuseColor = { 0.4f,0.4f,1.0f,0.4f};
	}

	if (GetAsyncKeyState('4') & 0x1)
	{
		m_pointLight.diffuseColor = { 1.0f,1.0f,1.0f,1.0f };
	}

	//spotlight toggle
	if (GetAsyncKeyState('5') & 0x1)
	{
		m_spotLight.diffuseColor = { 1.0f,0.4f,0.4f,0.4f };
	}
	if (GetAsyncKeyState('6') & 0x1)
	{
		m_spotLight.diffuseColor = { 1.0, 1.0f, 1.0f, 1.0f };
	}

	m_deviceContext->PSSetConstantBuffers(0, 1, &m_cBufferForLight);
	m_deviceContext->PSSetConstantBuffers(1, 1, &m_cBufferForPointLight);
	m_deviceContext->PSSetConstantBuffers(2, 1, &m_cBufferForSpotLight);

	////plane
	//m_deviceContext->VSGetConstantBuffers(0, 2, m_cBuffer);
	//m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
	//m_deviceContext->IASetVertexBuffers(0, 1, &m_planeVertexBuffer, &stride, &offsets);
	//m_deviceContext->IASetIndexBuffer(m_planeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//m_deviceContext->PSSetShaderResources(0, 1, &m_PlaneShaderView);
	//m_deviceContext->PSSetShaderResources(1, 1, &m_SecondPlaneShaderView);
	//m_deviceContext->DrawIndexed(modelSizeForPlane, 0, 0);
	//m_deviceContext->PSSetShaderResources(1, 1, &m_NullShaderView);


	//Ogre_model
	stride = sizeof(SIMPLE_VERTEX);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
	m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
	m_deviceContext->GSSetShader(NULL, NULL, NULL);
	m_deviceContext->IASetIndexBuffer(m_modelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_modelVertexBuffer, &stride, &offsets);
	m_deviceContext->PSSetShaderResources(0, 1, &m_textureView);
	m_deviceContext->DrawIndexed(modelSize, 0, 0);

	// plane
	m_deviceContext->VSGetConstantBuffers(0, 2, m_cBuffer);
	m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
	m_deviceContext->GSSetShader(NULL, NULL, NULL);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_planeVertexBuffer, &stride, &offsets);
	m_deviceContext->IASetIndexBuffer(m_planeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->PSSetShaderResources(0, 1, &m_PlaneShaderView);
	m_deviceContext->PSSetShaderResources(1, 1, &m_SecondPlaneShaderView);
	m_deviceContext->DrawIndexed(modelSizeForPlane, 0, 0);
	m_deviceContext->PSSetShaderResources(1, 1, &m_NullShaderView);

	toObject.worldMatrix = skyboxWorld;

	m_SwapChain->Present(0, 0);

	return true;
}

bool D3D_DEMO::ShutDown()
{
	m_device->Release();
	m_SwapChain->Release();
	m_deviceContext->Release();

	m_cBuffer[0]->Release();
	m_cBuffer[1]->Release();
	m_cBufferForLight->Release();
	m_cBufferForPointLight->Release();
	m_cBufferForSpotLight->Release();
	m_iBuffer->Release();
	z_buffer->Release();
	m_matrixBuffer->Release();
	m_depthStencilView->Release();

	UnregisterClass(L"DirectXApplication", application);
	return true;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	D3D_DEMO myApp(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY) : { PostQuitMessage(0); }
						break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}