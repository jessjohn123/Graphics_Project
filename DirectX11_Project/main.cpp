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
#include "Trivial_VS_ForGeometry.csh"
#include "Trivial_VS_ForNormalMapping.csh"
#include "Trivial_VS_ForTransparency.csh"
#include "Trivial_PS.csh"
#include "Trivial_PS_Texture.csh"
#include "Trivial_PS_For_Skybox.csh"
#include "Trivial_PS_ForGeometry.csh"
#include "Trivial_PS_ForNormalMapping.csh"
#include "Trivial_PS_ForTransparency.csh"
#include "Trivial_GS.csh"
#include "LoadModel.h"
#include "DDSTextureLoader.h"
#include "XTime.h"
#include <thread>

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

//#define BACKBUFFER_WIDTH 1280	
//#define BACKBUFFER_HEIGHT 720

//*******************************************//
//********* SIMPLE WINDOW APP CLASS ********//
//*****************************************//

#define SAFE_RELEASE(p) {if(p){p->Release(); p = nullptr;}}

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
		XMFLOAT3 tangent;
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

	struct DIR_VERTEX
	{
		XMFLOAT4 diffColor;
		XMFLOAT3 lightDirt;
		float padding;
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
	ID3D11RenderTargetView* m_renderTarView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11DepthStencilView* m_depthStencilViewForRenderToScene;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11RasterizerState* m_rasterState;
	ID3D11RasterizerState* m_rasterForSkybox;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cBuffer[2];
	ID3D11Buffer* m_cBufferForLight;
	ID3D11Buffer* m_cBufferForPointLight;
	ID3D11Buffer* m_cBufferForSpotLight;
	ID3D11Buffer* m_cBufferForNormalMap;								 // constant buffer normal map
	ID3D11Buffer* m_iBuffer;
	ID3D11Texture2D* z_buffer;
	ID3D11VertexShader* m_vertexShader;
	ID3D11VertexShader* m_vertexShaderForGeometry;
	ID3D11VertexShader* m_vertexShaderForNormalMap;						// normal map vertex shader
	ID3D11VertexShader* m_vertexPointerForNM;
	ID3D11PixelShader* m_pixelShaderForGeometry;
	ID3D11PixelShader* m_pixelShader;
	ID3D11PixelShader* m_pixelShaderForTexture;
	ID3D11PixelShader* m_pixelShaderForSkybox;
	ID3D11PixelShader* m_pixelPointerForNM;
	ID3D11PixelShader* m_pixelShaderForNormalMap;						//normal map pixel shader
	ID3D11GeometryShader* m_geometryShader;
	ID3D11InputLayout* m_layout;
	ID3D11InputLayout* m_layoutForGeometryShader;
	ID3D11InputLayout* m_layoutForNormalMap;							//normal map input layer
	ID3D11InputLayout* m_inputLayerPointForNM;
	D3D11_VIEWPORT m_viewport;
	D3D11_VIEWPORT m_secondViewPort;
	D3D11_VIEWPORT m_thirdViewPort;
	D3D_FEATURE_LEVEL m_featureLevel;
	unsigned int num_of_vertices;
	SEND_TO_OBJECT toObject;
	SEND_TO_SCENE toScene;
	MatrixBufferType temp;
	MatrixBufferType m_tempForVP;
	SIMPLE_VERTEX* m_model;
	
	LoadMesh loadObj;
	unsigned int modelSize;
	ID3D11Buffer *m_modelVertexBuffer, *m_modelIndexBuffer;
	SIMPLE_VERTEX* m_skyModel;
	
	unsigned int modelSizeForSkybox;
	ID3D11Buffer * m_skyboxVertexBuffer, *m_skyboxIndexBuffer;
	SIMPLE_VERTEX* m_planeModel;
	
	unsigned int modelSizeForPlane;
	ID3D11Buffer *m_planeVertexBuffer, *m_planeIndexBuffer;
	ID3D11Texture2D *m_texture;
	ID3D11ShaderResourceView *m_textureView;
	ID3D11ShaderResourceView *m_PlaneShaderView;
	ID3D11ShaderResourceView *m_SkyBoxShaderView;
	ID3D11ShaderResourceView *m_NullShaderView = NULL;
	ID3D11ShaderResourceView *m_NormalMapView;								// Normal Map shader resource
	//ID3D11ShaderResourceView *m_secondNormalMapView;
	ID3D11ShaderResourceView *m_GeoShaderView = NULL;
	//ID3D11ShaderResourceView *m_secondNormalMapView;
	ID3D11ShaderResourceView *m_shaderResourcePointerForNM;
	//ID3D11ShaderResourceView *m_shaderResourcePointerForNM2;
	ID3D11ShaderResourceView *m_shaderForRenderToScene;
	ID3D11Texture2D *m_secondTexture;
	ID3D11Texture2D *m_texToRenderToScene;
	ID3D11Texture2D *m_secondTexToRenderToScene;
	ID3D11ShaderResourceView *m_SecondPlaneShaderView;
	ID3D11SamplerState *m_sampleTexture;
	XMMATRIX skyboxWorld;
	ID3D11Buffer *m_lightBuffer;	
	ID3D11Buffer * m_PointLightBuffer;
	ID3D11Buffer * m_SpotLightBuffer;
	//ID3D11Buffer *m_DirLightBufferForNormalMap;								// Normal Map light buffer
	DIR_VERTEX m_lightForNormalMap;
	LIGHT_VERTEX m_light;
	POINT_LIGHT_VERTEX m_pointLight;
	SPOT_LIGHT_VERTEX m_spotLight;
	SIMPLE_VERTEX m_ForQuad;
	ID3D11Buffer *m_vertexBufferForGeometry;
	GEO_VERTEX m_geoVertex;
	ID3D11Debug*  DebugDevice;
	ID3D11Texture2D* ptrToBackBuffer;
	bool bSplitScreen;
	XTime time;

	
	

	unsigned int indices[60] = { 0,1,10,1,2,10,2,3,10,3,4,10,4,5,10,5,6,10,6,7,10,7,8,10,8,9,10,9,0,10,
		0,9,11,9,8,11,8,7,11,7,6,11,6,5,11,5,4,11,4,3,11,3,2,11,2,1,11,1,0,11 };

	D3D_DEMO() {}
public:
	void Initialize(HINSTANCE hinst, WNDPROC proc);
	static D3D_DEMO *GetInstance();
	bool Run();
	void ReportLiveObjects();
	bool ShutDown();
	void ResizingOfWindows();
	void secondViewPort();
	void thirdViewPort();
	void RenderToScene();
	void CreateTangents(SIMPLE_VERTEX* model, UINT numIndicies);
	void LoadTheModel();
	void LoadTheGround();
	void LoadTheSkyBox();

	void SaveBinary(const char* _file, const vector<SIMPLE_VERTEX> _vertices, const vector<UINT> _indices);
	bool LoadBinary(const char* _file, vector<SIMPLE_VERTEX>& _vertices, vector<UINT>& _indices);
};

void D3D_DEMO::Initialize(HINSTANCE hinst, WNDPROC proc)
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

	window = CreateWindow(L"DirectXApplication", L"Lab 1a Line Land", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	DXGI_RATIONAL m_rational;
	D3D11_BUFFER_DESC m_vertexBuffer = {};
	D3D11_SUBRESOURCE_DATA m_vertexData = {};
	D3D11_BUFFER_DESC m_constantBuffer = {};
	D3D11_SUBRESOURCE_DATA m_constantData = {};
	D3D11_BUFFER_DESC m_indexBuffer = {};
	D3D11_SUBRESOURCE_DATA m_indexData = {};
	D3D11_TEXTURE2D_DESC m_texture2D = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC d_stencil = {};
	D3D11_RASTERIZER_DESC m_RasterDesc = {};
	//D3D11_TEXTURE2D_DESC textureDesc;
	//D3D11_SHADER_RESOURCE_VIEW_DESC texViewDesc;
	D3D11_TEXTURE2D_DESC secondTextureDesc;
	//D3D11_SHADER_RESOURCE_VIEW_DESC secondTexViewDesc;
	
	D3D11_SAMPLER_DESC m_texture;
	//D3D11_BUFFER_DESC lightBufferForNormalMapDesc = {};
	D3D11_BUFFER_DESC lightBufferDesc = {};
	D3D11_BUFFER_DESC point_lightBufferDesc = {};
	D3D11_BUFFER_DESC spotLightBufferDesc = {};
	D3D11_BUFFER_DESC m_constantBufferForLight = {};
	D3D11_SUBRESOURCE_DATA m_constantDataForLight = {};
	D3D11_BUFFER_DESC m_constantBufferForPointLight = {};
	D3D11_BUFFER_DESC m_constantBufferForDirLightForNM = {};
	D3D11_SUBRESOURCE_DATA m_constantDataForDirLightForNM = {};
	D3D11_SUBRESOURCE_DATA m_constantDataForPointLight = {};
	D3D11_BUFFER_DESC m_constantBufferForSpotLight = {};
	D3D11_SUBRESOURCE_DATA m_constantDataForSpotLight = {};
	D3D11_BUFFER_DESC m_vertexBufferDescForGeometry = {};
	D3D11_SUBRESOURCE_DATA m_vertexDataForGeometry = {};

	

	//Render the scene to the texture
	D3D11_TEXTURE2D_DESC texDescForRenderToScene;
	D3D11_TEXTURE2D_DESC secondTexDescForRenderToScene;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewForRenderToScene = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDescForRenderToScene = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC d_stencilForRenderToScene = {};


	swapChainDesc.BufferDesc.Width = BACKBUFFER_WIDTH;
	swapChainDesc.BufferDesc.Height = BACKBUFFER_HEIGHT;

	m_rational.Numerator = 0;
	m_rational.Denominator = 1;

	swapChainDesc.BufferDesc.RefreshRate = m_rational;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SampleDesc.Count = 4;
	swapChainDesc.SampleDesc.Quality = NULL;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.OutputWindow = window;
	swapChainDesc.Windowed = true;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	m_viewport.Width = BACKBUFFER_WIDTH;
	m_viewport.Height = BACKBUFFER_HEIGHT;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	/*m_secondViewPort.Width = BACKBUFFER_WIDTH * 0.5;
	m_secondViewPort.Height = BACKBUFFER_HEIGHT * 0.5;
	m_secondViewPort.MinDepth = 0.0f;
	m_secondViewPort.MaxDepth = 1.0f;
	m_secondViewPort.TopLeftX = 0;
	m_secondViewPort.TopLeftY = 0;*/

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

	//Loading through single thread

	vector<thread>loadingThreads;
	
	loadingThreads.push_back(thread(&D3D_DEMO::LoadTheModel, this));
	loadingThreads.push_back(thread(&D3D_DEMO::LoadTheGround, this));
	loadingThreads.push_back(thread(&D3D_DEMO::LoadTheSkyBox, this));

	size_t count = loadingThreads.size();

	for (int i = 0; i < count; i++)
	{
		loadingThreads[i].join();
	}

	loadingThreads.clear();

	//Loading the ogre model
/*
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
	indicesForN.clear();*/

	//LoadTheModel();

	//loading the plane
	/*loadObj.LoadObj("Plane.obj", vert_vertices, text_vertices, norm_vertices, indicesForV, indicesForT, indicesForN);

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

	CreateTangents(m_planeModel, indicesForV.size());

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
	indicesForN.clear();*/

	//LoadTheGround();

	//loading cube
	/*loadObj.LoadObj("Cube.obj", vert_vertices, text_vertices, norm_vertices, indicesForV, indicesForT, indicesForN);

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

	m_device->CreateBuffer(&skymodelIndDesc, &skyindexData, &m_skyboxIndexBuffer);*/

	//LoadTheSkyBox();

	//setting up the desc of the vertex buffer for star
	m_vertexBuffer.Usage = D3D11_USAGE_IMMUTABLE;
	m_vertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_vertexBuffer.CPUAccessFlags = NULL;
	m_vertexBuffer.ByteWidth = sizeof(SIMPLE_VERTEX) * num_of_vertices;

	//give the subresource structure a pointer to the vertex data
	m_vertexData.pSysMem = ver_of_star;

	m_device->CreateBuffer(&m_vertexBuffer, &m_vertexData, &m_matrixBuffer);


	//Setting up the vertex and pixel shaders
	m_device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &m_vertexShader);
	m_device->CreateVertexShader(Trivial_VS_ForGeometry, sizeof(Trivial_VS_ForGeometry), NULL, &m_vertexShaderForGeometry);
	m_device->CreateVertexShader(Trivial_VS_ForNormalMapping, sizeof(Trivial_VS_ForNormalMapping), NULL, &m_vertexShaderForNormalMap); // normal map vertex shader
	m_device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &m_pixelShader);
	m_device->CreatePixelShader(Trivial_PS_Texture, sizeof(Trivial_PS_Texture), NULL, &m_pixelShaderForTexture);
	m_device->CreatePixelShader(Trivial_PS_For_Skybox, sizeof(Trivial_PS_For_Skybox), NULL, &m_pixelShaderForSkybox);
	m_device->CreatePixelShader(Trivial_PS_ForGeometry, sizeof(Trivial_PS_ForGeometry), NULL, &m_pixelShaderForGeometry);
	m_device->CreatePixelShader(Trivial_PS_ForNormalMapping, sizeof(Trivial_PS_ForNormalMapping), NULL, &m_pixelShaderForNormalMap);   // normal map pixel shader
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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_device->CreateInputLayout(m_inputLayoutForGS, ARRAYSIZE(m_inputLayoutForGS), Trivial_VS_ForGeometry, sizeof(Trivial_VS_ForGeometry), &m_layoutForGeometryShader);
	
	//setting up the input layout for the normal map taking into account tangent and binormal
	D3D11_INPUT_ELEMENT_DESC m_inputLayoutForNM[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TANGENT",0, DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	m_device->CreateInputLayout(m_inputLayoutForNM, ARRAYSIZE(m_inputLayoutForNM), Trivial_VS_ForNormalMapping, sizeof(Trivial_VS_ForNormalMapping), &m_layoutForNormalMap);
	
	
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


	//Constant Buffer directional light set up for Normal Map
	m_lightForNormalMap.diffColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	//setting the direction of light to positive z-axis
	m_lightForNormalMap.lightDirt = { 0.0f, 0.0f, 1.0f };        
	
	//setting up the desc of the dir light for normal map
	m_constantBufferForDirLightForNM.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_constantBufferForDirLightForNM.Usage = D3D11_USAGE_DYNAMIC;
	m_constantBufferForDirLightForNM.ByteWidth = sizeof(DIR_VERTEX);
	m_constantBufferForDirLightForNM.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//setting up the desc for dir light data
	m_constantDataForDirLightForNM.pSysMem = &m_lightForNormalMap;
	m_device->CreateBuffer(&m_constantBufferForDirLightForNM, &m_constantDataForDirLightForNM, &m_cBufferForNormalMap);

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
	m_pointLight.lightPos = {0.0f, -4.0f, 0.0f};
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

	//setting up the view for third viewport
	m_tempForVP.view = XMMatrixIdentity();
	m_tempForVP.world = XMMatrixIdentity();
	m_tempForVP.view.r[3].m128_f32[2] = -5;

	//setting up the projection for rendering 3D model
	toScene.proejctionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(60), ASPECTRATIO, SCREEN_NEAR, SCREEN_DEPTH);

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
	m_geoVertex.m_position = { -4,0,0,1 };
	m_geoVertex.m_color = { 1,0,0,1 };
	GEO_VERTEX m_quadPos[2];
	m_quadPos[0] = m_geoVertex;

	m_geoVertex.m_position = { 4,0,0,1 };
	m_geoVertex.m_color = { 1,0,0,1 };
	m_quadPos[1] = m_geoVertex;

	/*m_geoVertex.m_position = { -6,0,-25,1 };
	m_geoVertex.m_color = { 1,0,0,1 };
	m_quadPos[2] = m_geoVertex;*/

	//setting up vertex buffer for the geometry shader
	m_vertexBufferDescForGeometry.Usage = D3D11_USAGE_IMMUTABLE;
	m_vertexBufferDescForGeometry.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_vertexBufferDescForGeometry.CPUAccessFlags = NULL;
	m_vertexBufferDescForGeometry.ByteWidth = sizeof(GEO_VERTEX)*2;

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
	m_texture2D.SampleDesc.Count = 4;
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
	secondTextureDesc.SampleDesc.Count = 4;
	secondTextureDesc.CPUAccessFlags = 0;
	secondTextureDesc.MiscFlags = 0;
	m_device->CreateTexture2D(&secondTextureDesc, NULL, &m_secondTexture);

	//for setting up the quad
	m_ForQuad.position = XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f);
	m_ForQuad.color = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);

	d_stencil.Format = DXGI_FORMAT_D32_FLOAT;
	d_stencil.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d_stencil.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(z_buffer, NULL, &m_depthStencilView);

	//setting up the raster
	m_RasterDesc.FillMode = D3D11_FILL_SOLID;
	m_RasterDesc.CullMode = D3D11_CULL_BACK;
	m_RasterDesc.FrontCounterClockwise = false;
	m_RasterDesc.DepthBias = NULL;
	m_RasterDesc.SlopeScaledDepthBias = 0.0f;
	m_RasterDesc.DepthBiasClamp = 0.0f;
	m_RasterDesc.DepthClipEnable = true;
	m_RasterDesc.ScissorEnable = false;
	m_RasterDesc.MultisampleEnable = true;
	m_RasterDesc.AntialiasedLineEnable = true;

	m_device->CreateRasterizerState(&m_RasterDesc, &m_rasterState);

	//setting up the desc for the first texture(color) for rendering to the scene
	ZeroMemory(&texDescForRenderToScene, sizeof(texDescForRenderToScene));
	texDescForRenderToScene.Width = BACKBUFFER_WIDTH;
	texDescForRenderToScene.Height = BACKBUFFER_HEIGHT;
	texDescForRenderToScene.Usage = D3D11_USAGE_DEFAULT;
	texDescForRenderToScene.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDescForRenderToScene.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDescForRenderToScene.MipLevels = 1;
	texDescForRenderToScene.ArraySize = 1;
	texDescForRenderToScene.SampleDesc.Count = 1;
	texDescForRenderToScene.CPUAccessFlags = 0;
	texDescForRenderToScene.MiscFlags = 0;
	m_device->CreateTexture2D(&texDescForRenderToScene, NULL, &m_texToRenderToScene);

	//For the depth texture 
	ZeroMemory(&secondTexDescForRenderToScene, sizeof(secondTexDescForRenderToScene));
	secondTexDescForRenderToScene.Width = BACKBUFFER_WIDTH;
	secondTexDescForRenderToScene.Height = BACKBUFFER_HEIGHT;
	secondTexDescForRenderToScene.Usage = D3D11_USAGE_DEFAULT;
	secondTexDescForRenderToScene.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	secondTexDescForRenderToScene.Format = DXGI_FORMAT_D32_FLOAT;
	secondTexDescForRenderToScene.MipLevels = 1;
	secondTexDescForRenderToScene.ArraySize = 1;
	secondTexDescForRenderToScene.SampleDesc.Count = 1;
	secondTexDescForRenderToScene.CPUAccessFlags = 0;
	secondTexDescForRenderToScene.MiscFlags = 0;
	m_device->CreateTexture2D(&secondTexDescForRenderToScene, NULL, &m_secondTexToRenderToScene);


	//setting up the desc for the depth stencil view
	d_stencilForRenderToScene.Format = DXGI_FORMAT_D32_FLOAT;
	d_stencilForRenderToScene.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d_stencilForRenderToScene.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(m_secondTexToRenderToScene, NULL, &m_depthStencilViewForRenderToScene);
	
	//setting up the render target view for rendering the scene 
	renderTargetViewForRenderToScene.Format = texDescForRenderToScene.Format;
	renderTargetViewForRenderToScene.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewForRenderToScene.Texture2D.MipSlice = 0;
	m_device->CreateRenderTargetView(m_texToRenderToScene,NULL, &m_renderTarView);


	//setting up the shader resource view for rendering the scene
	shaderResViewDescForRenderToScene.Format = texDescForRenderToScene.Format;
	shaderResViewDescForRenderToScene.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDescForRenderToScene.Texture2D.MostDetailedMip = 0;
	shaderResViewDescForRenderToScene.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView(m_texToRenderToScene, NULL, &m_shaderForRenderToScene);


	//passing info inside the dds texture loader
	CreateDDSTextureFromFile(m_device, L"OgreTextures/SkinColorMostro_COLOR.dds", NULL, &m_textureView);
	//CreateDDSTextureFromFile(m_device, NULL, NULL, &m_NullShaderView);
	//passing info inside the dds texture loader for plane
	//CreateDDSTextureFromFile(m_device, L"Textures/checkerboard.dds", NULL, &m_PlaneShaderView);
	CreateDDSTextureFromFile(m_device, L"Textures/Seamless ground sand texture (5).dds", NULL, &m_PlaneShaderView);
	CreateDDSTextureFromFile(m_device, L"Textures/brickwall.dds", NULL, &m_SecondPlaneShaderView);
	//setting up desc for skybox
	CreateDDSTextureFromFile(m_device, L"mp_rip/OutputCube.dds", NULL, &m_SkyBoxShaderView);
	//setting up desc for loading normal map texture
	CreateDDSTextureFromFile(m_device, L"Textures/NormalMap (1).dds", NULL, &m_NormalMapView);
	//CreateDDSTextureFromFile(m_device, L"Textures/NormalMap (4).dds", NULL, &m_secondNormalMapView);
	


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

	m_shaderResourcePointerForNM = m_PlaneShaderView;
	m_vertexPointerForNM = m_vertexShader;
	m_pixelPointerForNM = m_pixelShaderForTexture;
	m_inputLayerPointForNM = m_layout;

	m_shaderResourcePointerForNM = m_SecondPlaneShaderView;
	//m_shaderResourcePointerForNM = m_PlaneShaderView;
	//m_shaderResourcePointerForNM2 = m_SecondPlaneShaderView;

	bSplitScreen = false;
}

void D3D_DEMO::LoadTheModel()
{

	std::vector<float3> vert_vertices, norm_vertices;
	std::vector<float2> text_vertices;
	std::vector<unsigned int> indicesForV, indicesForT, indicesForN;
	//unsigned int *vert_indices, *text_indices, *norm_indices;

	//loading the 3D-Model : Ogre , calling the load obj function

	vector<SIMPLE_VERTEX>_vertices;
	vector<UINT>_indices;

	//vert_vertices.resize(indicesForV.size());
	if (LoadBinary("OgreOBJ.bin",_vertices, _indices ) == false)
	{
	loadObj.LoadObj("OgreOBJ.obj", vert_vertices, text_vertices, norm_vertices, indicesForV, indicesForT, indicesForN);

		std::vector<unsigned int> vert_indices(indicesForV.size());
		std::vector<unsigned int> text_indices(indicesForT.size());
		std::vector<unsigned int> norm_indices(indicesForN.size());

		/*vert_indices = new unsigned int[indicesForV.size()];
		text_indices = new unsigned int[indicesForT.size()];
		norm_indices = new unsigned int[indicesForN.size()];*/

		/*unsigned int* model_indices = new unsigned int[indicesForV.size()];

		m_model = new SIMPLE_VERTEX[indicesForV.size()];*/

		modelSize = indicesForV.size();
		_vertices.resize(modelSize);
		_indices.resize(modelSize);

		for (unsigned int i = 0; i < modelSize; i++)
		{
			vert_indices[i] = indicesForV[i];
			norm_indices[i] = indicesForN[i];
			text_indices[i] = indicesForT[i];
			_indices[i] = i;
		}

		for (unsigned int j = 0; j < indicesForV.size(); j++)
		{
			_vertices[j].position.x = vert_vertices[vert_indices[j]].x + 5;
			_vertices[j].position.y = vert_vertices[vert_indices[j]].y - 5;
			_vertices[j].position.z = vert_vertices[vert_indices[j]].z;

			_vertices[j].color = { 0, 1, 0, 0 };
			_vertices[j].texture.x = text_vertices[text_indices[j]].x;
			_vertices[j].texture.y = text_vertices[text_indices[j]].y;
			_vertices[j].normal.x = norm_vertices[norm_indices[j]].x;
			_vertices[j].normal.y = norm_vertices[norm_indices[j]].y;
			_vertices[j].normal.z = norm_vertices[norm_indices[j]].z;
		}


		SaveBinary("OgreOBJ.bin", _vertices, _indices);
	}
	modelSize = _vertices.size();


	//setting up model vertex buffer
	D3D11_BUFFER_DESC modelVertDesc;
	modelVertDesc.Usage = D3D11_USAGE_IMMUTABLE;
	modelVertDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	modelVertDesc.CPUAccessFlags = NULL;
	modelVertDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * _vertices.size();
	modelVertDesc.MiscFlags = NULL;
	modelVertDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA modelData;
	modelData.pSysMem = &_vertices[0];
	modelData.SysMemPitch = 0;
	modelData.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&modelVertDesc, &modelData, &m_modelVertexBuffer);

	//setting up the index buffer
	D3D11_BUFFER_DESC modelIndDesc;
	modelIndDesc.Usage = D3D11_USAGE_DEFAULT;
	modelIndDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	modelIndDesc.CPUAccessFlags = NULL;
	modelIndDesc.ByteWidth = sizeof(unsigned int) * _indices.size();
	modelIndDesc.MiscFlags = NULL;
	modelIndDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = &_indices[0];
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



}

void D3D_DEMO::LoadTheGround()
{
	std::vector<float3> vert_vertices, norm_vertices;
	std::vector<float2> text_vertices;
	std::vector<unsigned int> indicesForV, indicesForT, indicesForN;

	//unsigned int *m_vertForPlane, *m_textForPlane, *m_normForPlane;
	
	//loading the plane, calling the load function

	vector<SIMPLE_VERTEX> _vertices;
	vector<UINT> _indices;

	loadObj.LoadObj("Plane.obj", vert_vertices, text_vertices, norm_vertices, indicesForV, indicesForT, indicesForN);
	if (LoadBinary("Plane.bin", _vertices, _indices) == false)
	{
	

		std::vector<unsigned int>m_vertForPlane(indicesForV.size());
		std::vector<unsigned int>m_textForPlane(indicesForT.size());
		std::vector<unsigned int>m_normForPlane(indicesForN.size());

		/*m_vertForPlane = new unsigned int[indicesForV.size()];
		m_textForPlane = new unsigned int[indicesForT.size()];
		m_normForPlane = new unsigned int[indicesForN.size()];*/

		/*unsigned int* plane_model_indices = new unsigned int[indicesForV.size()];

		m_planeModel = new SIMPLE_VERTEX[indicesForV.size()];*/

		modelSizeForPlane = indicesForV.size();
		_vertices.resize(modelSizeForPlane);
		_indices.resize(modelSizeForPlane);

		for (unsigned int i = 0; i < modelSizeForPlane; i++)
		{
			m_vertForPlane[i] = indicesForV[i];
			m_normForPlane[i] = indicesForN[i];
			m_textForPlane[i] = indicesForT[i];
			_indices[i] = i;
		}

		for (unsigned int j = 0; j < indicesForV.size(); j++)
		{
			_vertices[j].position.x = vert_vertices[m_vertForPlane[j]].x;
			_vertices[j].position.y = vert_vertices[m_vertForPlane[j]].y - 5;
			_vertices[j].position.z = vert_vertices[m_vertForPlane[j]].z;

			_vertices[j].color = { 0, 1, 0, 0 };
			_vertices[j].texture.x = text_vertices[m_textForPlane[j]].x;
			_vertices[j].texture.y = text_vertices[m_textForPlane[j]].y;
			_vertices[j].normal.x = norm_vertices[m_normForPlane[j]].x;
			_vertices[j].normal.y = norm_vertices[m_normForPlane[j]].y;
			_vertices[j].normal.z = norm_vertices[m_normForPlane[j]].z;
		}

		SaveBinary("Plane.bin", _vertices, _indices);
	}

	modelSizeForPlane = _vertices.size();

	CreateTangents(&_vertices[0]/*m_planeModel*/, indicesForV.size());

	//setting up the plane model vertex buffer
	D3D11_BUFFER_DESC modelVertDescForPlane;
	modelVertDescForPlane.Usage = D3D11_USAGE_IMMUTABLE;
	modelVertDescForPlane.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	modelVertDescForPlane.CPUAccessFlags = NULL;
	modelVertDescForPlane.ByteWidth = sizeof(SIMPLE_VERTEX) * _vertices.size();
	modelVertDescForPlane.MiscFlags = NULL;
	modelVertDescForPlane.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA modelDataForPlane;
	modelDataForPlane.pSysMem = &_vertices[0];
	modelDataForPlane.SysMemPitch = 0;
	modelDataForPlane.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&modelVertDescForPlane, &modelDataForPlane, &m_planeVertexBuffer);

	//setting up the plane model index buffer
	D3D11_BUFFER_DESC modelIndDescForPlane;
	modelIndDescForPlane.Usage = D3D11_USAGE_DEFAULT;
	modelIndDescForPlane.BindFlags = D3D11_BIND_INDEX_BUFFER;
	modelIndDescForPlane.CPUAccessFlags = NULL;
	modelIndDescForPlane.ByteWidth = sizeof(unsigned int) * _indices.size();
	modelIndDescForPlane.MiscFlags = NULL;
	modelIndDescForPlane.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexDataForPlane;
	indexDataForPlane.pSysMem = &_indices[0];
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

}

void D3D_DEMO::LoadTheSkyBox()
{
	std::vector<float3> vert_vertices, norm_vertices;
	std::vector<float2> text_vertices;
	std::vector<unsigned int> indicesForV, indicesForT, indicesForN;

	//unsigned int *m_vert, *m_text, *m_norm;

	//loading the skybox, calling the load obj funcion
	loadObj.LoadObj("Cube.obj", vert_vertices, text_vertices, norm_vertices, indicesForV, indicesForT, indicesForN);

	std::vector<unsigned int>m_vert(indicesForV.size());
	std::vector<unsigned int>m_text(indicesForT.size());
	std::vector<unsigned int>m_norm(indicesForN.size());


	/*m_vert = new unsigned int[indicesForV.size()];
	m_text = new unsigned int[indicesForT.size()];
	m_norm = new unsigned int[indicesForN.size()];*/

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
	skyindexData.pSysMem = &m_vert[0];
	skyindexData.SysMemPitch = 0;
	skyindexData.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&skymodelIndDesc, &skyindexData, &m_skyboxIndexBuffer);
}


void D3D_DEMO::SaveBinary(const char* _file, const vector<SIMPLE_VERTEX> _vertices, const vector<UINT> _indices)
{
	fstream out;

	out.open(_file, ios_base::binary | ios_base::out);

	if (out.is_open())
	{
		size_t vertCount = _vertices.size();
		size_t indexCount = _indices.size();

		if (vertCount > 0)
		{
			out.write((char*)&vertCount, sizeof(vertCount));
			out.write((char*)&_vertices[0], sizeof(SIMPLE_VERTEX) * vertCount);
		}

		if (indexCount > 0)
		{
			out.write((char*)&indexCount, sizeof(indexCount));
			out.write((char*)&_indices[0], sizeof(UINT) * indexCount);
		}

	
	}

	out.close();
}

bool D3D_DEMO::LoadBinary(const char* _file, vector<SIMPLE_VERTEX>& _vertices, vector<UINT>& _indices)
{
	ifstream in;

	in.open(_file, ios_base::binary | ios_base::in);

	if (in.is_open())
	{
		size_t vertCount = 0;
		in.read((char*)&vertCount, sizeof(vertCount));

		if (vertCount > 0)
		{
			_vertices.resize(vertCount);
			in.read((char*)&_vertices[0], sizeof(SIMPLE_VERTEX) * vertCount);
		}


		size_t indexCount = 0;
		in.read((char*)&indexCount, sizeof(indexCount));

		if (indexCount > 0)
		{
			_indices.resize(indexCount);
			in.read((char*)&_indices[0], sizeof(UINT) * indexCount);
		}
	}
	else
		return false;


	in.close();
	return true;
}


void D3D_DEMO::CreateTangents(SIMPLE_VERTEX* model, UINT numIndicies)
{
	for (unsigned int j = 0; j < numIndicies; j += 3)
	{
		// find the 3 verts that pertain to this triangle
		SIMPLE_VERTEX vert1 = model[j];
		SIMPLE_VERTEX vert2 = model[j + 1];
		SIMPLE_VERTEX vert3 = model[j + 2];

		SIMPLE_VERTEX Edge1, Edge2;
		Edge1.position.x = vert2.position.x - vert1.position.x;
		Edge1.position.y = vert2.position.y - vert1.position.y;
		Edge1.position.z = vert2.position.z - vert1.position.z;

		Edge2.position.x = vert3.position.x - vert1.position.x;
		Edge2.position.y = vert3.position.y - vert1.position.y;
		Edge2.position.z = vert3.position.z - vert1.position.z;


		SIMPLE_VERTEX TextEdge1, TextEdge2;
		TextEdge1.position.x = vert2.texture.x - vert1.texture.x;
		TextEdge1.position.y = vert2.texture.y - vert1.texture.y;

		TextEdge2.position.x = vert3.texture.x - vert1.texture.x;
		TextEdge2.position.y = vert3.texture.y - vert1.texture.y;

		// do the fancy math in the powerpoint slides
		float m_ratio;
		XMVECTOR m_dotResult, m_Tangent;
		m_ratio = 1.0f / (TextEdge1.position.x * TextEdge2.position.y - TextEdge2.position.x * TextEdge1.position.y);

		XMFLOAT3 uDir, vDir;
		uDir = XMFLOAT3(((TextEdge2.position.y * Edge1.position.x - TextEdge1.position.y * Edge2.position.x) * m_ratio),
					    ((TextEdge2.position.y * Edge1.position.y - TextEdge1.position.y * Edge2.position.y) * m_ratio),
					    ((TextEdge2.position.y * Edge1.position.z - TextEdge1.position.y * Edge2.position.z) * m_ratio));

		vDir = XMFLOAT3(((TextEdge1.position.x * Edge2.position.x - TextEdge2.position.x * Edge1.position.x) * m_ratio),
						((TextEdge1.position.x * Edge2.position.y - TextEdge2.position.x * Edge1.position.y) * m_ratio),
						((TextEdge1.position.x * Edge2.position.z - TextEdge2.position.x * Edge1.position.z) * m_ratio));

		// insert the value you found (tangent) into the vector of SIMPLE_VERTEX
		XMVECTOR m_normal = XMVectorSet(model[j].normal.x, model[j].normal.y, model[j].normal.z, 0.0f);
		XMVECTOR uDirt = XMVectorSet(uDir.x, uDir.y, uDir.z, 0.0f);
		m_dotResult = XMVector3Dot(m_normal, uDirt);
		m_Tangent = uDirt - m_normal * m_dotResult.m128_f32[0];

		m_Tangent = XMVector3Normalize(-m_Tangent);

		 XMStoreFloat3(&model[j].tangent, m_Tangent);
		 XMStoreFloat3(&model[j + 1].tangent, m_Tangent);
		 XMStoreFloat3(&model[j + 2].tangent, m_Tangent);

	}

}

void D3D_DEMO::ResizingOfWindows()
{
	if (m_SwapChain == nullptr)
	{
		return;
	}
 
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(z_buffer);
	SAFE_RELEASE(ptrToBackBuffer);

	m_deviceContext->ClearState();
	m_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);  //swapchain flag , width and height -> backbuffer w/h
	HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&ptrToBackBuffer);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	m_SwapChain->GetDesc(&swapChainDesc);


	m_viewport.Width = swapChainDesc.BufferDesc.Width;
	m_viewport.Height = swapChainDesc.BufferDesc.Height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_deviceContext->RSSetViewports(1, &m_viewport);

	m_device->CreateRenderTargetView(ptrToBackBuffer, NULL, &m_renderTargetView);
	
	D3D11_TEXTURE2D_DESC m_texture2D = {};

	D3D11_DEPTH_STENCIL_VIEW_DESC d_stencil = {};

	m_texture2D.Width = m_viewport.Width;
	m_texture2D.Height = m_viewport.Height;
	m_texture2D.Usage = D3D11_USAGE_DEFAULT;
	m_texture2D.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	m_texture2D.Format = DXGI_FORMAT_D32_FLOAT;
	m_texture2D.MipLevels = 1;
	m_texture2D.ArraySize = 1;
	m_texture2D.SampleDesc.Count = 4;
	m_device->CreateTexture2D(&m_texture2D, NULL, &z_buffer);

	d_stencil.Format = DXGI_FORMAT_D32_FLOAT;
	d_stencil.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d_stencil.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(z_buffer, NULL, &m_depthStencilView);


	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	toScene.proejctionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(60), m_viewport.Width/m_viewport.Height, SCREEN_NEAR, SCREEN_DEPTH);

}

D3D_DEMO * D3D_DEMO::GetInstance()
{
	static D3D_DEMO instance;
	return &instance;
}

bool D3D_DEMO::Run()
{
	time.Signal();

	//Movement code
	XMFLOAT3 vec3;
	if (GetAsyncKeyState('W'))
	{
		//temp.view.r[3].m128_f32[2] += 0.02f;
		temp.view.r[3].m128_f32[2] += time.Delta();
	}

	if (GetAsyncKeyState('S'))
	{
		//temp.view.r[3].m128_f32[2] -= 0.02f;
		//m_light.lightDirection.z -= 0.02f
		temp.view.r[3].m128_f32[2] -= time.Delta();

	}

	if (GetAsyncKeyState('A'))
	{
		//temp.view.r[3].m128_f32[0] -= 0.02f;
		temp.view.r[3].m128_f32[0] -= time.Delta();
	}

	if (GetAsyncKeyState('D'))
	{
		//temp.view.r[3].m128_f32[0] += 0.02f;
		temp.view.r[3].m128_f32[0] += time.Delta();
	}

	if (GetAsyncKeyState(VK_UP))
	{
		temp.view.r[3].m128_f32[1] += time.Delta();
	}

	if (GetAsyncKeyState(VK_DOWN))
	{
		temp.view.r[3].m128_f32[1] -= time.Delta();
	}

	//setting up the camera rotation
	if (GetAsyncKeyState(VK_NUMPAD6))
	{
		//temp.view = XMMatrixMultiply(XMMatrixRotationY(0.02), temp.view);
		temp.view = XMMatrixMultiply(XMMatrixRotationY(time.Delta()), temp.view);
	}

	if (GetAsyncKeyState(VK_NUMPAD4))
	{
		//temp.view = XMMatrixMultiply(XMMatrixRotationY(-0.02f), temp.view);
		temp.view = XMMatrixMultiply(XMMatrixRotationY(-(time.Delta())), temp.view);
	}

	vec3.x = temp.view.r[3].m128_f32[0];
	vec3.y = temp.view.r[3].m128_f32[1];
	vec3.z = temp.view.r[3].m128_f32[2];

	temp.view.r[3].m128_f32[0] = 0;
	temp.view.r[3].m128_f32[1] = 0;
	temp.view.r[3].m128_f32[2] = 0;

	if (GetAsyncKeyState('I'))
	{
		//m_light.diffuseColor.z += 0.10f;
		m_light.lightDirection.z += time.Delta();
	}
	if (GetAsyncKeyState('K'))
	{
		//m_light.diffuseColor.z -= 0.10f;
		m_light.lightDirection.z -= time.Delta();
	}
	if (GetAsyncKeyState('J'))
	{
		//m_light.diffuseColor.x -= 0.10f;
		m_light.lightDirection.x -= time.Delta();
	}
	if (GetAsyncKeyState('L'))
	{
		//m_light.diffuseColor.x += 0.10f;
		m_light.lightDirection.x += time.Delta();
	}
	//movement code for pointlight
	if (GetAsyncKeyState('F'))
	{
		//m_light.diffuseColor.z += 0.10f;
		m_pointLight.lightPos.z += time.Delta();
	}
	if (GetAsyncKeyState('V'))
	{
		//m_light.diffuseColor.z -= 0.10f;
		m_pointLight.lightPos.z -= time.Delta();
	}
	if (GetAsyncKeyState('C'))
	{
		//m_light.diffuseColor.x -= 0.10f;
		m_pointLight.lightPos.x -= time.Delta();
	}
	if (GetAsyncKeyState('B'))
	{
		//m_light.diffuseColor.x += 0.10f;
		m_pointLight.lightPos.x += time.Delta();
	}

	//movement code for spotlight
	if (GetAsyncKeyState('Y'))
	{
		m_spotLight.lightPosition.z += time.Delta();
	}

	if (GetAsyncKeyState('H'))
	{
		m_spotLight.lightPosition.z -= time.Delta();
	}

	if (GetAsyncKeyState('T'))
	{
		m_spotLight.lightPosition.x -= time.Delta();
	}

	if (GetAsyncKeyState('U'))
	{
		m_spotLight.lightPosition.x += time.Delta();
	}

	if (GetAsyncKeyState(VK_NUMPAD5))
	{
		temp.view = XMMatrixMultiply(XMMatrixRotationX(-(time.Delta())), temp.view);
	}

	if (GetAsyncKeyState(VK_NUMPAD8))
	{
		temp.view = XMMatrixMultiply(XMMatrixRotationX(time.Delta()), temp.view);
	}

	temp.view.r[3].m128_f32[0] = vec3.x;
	temp.view.r[3].m128_f32[1] = vec3.y;
	temp.view.r[3].m128_f32[2] = vec3.z;

	skyboxWorld.r[3].m128_f32[0] = vec3.x;
	skyboxWorld.r[3].m128_f32[1] = vec3.y;
	skyboxWorld.r[3].m128_f32[2] = vec3.z;

	//directional toggle
	if (GetAsyncKeyState('2') & 0x1)
	{
		m_light.diffuseColor = { 1.0f,1.0f,1.0f,1.0f };
	}
	if (GetAsyncKeyState('1') & 0x1)
	{
		m_light.diffuseColor = { 0.4f,1.0f,0.4f,0.4f };
	}

	//pointlight toggle
	if (GetAsyncKeyState('3') & 0x1)
	{
		m_pointLight.diffuseColor = { 0.4f,0.4f,1.0f,0.4f };
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


	if (GetAsyncKeyState(VK_SPACE) & 0x1)
	{
		bSplitScreen = !bSplitScreen;
	}

	RenderToScene();

	if(bSplitScreen == false)
	{
		//rotation of the star
		toObject.worldMatrix = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(5.0f)), toObject.worldMatrix);
		
		//desc of camera
		toScene.viewMatrix = XMMatrixInverse(NULL, temp.view);
		
		m_deviceContext->RSSetState(m_rasterState);
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

		m_deviceContext->RSSetViewports(1, &m_viewport);

		float black[4] = { 0,0,0,0 };
		m_deviceContext->ClearRenderTargetView(m_renderTargetView, black);
		m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		toObject.worldMatrix = skyboxWorld;

		D3D11_MAPPED_SUBRESOURCE m_mapSource;
		m_deviceContext->Map(m_cBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource);
		SEND_TO_OBJECT *m_temp = ((SEND_TO_OBJECT*)m_mapSource.pData);
		*m_temp = toObject;
		//memcpy(m_mapSource.pData, &toObject, sizeof(&toObject));
		m_deviceContext->Unmap(m_cBuffer[0], 0);

		D3D11_MAPPED_SUBRESOURCE m_mapSource2;
		m_deviceContext->Map(m_cBuffer[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource2);
		SEND_TO_SCENE *temp2 = ((SEND_TO_SCENE*)m_mapSource2.pData);
		*temp2 = toScene;
		//memcpy(m_mapSource2.pData, &toScene, sizeof(&toScene));
		m_deviceContext->Unmap(m_cBuffer[1], 0);

		D3D11_MAPPED_SUBRESOURCE m_mapSourceForLight;
		m_deviceContext->Map(m_cBufferForLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForLight);
		LIGHT_VERTEX *m_Light = ((LIGHT_VERTEX*)m_mapSourceForLight.pData);
		*m_Light = m_light;
		//memcpy(m_mapSourceForLight.pData, &m_light, sizeof(&m_light));
		m_deviceContext->Unmap(m_cBufferForLight, 0);

		D3D11_MAPPED_SUBRESOURCE m_mapSourceForPointLight;
		m_deviceContext->Map(m_cBufferForPointLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForPointLight);
		POINT_LIGHT_VERTEX *m_PointLight = ((POINT_LIGHT_VERTEX*)m_mapSourceForPointLight.pData);
		*m_PointLight = m_pointLight;
		//memcpy(m_mapSourceForPointLight.pData, &m_pointLight, sizeof(&m_pointLight));
		m_deviceContext->Unmap(m_cBufferForPointLight, 0);

		D3D11_MAPPED_SUBRESOURCE m_mapSourceForSpotLight;
		m_deviceContext->Map(m_cBufferForSpotLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForSpotLight);
		SPOT_LIGHT_VERTEX *m_SpotLight = ((SPOT_LIGHT_VERTEX*)m_mapSourceForSpotLight.pData);
		*m_SpotLight = m_spotLight;
		//memcpy(m_mapSourceForSpotLight.pData, &m_spotLight, sizeof(&m_spotLight));
		m_deviceContext->Unmap(m_cBufferForSpotLight, 0);

		unsigned int stride = sizeof(SIMPLE_VERTEX);
		unsigned int offsets = 0;

		//skybox
		m_deviceContext->RSSetState(m_rasterForSkybox);
		m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
		m_deviceContext->VSSetConstantBuffers(0, 2, m_cBuffer);
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
		m_deviceContext->PSSetShaderResources(0, 1, &m_shaderForRenderToScene);
		m_deviceContext->Draw(2, 0);
		m_deviceContext->PSSetShaderResources(0, 1, &m_GeoShaderView);

		//setting up the constant buffers for three diff lights
		m_deviceContext->PSSetConstantBuffers(0, 1, &m_cBufferForLight);
		m_deviceContext->PSSetConstantBuffers(1, 1, &m_cBufferForPointLight);
		m_deviceContext->PSSetConstantBuffers(2, 1, &m_cBufferForSpotLight);

		//Ogre_model
		stride = sizeof(SIMPLE_VERTEX);
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_deviceContext->GSSetShader(NULL, NULL, NULL);
		m_deviceContext->IASetInputLayout(m_layout);
		m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
		m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
		m_deviceContext->IASetIndexBuffer(m_modelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_deviceContext->IASetVertexBuffers(0, 1, &m_modelVertexBuffer, &stride, &offsets);
		m_deviceContext->PSSetShaderResources(0, 1, &m_textureView);
		m_deviceContext->DrawIndexed(modelSize, 0, 0);
		
		// check for the button press
		if (GetAsyncKeyState('7'))
		{
			m_pixelPointerForNM = m_pixelShaderForTexture;
			m_vertexPointerForNM = m_vertexShader;
			m_inputLayerPointForNM = m_layout;
			
			m_shaderResourcePointerForNM = m_SecondPlaneShaderView; 

			/*m_shaderResourcePointerForNM = m_PlaneShaderView;
			m_shaderResourcePointerForNM2 = m_SecondPlaneShaderView;*/
		}
		if (GetAsyncKeyState('8'))
		{
			m_pixelPointerForNM = m_pixelShaderForNormalMap;
			m_vertexPointerForNM = m_vertexShaderForNormalMap;
			m_inputLayerPointForNM = m_layoutForNormalMap;
			
			m_shaderResourcePointerForNM = m_NormalMapView;  

			/*m_shaderResourcePointerForNM = m_NormalMapView;
			m_shaderResourcePointerForNM2 = m_secondNormalMapView;*/
	
		}

		// plane
		m_deviceContext->VSSetConstantBuffers(0, 2, m_cBuffer);
		//m_deviceContext->VSSetShader(m_vertexShaderForNormalMap, NULL, NULL);
		m_deviceContext->VSSetShader(m_vertexPointerForNM, NULL, NULL);
		//m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
		//m_deviceContext->PSSetShader(m_pixelShaderForNormalMap, NULL, NULL);
		m_deviceContext->PSSetShader(m_pixelPointerForNM, NULL, NULL);
		//m_deviceContext->IASetInputLayout(m_layoutForNormalMap);
		m_deviceContext->IASetInputLayout(m_inputLayerPointForNM);
		m_deviceContext->GSSetShader(NULL, NULL, NULL);
		m_deviceContext->IASetVertexBuffers(0, 1, &m_planeVertexBuffer, &stride, &offsets);
		m_deviceContext->IASetIndexBuffer(m_planeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		
		/*m_deviceContext->PSSetShaderResources(0, 1, &m_PlaneShaderView);
		m_deviceContext->PSSetShaderResources(1, 1, &m_SecondPlaneShaderView);
		m_deviceContext->PSSetShaderResources(2, 1, &m_NormalMapView);
		m_deviceContext->PSSetShaderResources(3, 1, &m_secondNormalMapView);
		m_deviceContext->PSSetShaderResources(0, 1, &m_shaderResourcePointerForNM);
		m_deviceContext->PSSetShaderResources(1, 1, &m_shaderResourcePointerForNM2);*/

		m_deviceContext->PSSetShaderResources(0, 1, &m_PlaneShaderView);
		m_deviceContext->PSSetShaderResources(1, 1, &m_shaderResourcePointerForNM);			//the original way
		
		m_deviceContext->DrawIndexed(modelSizeForPlane, 0, 0);
		m_deviceContext->PSSetShaderResources(1, 1, &m_NullShaderView);
	}

	else
	{
		secondViewPort();
		thirdViewPort();
	}


	m_SwapChain->Present(1, 0);

	return true;
}

void D3D_DEMO::RenderToScene()
{
	//desc of cam
	toScene.viewMatrix = XMMatrixInverse(NULL, temp.view);

	m_deviceContext->RSSetState(m_rasterState);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTarView, m_depthStencilViewForRenderToScene);

	m_deviceContext->RSSetViewports(1, &m_viewport);

	float blue[4] = { 0, 0, 1, 0 };
	m_deviceContext->ClearRenderTargetView(m_renderTarView, blue);
	m_deviceContext->ClearDepthStencilView(m_depthStencilViewForRenderToScene, D3D11_CLEAR_DEPTH, 1.0f, 0);

	toObject.worldMatrix = skyboxWorld;

	D3D11_MAPPED_SUBRESOURCE m_mapSource;
	m_deviceContext->Map(m_cBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource);
	SEND_TO_OBJECT *m_temp = ((SEND_TO_OBJECT*)m_mapSource.pData);
	*m_temp = toObject;
	//memcpy(m_mapSource.pData, &toObject, sizeof(&toObject));
	m_deviceContext->Unmap(m_cBuffer[0], 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSource2;
	m_deviceContext->Map(m_cBuffer[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource2);
	SEND_TO_SCENE *temp2 = ((SEND_TO_SCENE*)m_mapSource2.pData);
	*temp2 = toScene;
	//memcpy(m_mapSource2.pData, &toScene, sizeof(&toScene));
	m_deviceContext->Unmap(m_cBuffer[1], 0);

	
	unsigned int stride = sizeof(SIMPLE_VERTEX);
	unsigned int offsets = 0;

	//skybox
	m_deviceContext->RSSetState(m_rasterForSkybox);
	m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
	m_deviceContext->VSSetConstantBuffers(0, 2, m_cBuffer);
	m_deviceContext->PSSetShader(m_pixelShaderForSkybox, NULL, NULL);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_skyboxVertexBuffer, &stride, &offsets);
	m_deviceContext->IASetIndexBuffer(m_skyboxIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->PSSetShaderResources(0, 1, &m_SkyBoxShaderView);
	m_deviceContext->PSSetSamplers(0, 1, &m_sampleTexture);
	m_deviceContext->DrawIndexed(modelSizeForSkybox, 0, 0);

	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

}
void D3D_DEMO::secondViewPort()
{
	//rotation of the star
	toObject.worldMatrix = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(5.0f)), toObject.worldMatrix);
	
	DXGI_SWAP_CHAIN_DESC m_swapChainDesc;
	m_SwapChain->GetDesc(&m_swapChainDesc);

	m_secondViewPort.Width = m_swapChainDesc.BufferDesc.Width * 0.5f;
	m_secondViewPort.Height = m_swapChainDesc.BufferDesc.Height * 0.5f;
	m_secondViewPort.MinDepth = 0.0f;
	m_secondViewPort.MaxDepth = 1.0f;
	m_secondViewPort.TopLeftX = 0;
	m_secondViewPort.TopLeftY = 0;
	
	//orientation of model
	toScene.proejctionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(60), m_secondViewPort.Width / m_secondViewPort.Height, SCREEN_NEAR, SCREEN_DEPTH);
	//desc of camera
	toScene.viewMatrix = XMMatrixInverse(NULL, temp.view);

	m_deviceContext->RSSetState(m_rasterState);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	m_deviceContext->RSSetViewports(1, &m_secondViewPort);

	float black[4] = { 0,0,0,0 };
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, black);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	toObject.worldMatrix = skyboxWorld;

	D3D11_MAPPED_SUBRESOURCE m_mapSource;
	m_deviceContext->Map(m_cBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource);
	SEND_TO_OBJECT *m_temp = ((SEND_TO_OBJECT*)m_mapSource.pData);
	*m_temp = toObject;
	//memcpy(m_mapSource.pData, &toObject, sizeof(&toObject));
	m_deviceContext->Unmap(m_cBuffer[0], 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSource2;
	m_deviceContext->Map(m_cBuffer[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource2);
	SEND_TO_SCENE *temp2 = ((SEND_TO_SCENE*)m_mapSource2.pData);
	*temp2 = toScene;
	//memcpy(m_mapSource2.pData, &toScene, sizeof(&toScene));
	m_deviceContext->Unmap(m_cBuffer[1], 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForLight;
	m_deviceContext->Map(m_cBufferForLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForLight);
	LIGHT_VERTEX *m_Light = ((LIGHT_VERTEX*)m_mapSourceForLight.pData);
	*m_Light = m_light;
	//memcpy(m_mapSourceForLight.pData, &m_light, sizeof(&m_light));
	m_deviceContext->Unmap(m_cBufferForLight, 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForPointLight;
	m_deviceContext->Map(m_cBufferForPointLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForPointLight);
	POINT_LIGHT_VERTEX *m_PointLight = ((POINT_LIGHT_VERTEX*)m_mapSourceForPointLight.pData);
	*m_PointLight = m_pointLight;
	//memcpy(m_mapSourceForPointLight.pData, &m_pointLight, sizeof(&m_pointLight));
	m_deviceContext->Unmap(m_cBufferForPointLight, 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForSpotLight;
	m_deviceContext->Map(m_cBufferForSpotLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForSpotLight);
	SPOT_LIGHT_VERTEX *m_SpotLight = ((SPOT_LIGHT_VERTEX*)m_mapSourceForSpotLight.pData);
	*m_SpotLight = m_spotLight;
	//memcpy(m_mapSourceForSpotLight.pData, &m_spotLight, sizeof(&m_spotLight));
	m_deviceContext->Unmap(m_cBufferForSpotLight, 0);

	unsigned int stride = sizeof(SIMPLE_VERTEX);
	unsigned int offsets = 0;

	//skybox
	m_deviceContext->RSSetState(m_rasterForSkybox);
	m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
	m_deviceContext->VSSetConstantBuffers(0, 2, m_cBuffer);
	m_deviceContext->PSSetShader(m_pixelShaderForSkybox, NULL, NULL);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_skyboxVertexBuffer, &stride, &offsets);
	m_deviceContext->IASetIndexBuffer(m_skyboxIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->PSSetShaderResources(0, 1, &m_SkyBoxShaderView);
	m_deviceContext->PSSetSamplers(0, 1, &m_sampleTexture);
	m_deviceContext->DrawIndexed(modelSizeForSkybox, 0, 0);

	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	//star
	toObject.worldMatrix = temp.world;
	m_deviceContext->Map(m_cBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource);
	m_temp = ((SEND_TO_OBJECT*)m_mapSource.pData);
	*m_temp = toObject;
	m_deviceContext->Unmap(m_cBuffer[0], 0);

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
	m_deviceContext->PSSetShaderResources(0, 1, &m_shaderForRenderToScene);
	m_deviceContext->Draw(2, 0);
	m_deviceContext->PSSetShaderResources(0, 1, &m_GeoShaderView);

	//setting up the constant buffers for three diff lights
	m_deviceContext->PSSetConstantBuffers(0, 1, &m_cBufferForLight);
	m_deviceContext->PSSetConstantBuffers(1, 1, &m_cBufferForPointLight);
	m_deviceContext->PSSetConstantBuffers(2, 1, &m_cBufferForSpotLight);

	//Ogre_model
	stride = sizeof(SIMPLE_VERTEX);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->GSSetShader(NULL, NULL, NULL);
	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
	m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
	m_deviceContext->IASetIndexBuffer(m_modelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_modelVertexBuffer, &stride, &offsets);
	m_deviceContext->PSSetShaderResources(0, 1, &m_textureView);
	m_deviceContext->DrawIndexed(modelSize, 0, 0);

	// plane
	m_deviceContext->VSSetConstantBuffers(0, 2, m_cBuffer);
	m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
	m_deviceContext->GSSetShader(NULL, NULL, NULL);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_planeVertexBuffer, &stride, &offsets);
	m_deviceContext->IASetIndexBuffer(m_planeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->PSSetShaderResources(0, 1, &m_PlaneShaderView);
	m_deviceContext->PSSetShaderResources(1, 1, &m_SecondPlaneShaderView);
	m_deviceContext->DrawIndexed(modelSizeForPlane, 0, 0);
	m_deviceContext->PSSetShaderResources(1, 1, &m_NullShaderView);

}

void D3D_DEMO::thirdViewPort()
{
	//rotation of the star
	toObject.worldMatrix = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(5.0f)), toObject.worldMatrix);

	DXGI_SWAP_CHAIN_DESC _swapChainDesc;
	m_SwapChain->GetDesc(&_swapChainDesc);

	m_thirdViewPort.Width = _swapChainDesc.BufferDesc.Width * 0.5;
	m_thirdViewPort.Height = _swapChainDesc.BufferDesc.Height * 0.5;
	m_thirdViewPort.MinDepth = 0.0f;
	m_thirdViewPort.MaxDepth = 1.0f;
	m_thirdViewPort.TopLeftX = m_thirdViewPort.Width;
	m_thirdViewPort.TopLeftY = m_thirdViewPort.Height;


	//orientation of model
	toScene.proejctionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(60), m_thirdViewPort.Width / m_thirdViewPort.Height, SCREEN_NEAR, SCREEN_DEPTH);
	//desc of camera
	toScene.viewMatrix = XMMatrixInverse(NULL, m_tempForVP.view);

	m_deviceContext->RSSetState(m_rasterState);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	m_deviceContext->RSSetViewports(1, &m_thirdViewPort);

	float black[4] = { 0,0,0,0 };
	//m_deviceContext->ClearRenderTargetView(m_renderTargetView, black);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	toObject.worldMatrix = m_tempForVP.view;

	D3D11_MAPPED_SUBRESOURCE m_mapSource;
	m_deviceContext->Map(m_cBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource);
	SEND_TO_OBJECT *m_temp = ((SEND_TO_OBJECT*)m_mapSource.pData);
	*m_temp = toObject;
	//memcpy(m_mapSource.pData, &toObject, sizeof(&toObject));
	m_deviceContext->Unmap(m_cBuffer[0], 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSource2;
	m_deviceContext->Map(m_cBuffer[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource2);
	SEND_TO_SCENE *temp2 = ((SEND_TO_SCENE*)m_mapSource2.pData);
	*temp2 = toScene;
	//memcpy(m_mapSource2.pData, &toScene, sizeof(&toScene));
	m_deviceContext->Unmap(m_cBuffer[1], 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForLight;
	m_deviceContext->Map(m_cBufferForLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForLight);
	LIGHT_VERTEX *m_Light = ((LIGHT_VERTEX*)m_mapSourceForLight.pData);
	*m_Light = m_light;
	//memcpy(m_mapSourceForLight.pData, &m_light, sizeof(&m_light));
	m_deviceContext->Unmap(m_cBufferForLight, 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForPointLight;
	m_deviceContext->Map(m_cBufferForPointLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForPointLight);
	POINT_LIGHT_VERTEX *m_PointLight = ((POINT_LIGHT_VERTEX*)m_mapSourceForPointLight.pData);
	*m_PointLight = m_pointLight;
	//memcpy(m_mapSourceForPointLight.pData, &m_pointLight, sizeof(&m_pointLight));
	m_deviceContext->Unmap(m_cBufferForPointLight, 0);

	D3D11_MAPPED_SUBRESOURCE m_mapSourceForSpotLight;
	m_deviceContext->Map(m_cBufferForSpotLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSourceForSpotLight);
	SPOT_LIGHT_VERTEX *m_SpotLight = ((SPOT_LIGHT_VERTEX*)m_mapSourceForSpotLight.pData);
	*m_SpotLight = m_spotLight;
	//memcpy(m_mapSourceForSpotLight.pData, &m_spotLight, sizeof(&m_spotLight));
	m_deviceContext->Unmap(m_cBufferForSpotLight, 0);

	unsigned int stride = sizeof(SIMPLE_VERTEX);
	unsigned int offsets = 0;

	//skybox
	m_deviceContext->RSSetState(m_rasterForSkybox);
	m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
	m_deviceContext->VSSetConstantBuffers(0, 2, m_cBuffer);
	m_deviceContext->PSSetShader(m_pixelShaderForSkybox, NULL, NULL);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_skyboxVertexBuffer, &stride, &offsets);
	m_deviceContext->IASetIndexBuffer(m_skyboxIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->PSSetShaderResources(0, 1, &m_SkyBoxShaderView);
	m_deviceContext->PSSetSamplers(0, 1, &m_sampleTexture);
	m_deviceContext->DrawIndexed(modelSizeForSkybox, 0, 0);

	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	//star
	toObject.worldMatrix = XMMatrixIdentity();
	m_deviceContext->Map(m_cBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mapSource);
	m_temp = ((SEND_TO_OBJECT*)m_mapSource.pData);
	*m_temp = toObject;
	m_deviceContext->Unmap(m_cBuffer[0], 0);

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
	m_deviceContext->PSSetShaderResources(0, 1, &m_shaderForRenderToScene);
	m_deviceContext->Draw(2, 0);
	m_deviceContext->PSSetShaderResources(0, 1, &m_GeoShaderView);

	//setting up the constant buffers for three diff lights
	m_deviceContext->PSSetConstantBuffers(0, 1, &m_cBufferForLight);
	m_deviceContext->PSSetConstantBuffers(1, 1, &m_cBufferForPointLight);
	m_deviceContext->PSSetConstantBuffers(2, 1, &m_cBufferForSpotLight);

	//Ogre_model
	stride = sizeof(SIMPLE_VERTEX);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->GSSetShader(NULL, NULL, NULL);
	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->VSSetShader(m_vertexShader, NULL, NULL);
	m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
	m_deviceContext->IASetIndexBuffer(m_modelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_modelVertexBuffer, &stride, &offsets);
	m_deviceContext->PSSetShaderResources(0, 1, &m_textureView);
	m_deviceContext->DrawIndexed(modelSize, 0, 0);

	// plane
	m_deviceContext->VSSetConstantBuffers(0, 2, m_cBuffer);
	m_deviceContext->PSSetShader(m_pixelShaderForTexture, NULL, NULL);
	m_deviceContext->GSSetShader(NULL, NULL, NULL);
	m_deviceContext->IASetVertexBuffers(0, 1, &m_planeVertexBuffer, &stride, &offsets);
	m_deviceContext->IASetIndexBuffer(m_planeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->PSSetShaderResources(0, 1, &m_PlaneShaderView);
	m_deviceContext->PSSetShaderResources(1, 1, &m_SecondPlaneShaderView);
	m_deviceContext->DrawIndexed(modelSizeForPlane, 0, 0);
	m_deviceContext->PSSetShaderResources(1, 1, &m_NullShaderView);
}

void D3D_DEMO::ReportLiveObjects()
{
	#ifdef _DEBUG
	HRESULT result = m_device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast <void**>(&DebugDevice));
	//CHECKERROR
	result = DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	//CHECKERROR
	DebugDevice->Release();
	#endif 
}

bool D3D_DEMO::ShutDown()
{
	m_SwapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(m_SwapChain);
	SAFE_RELEASE(m_deviceContext);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_renderTarView);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(m_depthStencilViewForRenderToScene);
	SAFE_RELEASE(m_rasterState);
	SAFE_RELEASE(m_rasterForSkybox);
	SAFE_RELEASE(m_matrixBuffer);
	SAFE_RELEASE(m_cBuffer[0]);
	SAFE_RELEASE(m_cBuffer[1]);
	SAFE_RELEASE(m_cBufferForLight);
	SAFE_RELEASE(m_cBufferForPointLight);
	SAFE_RELEASE(m_cBufferForSpotLight);
	SAFE_RELEASE(m_cBufferForNormalMap);
	SAFE_RELEASE(m_iBuffer);
	SAFE_RELEASE(z_buffer);
	SAFE_RELEASE(m_texToRenderToScene);
	SAFE_RELEASE(m_secondTexToRenderToScene);
	SAFE_RELEASE(m_vertexShader);
	SAFE_RELEASE(m_vertexShaderForGeometry);
	//SAFE_RELEASE(m_vertexPointerForNM);
	SAFE_RELEASE(m_vertexShaderForNormalMap);
	SAFE_RELEASE(m_pixelShader);
	SAFE_RELEASE(m_pixelShaderForGeometry);
	SAFE_RELEASE(m_pixelShaderForTexture);
	SAFE_RELEASE(m_pixelShaderForSkybox);
	//SAFE_RELEASE(m_pixelPointerForNM);
	SAFE_RELEASE(m_pixelShaderForNormalMap);
	SAFE_RELEASE(m_geometryShader);
	SAFE_RELEASE(m_layout);
	SAFE_RELEASE(m_layoutForGeometryShader);
	SAFE_RELEASE(m_layoutForNormalMap);
	SAFE_RELEASE(m_modelVertexBuffer);
	SAFE_RELEASE(m_modelIndexBuffer);
	SAFE_RELEASE(m_planeVertexBuffer);
	SAFE_RELEASE(m_planeIndexBuffer);
	SAFE_RELEASE(m_skyboxVertexBuffer);
	SAFE_RELEASE(m_skyboxIndexBuffer);
//	m_texture->Release();
	SAFE_RELEASE(m_textureView);
	SAFE_RELEASE(m_PlaneShaderView);
	SAFE_RELEASE(m_SkyBoxShaderView);
	//SAFE_RELEASE(m_shaderResourcePointerForNM);
	SAFE_RELEASE(m_NormalMapView);
	SAFE_RELEASE(m_shaderForRenderToScene);
	//SAFE_RELEASE(m_secondNormalMapView);
//	m_NullShaderView->Release();
	SAFE_RELEASE(m_secondTexture);
	SAFE_RELEASE(m_SecondPlaneShaderView);
	SAFE_RELEASE(m_sampleTexture);
	SAFE_RELEASE(m_lightBuffer);
	SAFE_RELEASE(m_PointLightBuffer);
	SAFE_RELEASE(m_SpotLightBuffer);
	//m_DirLightBufferForNormalMap->Release();
	SAFE_RELEASE(m_vertexBufferForGeometry);
	//ptrToBackBuffer
	SAFE_RELEASE(ptrToBackBuffer);
	delete m_model;
	delete m_skyModel;
	delete m_planeModel;
	ReportLiveObjects();
	SAFE_RELEASE(m_device);
	
	UnregisterClass(L"DirectXApplication", application);
	return true;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	D3D_DEMO::GetInstance()->Initialize(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && D3D_DEMO::GetInstance()->Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	D3D_DEMO::GetInstance()->ShutDown();
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case(WM_SIZE) : {  D3D_DEMO::GetInstance()->ResizingOfWindows(); }
					break;
	case (WM_DESTROY) : { PostQuitMessage(0); }
						break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}