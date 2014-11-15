//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include "XTime.h"

using namespace std;

#include <d3d11.h> 
#include <DirectXMath.h>
#pragma comment (lib, "d3d11.lib")
using namespace DirectX;
#include <directxcolors.h>

#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include "Trivial_GS.csh"
#include "Sky_PS.csh"
#include "withNormalMap_PS.csh"
#include "WithNormalMap_VS.csh"
#include "MultitexturePS.csh"
#include "MultitexturedVS.csh"
#include "NoInstance_VS.csh"
#include "NothingPS.csh"
#include "CubeVS.csh"
#include "cubeGS.csh"
#include "postprocessPS.csh"
#include "postPixel.csh"

#include "Sky_VS.csh"
#include "DDSTextureLoader.h"

//thread
#include <windows.h>
#include <conio.h>
#include <thread>

//object loader

#include <vector>
#include <fstream>
#include <istream>
#include <sstream>
using namespace std;

#define BACKBUFFER_WIDTH	1280.0f
#define BACKBUFFER_HEIGHT	768.0f
#define CIRCLESPEED (2 * 3.1415f) / 6

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;
#define WAIT_FOR_THREAD(r) if ((r)->joinable()) (r)->join();

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	ID3D11Device*					device;
	ID3D11RenderTargetView*			renderTargetView;
	ID3D11DeviceContext*			inmediateContext;
	IDXGISwapChain*					swapchain;
	D3D11_VIEWPORT*					viewport;
	D3D11_VIEWPORT*					differentViewport;

	ID3D11VertexShader*				vertexShader;
	ID3D11PixelShader*				pixelShader;
	ID3D11GeometryShader*			geometryshader;

	ID3D11VertexShader*				SkyvertexShader;
	ID3D11PixelShader*				SkypixelShader;
	ID3D11PixelShader*				objectNormalMappingPS;
	ID3D11VertexShader*				objectNormalMappingVS;
	ID3D11PixelShader*				multiTexturingPS;
	ID3D11VertexShader*				multiTexturingVS;
	ID3D11PixelShader*				noLPS;
	ID3D11VertexShader*				CubeVertexShader;
	ID3D11GeometryShader*			CubeGeometryShader;
	ID3D11VertexShader*				objectVS;
	ID3D11PixelShader*				postPS;
	ID3D11PixelShader*				postNightPS;
	
	ID3D11Texture2D*				zBuffer;
	ID3D11DepthStencilState *		stencilState;
	ID3D11DepthStencilView*			stencilView;
	ID3D11RasterizerState*			rasterState;
	ID3D11RasterizerState*			reverserasterState;
	ID3D11RasterizerState*			SkyrasterState;

	ID3D11SamplerState*				samplerState;
	ID3D11Buffer*					WorldconstantBuffer;
	ID3D11Buffer*					SceneconstantBuffer;
	ID3D11Buffer*					DirectionalLightconstantBuffer;
	ID3D11Buffer*					SpotLightconstantBuffer;
	ID3D11Buffer*					PointLightconstantBuffer;
	ID3D11Buffer*					cameraPositionBuffer;
	ID3D11Buffer*					instanceConstantBuffer;
	ID3D11Buffer*					SpecularConstantBuffer;
	ID3D11Buffer*					timeBuffer;

	ID3D11InputLayout*				vertexLayout;

	ID3D11Buffer*					SkyVertexbuffer;
	ID3D11Buffer*					SkyIndexbuffer;
	ID3D11Buffer*					GroundVertexbuffer;
	ID3D11Buffer*					GroundIndexbuffer;
	ID3D11Buffer*					ObjectVertexbuffer[11];
	ID3D11Buffer*					ObjectIndexbuffer[11];
	ID3D11ShaderResourceView*		shaderResourceView[27];
	ID3D11SamplerState*				CubesTexSamplerState;
	ID3D11DeviceContext*			deferredcontext[2];
	ID3D11CommandList*				commandList[2];

	ID3D11BlendState*				Blending;

	XTime							time;
	XMMATRIX						rotation;
	XMMATRIX						rotationRTT;
	XMMATRIX						Rotationx;
	XMMATRIX						Rotationy;
	XMMATRIX						Rotationz;
	float							dt;
	int								sphereIndex;
	int								groundIndex;
	bool							post;
	bool							post2;

	//RTT
	ID3D11Texture2D*				RTTTextureMap;
	ID3D11ShaderResourceView*		shaderResourceViewMap;
	ID3D11DepthStencilView*			RTTstencilView;
	ID3D11RenderTargetView*			RTTrenderTargetView;
	ID3D11Texture2D*				RTTzBuffer;
	D3D11_VIEWPORT*					RTTviewport;
	//cube
	ID3D11Buffer*					cubeVertexbuffer;
	//object loader

	int indexCount[9]; // one for each object type

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;

	float camYaw = 0.0f;
	float camPitch = 0.0f;
	float angle;

	struct SEND_TO_WORLD
	{
		XMMATRIX World;
	};

	struct SEND_TO_SCENE
	{
		XMMATRIX ViewM;
		XMMATRIX ProjectM;
	};

	struct SEND_DIRECTIONAL_LIGHT
	{
		XMFLOAT3 pos;
		float padding;
		XMFLOAT3 dir;
		float rPoint;
		XMFLOAT4 col;
	};

	struct SEND_POINT_LIGHT
	{
		XMFLOAT3 pos;
		float range;
		XMFLOAT3 dir;
		float rPoint;
		XMFLOAT4 col;
	};

	struct SEND_SPOT_LIGHT
	{
		XMFLOAT3 pos;
		float padding;
		XMFLOAT3 dir;
		float sPoint;
		XMFLOAT4 col;
		float inner;
		float outer;
		XMFLOAT2 morepadding;
	};

	struct SEND_TOFOG
	{
		XMFLOAT4 camPos;
	};

	struct SEND_TIME
	{
		XMFLOAT4 time;
	};


	struct SEND_TOINSTANCE
	{
		XMMATRIX world[100]; //tree position
	};

	struct SEND_TO_SPECULO
	{
		XMFLOAT4 camPos;
		float power;
		XMFLOAT3 padell;
		float intense;
		XMFLOAT3 padel;
	};

	SEND_TO_WORLD					WtoShader[13];
	SEND_TOINSTANCE					instanceToShader;
	SEND_TOINSTANCE					skeletontoshader;
	SEND_TOINSTANCE					tombstoshader;
	SEND_TOINSTANCE					handstoshader;
	SEND_TOINSTANCE					wasterTS;
	SEND_TOINSTANCE					wasterTS2;
	SEND_TOINSTANCE					wasterTS3;
	SEND_TOINSTANCE					lamentTS;
	SEND_TO_WORLD					skytoShader;
	SEND_TO_SCENE					StoShader[4];
	//lights
	SEND_DIRECTIONAL_LIGHT			directionalLight[2];
	SEND_POINT_LIGHT				PointLightToS[2];
	SEND_SPOT_LIGHT					SpotLightToS;
	SEND_TO_SPECULO					speculatToShader;
	//fog
	SEND_TOFOG						myView;
	SEND_TIME						myTime;

public:
	friend void LoadingThread(DEMO_APP* myApp); 
	friend void DrawingThread(DEMO_APP* myApp);
	friend void StatuesLoadingThread(DEMO_APP* myApp);
	friend void FolliageLoadingThread(DEMO_APP* myApp);
	friend void TowerLoadingThread(DEMO_APP* myApp);
	friend void ObjectLoadingThread(DEMO_APP* myApp);
	friend void BeastLoadingThread(DEMO_APP* myApp);
	friend void AssaulterLoadingThread(DEMO_APP* myApp);
	friend void GraveLoadingThread(DEMO_APP* myApp);
	friend void ZombieHandLoadingThread(DEMO_APP* myApp);
	friend void ZombieLoadingThread(DEMO_APP* myApp);
	friend void LamentLoadingThread(DEMO_APP* myApp);

	friend void RRTDraw(DEMO_APP* myApp);
	friend void MinimapDraw(DEMO_APP* myApp);

	struct SimpleVertex
	{
		XMFLOAT3 UV;
		XMFLOAT3 norm;
		XMFLOAT3 Pos;
		XMFLOAT3 tangent;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	bool LoadObjectFromFile(string fileName, int slot);
	void CreateSphere(int LatLines, int LongLines);
	void Input();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);

	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"Nightmare", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

	angle = 0.0f;
	post = false;
	post2 = false;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = BACKBUFFER_WIDTH;
	sd.BufferDesc.Height = BACKBUFFER_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = window;
	//sd.Windowed = false;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG,
		nullptr, 0, D3D11_SDK_VERSION, &sd, &swapchain, &device, nullptr, &inmediateContext);
	ID3D11Texture2D* backBuffer = nullptr;
	hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));

	hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
	backBuffer->Release();

	std::thread myLoadingThread = std::thread(LoadingThread, this); //textures
	std::thread(StatuesLoadingThread, this).detach();
	std::thread(FolliageLoadingThread, this).detach();
	std::thread(TowerLoadingThread, this).detach();
	std::thread(ObjectLoadingThread, this).detach();
	std::thread(BeastLoadingThread, this).detach();
	std::thread(AssaulterLoadingThread, this).detach();
	std::thread(ZombieHandLoadingThread, this).detach();
	std::thread(GraveLoadingThread, this).detach();
	std::thread(ZombieLoadingThread, this).detach();
	std::thread(LamentLoadingThread, this).detach();

	viewport = new D3D11_VIEWPORT;
	viewport->Width = (FLOAT)sd.BufferDesc.Width;
	viewport->Height = (FLOAT)sd.BufferDesc.Height;
	viewport->MinDepth = 0.0f;
	viewport->MaxDepth = 1.0f;
	viewport->TopLeftX = 0;
	viewport->TopLeftY = 0;

	differentViewport = new D3D11_VIEWPORT;
	differentViewport->Width = viewport->Width *0.25f;
	differentViewport->Height = viewport->Height *0.25f;
	differentViewport->MinDepth = 0.0f;
	differentViewport->MaxDepth = 1.0f;
	differentViewport->TopLeftX = BACKBUFFER_WIDTH * 0.5f + BACKBUFFER_WIDTH  *0.25f;
	differentViewport->TopLeftY = 0;

	RTTviewport = new D3D11_VIEWPORT;
	RTTviewport->Width = (FLOAT)1200;	
	RTTviewport->Height = (FLOAT)1200;
	RTTviewport->MinDepth = 0.0f;
	RTTviewport->MaxDepth = 1.0f;
	RTTviewport->TopLeftX = 0;
	RTTviewport->TopLeftY = 0;

	hr = device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), nullptr, &vertexShader);
	hr = device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), nullptr, &pixelShader);
	hr = device->CreateGeometryShader(Trivial_GS, sizeof(Trivial_GS), nullptr, &geometryshader);

	hr = device->CreateVertexShader(Sky_VS, sizeof(Sky_VS), nullptr, &SkyvertexShader);
	hr = device->CreatePixelShader(Sky_PS, sizeof(Sky_PS), nullptr, &SkypixelShader);
	hr = device->CreatePixelShader(WithNormalMap_PS, sizeof(WithNormalMap_PS), nullptr, &objectNormalMappingPS);
	hr = device->CreateVertexShader(WithNormalMap_VS, sizeof(WithNormalMap_VS), nullptr, &objectNormalMappingVS);
	hr = device->CreatePixelShader(MultitexturePS, sizeof(MultitexturePS), nullptr, &multiTexturingPS);
	hr = device->CreateVertexShader(MultitexturedVS, sizeof(MultitexturedVS), nullptr, &multiTexturingVS);
	hr = device->CreatePixelShader(NothingPS, sizeof(NothingPS), nullptr, &noLPS);
	hr = device->CreateVertexShader(CubeVS, sizeof(CubeVS), nullptr, &CubeVertexShader);
	hr = device->CreateGeometryShader(cubeGS, sizeof(cubeGS), nullptr, &CubeGeometryShader);
	hr = device->CreateVertexShader(NoInstance_VS, sizeof(NoInstance_VS), nullptr, &objectVS);
	hr = device->CreatePixelShader(postprocessPS, sizeof(postprocessPS), nullptr, &postPS);
	hr = device->CreatePixelShader(postPixel, sizeof(postPixel), nullptr, &postNightPS);
	
	// Z BUFFER
	D3D11_TEXTURE2D_DESC dbDesc;

	ZeroMemory(&dbDesc, sizeof(dbDesc));

	dbDesc.Width = BACKBUFFER_WIDTH;
	dbDesc.Height = BACKBUFFER_HEIGHT;
	dbDesc.MipLevels = 1;
	dbDesc.ArraySize = 1;
	dbDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dbDesc.SampleDesc.Count = 1;
	dbDesc.SampleDesc.Quality = 0;
	dbDesc.Usage = D3D11_USAGE_DEFAULT;
	dbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dbDesc.CPUAccessFlags = 0;
	dbDesc.MiscFlags = 0;

	hr = device->CreateTexture2D(&dbDesc, NULL, &zBuffer);

	ZeroMemory(&dbDesc, sizeof(dbDesc));

	dbDesc.Width = 1200;
	dbDesc.Height = 1200;
	dbDesc.MipLevels = 1;
	dbDesc.ArraySize = 1;
	dbDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dbDesc.SampleDesc.Count = 1;
	dbDesc.SampleDesc.Quality = 0;
	dbDesc.Usage = D3D11_USAGE_DEFAULT;
	dbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dbDesc.CPUAccessFlags = 0;
	dbDesc.MiscFlags = 0;

	hr = device->CreateTexture2D(&dbDesc, NULL, &RTTzBuffer);

	//RTT z buffer
	ZeroMemory(&dbDesc, sizeof(dbDesc));

	dbDesc.Width = 1200;
	dbDesc.Height = 1200;
	dbDesc.MipLevels = 1;
	dbDesc.ArraySize = 1;
	dbDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	dbDesc.SampleDesc.Count = 1;
	dbDesc.SampleDesc.Quality = 0;
	dbDesc.Usage = D3D11_USAGE_DEFAULT;
	dbDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	dbDesc.CPUAccessFlags = 0;
	dbDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = device->CreateTexture2D(&dbDesc, NULL, &RTTTextureMap);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	hr = device->CreateRenderTargetView(RTTTextureMap, &renderTargetViewDesc, &RTTrenderTargetView);

	hr = device->CreateDepthStencilView(RTTzBuffer, NULL, &RTTstencilView);

	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	srDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(RTTTextureMap, &srDesc, &shaderResourceViewMap);

	//STELCIL
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	// Depth test parameters
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	// Stencil test parameters
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	// Stencil operations if pixel is front-facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateDepthStencilState(&depthStencilDesc, &stencilState);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(zBuffer, &depthStencilViewDesc, &stencilView);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//Create the Sample State
	hr = device->CreateSamplerState(&sampDesc, &CubesTexSamplerState);

	//Ground
	std::vector<SimpleVertex> myground;

	XMFLOAT3 initial = XMFLOAT3(-150.0f, -2.0f, 150.0f);

	for (int rows = 0; rows < 301; rows++)
	{
		for (int columns = 0; columns < 301; columns++)
		{
			SimpleVertex aux;
			aux.Pos = XMFLOAT3(initial.x, initial.y, initial.z);
			aux.norm = XMFLOAT3(0.0f, 1.0f, 0.0f);
			aux.UV = XMFLOAT3(columns*0.03333f, rows*0.03333f, 0.0f);

			myground.push_back(aux);

			initial.x++;
		}
		initial.x = -150.0f;
		initial.z--;
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = NULL;
	bd.ByteWidth = sizeof(SimpleVertex) * myground.size();
	bd.MiscFlags = 0; //unused
	bd.StructureByteStride = sizeof(SimpleVertex);

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &myground[0];

	hr = device->CreateBuffer(&bd, &InitData, &GroundVertexbuffer);

	std::vector<int> myGroundIndex;

	int vIndex = 0;
	for (int z = 0; z < 300; z++)
	{
		for (int x = 0; x < 300; x++)
		{
			// first triangle
			myGroundIndex.push_back(vIndex);
			myGroundIndex.push_back(vIndex + 301 + 1);
			myGroundIndex.push_back(vIndex + 301);

			// second triangle
			myGroundIndex.push_back(vIndex);
			myGroundIndex.push_back(vIndex + 1);
			myGroundIndex.push_back(vIndex + 301 + 1);

			vIndex++;
		}
		vIndex++;
	}
	D3D11_BUFFER_DESC Ibd;
	ZeroMemory(&Ibd, sizeof(Ibd));
	Ibd.Usage = D3D11_USAGE_DEFAULT;
	Ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Ibd.CPUAccessFlags = NULL;
	Ibd.ByteWidth = sizeof(int) * myGroundIndex.size();
	Ibd.MiscFlags = 0; //unused
	Ibd.StructureByteStride = sizeof(unsigned short);

	D3D11_SUBRESOURCE_DATA indexInitData;
	ZeroMemory(&indexInitData, sizeof(indexInitData));
	indexInitData.pSysMem = &myGroundIndex[0];
	indexInitData.SysMemPitch = 0;
	indexInitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&Ibd, &indexInitData, &GroundIndexbuffer);

	groundIndex = myGroundIndex.size();

	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_TO_WORLD);

	hr = device->CreateBuffer(&cbd, nullptr, &WorldconstantBuffer);

	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_TO_SCENE);

	hr = device->CreateBuffer(&cbd, nullptr, &SceneconstantBuffer);

	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_DIRECTIONAL_LIGHT);

	hr = device->CreateBuffer(&cbd, nullptr, &DirectionalLightconstantBuffer);

	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_SPOT_LIGHT);

	hr = device->CreateBuffer(&cbd, nullptr, &SpotLightconstantBuffer);

	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_POINT_LIGHT);

	hr = device->CreateBuffer(&cbd, nullptr, &PointLightconstantBuffer);

	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_TOFOG);

	hr = device->CreateBuffer(&cbd, nullptr, &cameraPositionBuffer);

	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_TOINSTANCE);

	hr = device->CreateBuffer(&cbd, nullptr, &instanceConstantBuffer);

	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_TO_SPECULO);

	hr = device->CreateBuffer(&cbd, nullptr, &SpecularConstantBuffer);


	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_TIME);

	hr = device->CreateBuffer(&cbd, nullptr, &timeBuffer);
	

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = device->CreateInputLayout(layout, numElements, Sky_VS, sizeof(Sky_VS), &vertexLayout);

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	//a
	hr = device->CreateRasterizerState(&rasterDesc, &rasterState);

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	//a
	hr = device->CreateRasterizerState(&rasterDesc, &reverserasterState);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	/*rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;*/

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	device->CreateBlendState(&blendDesc, &Blending);

	rasterDesc.CullMode = D3D11_CULL_NONE;
	hr = device->CreateRasterizerState(&rasterDesc, &SkyrasterState);


	D3D11_SAMPLER_DESC sampd;
	ZeroMemory(&sampd, sizeof(sampd));
	sampd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	sampd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampd.MaxLOD = D3D11_FLOAT32_MAX;
	sampd.MinLOD = 0;
	//CUBE
	SimpleVertex myCubePoint[1];

	myCubePoint[0].Pos = XMFLOAT3(125.0f, 5.0f, 125.0f);

	//To VRAM
	D3D11_BUFFER_DESC cubd;
	ZeroMemory(&cubd, sizeof(cubd));
	cubd.Usage = D3D11_USAGE_IMMUTABLE;
	cubd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	cubd.CPUAccessFlags = NULL;
	cubd.ByteWidth = sizeof(SimpleVertex);
	cubd.MiscFlags = 0; //unused
	cubd.StructureByteStride = sizeof(SimpleVertex);

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &myCubePoint;

	hr = device->CreateBuffer(&cubd, &InitData, &cubeVertexbuffer);

	hr = device->CreateSamplerState(&sampd, &samplerState);

	for (int i = 0; i < 2; i++)
		hr = device->CreateDeferredContext(0, &deferredcontext[i]);

	StoShader[0].ViewM = XMMatrixIdentity();
	StoShader[0].ProjectM = XMMatrixPerspectiveFovLH(1.30899694f, BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT, 0.1f, 300.0f);
	StoShader[1].ViewM = XMMatrixIdentity(); //viewforskybox
	StoShader[1].ProjectM = XMMatrixPerspectiveFovLH(1.30899694f, BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT, 0.1f, 300.0f);
	//new viewport
	StoShader[2].ViewM = XMMatrixIdentity(); 
	StoShader[2].ProjectM = XMMatrixOrthographicLH((BACKBUFFER_WIDTH*0.15f), (BACKBUFFER_HEIGHT*0.2f), 0.1f, 300.0f);
	//RTT camera

	StoShader[3].ViewM = XMMatrixIdentity();
	StoShader[3].ProjectM = XMMatrixPerspectiveFovLH(0.9f, 1.0f, 0.1f, 300.0f);


	XMFLOAT4 vec = XMFLOAT4(0.0f, 10.0f, -15.0f, 0.0f);
	camPosition = XMLoadFloat4(&vec);
	XMFLOAT4 vec2 = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	camTarget = XMLoadFloat4(&vec2);
	XMFLOAT4 vec3 = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	camUp = XMLoadFloat4(&vec3);

	StoShader[0].ViewM *= XMMatrixLookAtLH(camPosition, camTarget, camUp);

	vec = XMFLOAT4(0.0f, 5.0f, -25.0f, 0.0f);
	camPosition = XMLoadFloat4(&vec);
	vec2 = XMFLOAT4(0.0f, 0.0f, 10.0f, 0.0f);
	camTarget = XMLoadFloat4(&vec2);
	vec3 = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	camUp = XMLoadFloat4(&vec3);

	StoShader[3].ViewM *= XMMatrixLookAtLH(camPosition, camTarget, camUp);

	vec = XMFLOAT4(0.0f, 170.0f, -5.0f, 0.0f);
	camPosition = XMLoadFloat4(&vec);
	vec2 = XMFLOAT4(0.0f, 5.0f, 0.0f, 0.0f);
	camTarget = XMLoadFloat4(&vec2);
	vec3 = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	camUp = XMLoadFloat4(&vec3);

	StoShader[2].ViewM *= XMMatrixLookAtLH(camPosition, camTarget, camUp);

	CreateSphere(20, 20);

	skytoShader.World = XMMatrixIdentity();
	skytoShader.World *= XMMatrixScaling(100.0f, 100.0f, 100.0f);
	for (int i = 0; i < 13; i++)
		WtoShader[i].World = XMMatrixIdentity();

	//WtoShader[1].World = XMMatrixTranslation(15.0f, 0.0f, 15.0f);
	WtoShader[0].World = XMMatrixScaling(0.5f, 0.5f, 0.5f);

	//WtoShader[1].World *= XMMatrixScaling(0.5f, 0.5f, 0.5f);
	WtoShader[2].World *= XMMatrixScaling(0.05f, 0.05f, 0.05f);
	WtoShader[3].World *= XMMatrixScaling(0.5f, 0.5f, 0.5f);

	WtoShader[7].World *= XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixTranslation(125.0f, 8.0f, 125.0f);

	XMFLOAT3 wet = XMFLOAT3 (0.0f, 1.0f, 0.0f);
	XMVECTOR myrote = XMLoadFloat3(&wet);
	//clown
	WtoShader[8].World *= XMMatrixScaling(0.05f, 0.05f, 0.05f) * XMMatrixRotationAxis(myrote, 1.570796325f) * XMMatrixTranslation(12.0f, 58.6f, 19.0f);
	//beast
	WtoShader[9].World *= XMMatrixScaling(0.005f, 0.005f, 0.005f) * XMMatrixRotationAxis(myrote, 3.1415f) * XMMatrixTranslation(-11.5f, 57.7f, 19.0f);

	//Assaulter

	wet = XMFLOAT3(1.0f, 0.0f, 0.0f);
	myrote = XMLoadFloat3(&wet);
	WtoShader[10].World *= XMMatrixScaling(0.02f, 0.02f, 0.02f) * XMMatrixRotationAxis(myrote, -0.5f) * XMMatrixTranslation(-2.0f, 30.0f, 8.5f);
	//tower
	WtoShader[4].World *= XMMatrixScaling(0.15f, 0.15f, 0.15f);

	directionalLight[0].pos = XMFLOAT3(-150.0f, 150.0f, -150.0f);
	directionalLight[0].dir = XMFLOAT3(2.0f, -1.0f, 2.0f);
	directionalLight[0].col = XMFLOAT4(0.125f, 0.15f, 0.5f, 0.5f); //(65,105,225)

	directionalLight[1].pos = XMFLOAT3(0.0f, 50.0f,0.0f);
	directionalLight[1].dir = XMFLOAT3(0.0f, -0.5f, -1.0f);
	directionalLight[1].col = XMFLOAT4(1.0f, 0.0f, 1.0f, 0.8f);

	PointLightToS[0].pos = XMFLOAT3(-5.0f, 5.0f, -5.0f);
	PointLightToS[0].range = 55.0f;
	PointLightToS[0].dir = XMFLOAT3(2.0f, -1.0f, -3.0f);
	PointLightToS[0].col = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.8f);
	PointLightToS[0].rPoint = 3.5f;

	PointLightToS[1].pos = XMFLOAT3(0.0f, 2.0f, 0.0f);
	PointLightToS[1].dir = XMFLOAT3(0.0f, -1.0f, 0.0f);
	PointLightToS[1].col = XMFLOAT4(1.0f, 0.77f, 0.0f, 1.0f);
	PointLightToS[1].rPoint = 5.0f;

	SpotLightToS.pos = XMFLOAT3(0.0f, 10.0f, 0.0f);
	SpotLightToS.dir = XMFLOAT3(0.0f, -1.0f, 0.0f);
	SpotLightToS.col = XMFLOAT4(0.0f, 1.0f, 1.0, 1.0f);
	SpotLightToS.sPoint = 100.0f;
	SpotLightToS.inner = 0.72f;
	SpotLightToS.outer = 0.62f;
	SpotLightToS.padding = 1.0f;

	//specular
	speculatToShader.intense = 3.0f;
	speculatToShader.power = 25.0f;

	rotationRTT = XMMatrixIdentity();

	for (int i = 0; i < 100; i++)
	{
		float x, z;
		do
		{
			x = -100 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 200));
			z = -100 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 200));
		} while ((x >= -5.0f && x <= 5.0f) || (z >= -5.0f && z <= 5.0f));

		XMMATRIX move = XMMatrixIdentity();
		move = XMMatrixTranslation(x, 0.0f, z);

		XMMATRIX scaling = XMMatrixIdentity();
		scaling *= XMMatrixScaling(0.06f, 0.06f, 0.06f);

		instanceToShader.world[i] = XMMatrixIdentity();
		instanceToShader.world[i] *= scaling * move;
	}

	for (int i = 0; i < 100; i++)
	{
		float x, z;
		do
		{
			x = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
			z = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
		} while ((x > -60.0f && x < 60.0f) || (z > -60.0f && z < 60.0f));

		XMMATRIX move = XMMatrixIdentity();
		move = XMMatrixTranslation(x, 0.5f, z);

		XMMATRIX scaling = XMMatrixIdentity();
		scaling *= XMMatrixScaling(0.1f, 0.1f, 0.1f);

		tombstoshader.world[i] = XMMatrixIdentity();
		tombstoshader.world[i] *= scaling * move;
	}

	for (int i = 0; i < 100; i++)
	{

		XMMATRIX move = XMMatrixIdentity();
		move = tombstoshader.world[i];

		XMMATRIX scaling = XMMatrixIdentity();
		scaling *= XMMatrixScaling(0.3f, 0.3f, 0.3f);

		handstoshader.world[i] = XMMatrixIdentity();
		handstoshader.world[i] *= scaling * move;
	}

	for (int i = 0; i < 100; i++)
	{
		float x, z, y;
		x = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
		z = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
		y = -2 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 7));
		XMMATRIX move = XMMatrixIdentity();
		move = XMMatrixTranslation(x, y, z);

		XMMATRIX scaling = XMMatrixIdentity();
		scaling *= XMMatrixScaling(2.0f, 2.0f, 2.0f);

		wasterTS.world[i] = XMMatrixIdentity();
		wasterTS.world[i] *= scaling * move;
	}

	for (int i = 0; i < 100; i++)
	{
		float x, z, y;
		x = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
		z = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
		y = -2 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 7));
		XMMATRIX move = XMMatrixIdentity();
		move = XMMatrixTranslation(x, y, z);

		XMMATRIX scaling = XMMatrixIdentity();
		scaling *= XMMatrixScaling(2.0f, 2.0f, 2.0f);

		wasterTS2.world[i] = XMMatrixIdentity();
		wasterTS2.world[i] *= scaling * move;
	}

	for (int i = 0; i < 100; i++)
	{
		float x, z, y;
		x = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
		z = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
		y = -2 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 7));
		XMMATRIX move = XMMatrixIdentity();
		move = XMMatrixTranslation(x, y, z);

		XMMATRIX scaling = XMMatrixIdentity();
		scaling *= XMMatrixScaling(2.0f, 2.0f, 2.0f);

		wasterTS3.world[i] = XMMatrixIdentity();
		wasterTS3.world[i] *= scaling * move;
	}


	for (int i = 0; i < 100; i++)
	{
		float x, z;
		x = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));
		z = -120 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 260));

		XMMATRIX move = XMMatrixIdentity();
		move = XMMatrixTranslation(x, 0.5f, z);

		XMMATRIX scaling = XMMatrixIdentity();
		scaling *= XMMatrixScaling(2.0f, 2.0f, 2.0f);

		lamentTS.world[i] = XMMatrixIdentity();
		lamentTS.world[i] *= scaling * move;
	}


	WAIT_FOR_THREAD(&myLoadingThread);
	
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	time.Signal();
	dt = time.Delta();
	double mytime = time.TotalTime();
	Input();
	XMMATRIX viewAux = XMMatrixInverse(nullptr, StoShader[0].ViewM);
	//viewAux *= WtoShader[0].World;

	XMStoreFloat4(&myView.camPos, viewAux.r[3]);

	//point light
	PointLightToS[0].rPoint *= cos((PointLightToS[0].rPoint + mytime) * 5) * sin((PointLightToS[0].rPoint - mytime) * 5) * dt;
	// glow
	//PointLightToS.rPoint = 10.0f + 5.0f * cos(mytime) * sin(mytime);


	//spot light
	angle += dt * CIRCLESPEED;
	XMFLOAT3 position = XMFLOAT3(sinf(angle) * 30, 0.0f, cosf(angle) * 30);


	SpotLightToS.pos = XMFLOAT3(position.x, SpotLightToS.pos.y, position.z);
	SpotLightToS.col = XMFLOAT4(/*SpotLightToS.col.x -*/ cos(mytime), /*SpotLightToS.col.y -*/ sin(mytime), /*SpotLightToS.col.z - */cos(dt), 1.0f);
	XMFLOAT3 mydir = XMFLOAT3(position.x - SpotLightToS.pos.x, SpotLightToS.dir.y, position.z - SpotLightToS.pos.z);
	XMVECTOR myhel = XMLoadFloat3(&mydir);
	XMVector2Normalize(myhel);
	XMStoreFloat3(&mydir, myhel);

	SpotLightToS.dir = mydir;

	if (SpotLightToS.col.x < 0.0f)
		SpotLightToS.col.x = 0.0f;
	if (SpotLightToS.col.y < 0.0f)
		SpotLightToS.col.y = 0.0f;
	if (SpotLightToS.col.z < 0.0f)
		SpotLightToS.col.z = 0.0f;

	if (SpotLightToS.col.x > 1.0f)
		SpotLightToS.col.x = 1.0f;
	if (SpotLightToS.col.y > 1.0f)
		SpotLightToS.col.y = 1.0f;
	if (SpotLightToS.col.z > 1.0f)
		SpotLightToS.col.z = 1.0f;

	SpotLightToS.padding = 1.0f;

	 XMMATRIX CamviewAux = XMMatrixInverse(nullptr, StoShader[3].ViewM);
	//viewAux *= WtoShader[0].World;

	 XMStoreFloat4(&speculatToShader.camPos, CamviewAux.r[3]);

	inmediateContext->OMSetRenderTargets(1, &renderTargetView, stencilView);
	inmediateContext->ClearDepthStencilView(stencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	inmediateContext->ClearRenderTargetView(renderTargetView, Colors::MidnightBlue);
	inmediateContext->VSSetConstantBuffers(0, 1, &WorldconstantBuffer);
	inmediateContext->VSSetConstantBuffers(1, 1, &SceneconstantBuffer);

	//minimap
	std::thread myMinimap = std::thread(MinimapDraw, this);
	//Render To Texture
	std::thread rttThread = std::thread(RRTDraw, this);

	WAIT_FOR_THREAD(&rttThread);
	if (commandList[1])
		inmediateContext->ExecuteCommandList(commandList[1], true);
	SAFE_RELEASE(commandList[1]);

	SpotLightToS.padding = 1.0f;
	//skybox
	inmediateContext->OMSetRenderTargets(1, &renderTargetView, stencilView);
	inmediateContext->OMSetDepthStencilState(stencilState, 0);
	inmediateContext->ClearDepthStencilView(stencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	inmediateContext->RSSetViewports(1, viewport);

	inmediateContext->ClearRenderTargetView(renderTargetView, Colors::MidnightBlue);
	inmediateContext->VSSetConstantBuffers(0, 1, &WorldconstantBuffer);
	inmediateContext->VSSetConstantBuffers(1, 1, &SceneconstantBuffer);
	inmediateContext->PSSetSamplers(0, 1, &samplerState);

	inmediateContext->RSSetState(SkyrasterState);

	skytoShader.World = XMMatrixIdentity();
	XMMATRIX Scale = XMMatrixScaling(100.0f, 100.0f, 100.0f);
	XMMATRIX Translation = XMMatrixTranslation(XMVectorGetX(StoShader[0].ViewM.r[0]), XMVectorGetY(StoShader[0].ViewM.r[1]), XMVectorGetZ(StoShader[0].ViewM.r[2]));
	skytoShader.World = Scale * Translation;

	//infinite skybox
	XMMATRIX auxe = StoShader[0].ViewM;
	auxe.r[3].m128_f32[0] = auxe.r[3].m128_f32[1] = auxe.r[3].m128_f32[2] = 0.0f;
	StoShader[1].ViewM = auxe;

	inmediateContext->PSSetConstantBuffers(0, 1, &cameraPositionBuffer);

	D3D11_MAPPED_SUBRESOURCE  Resource;

	inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &StoShader[1], sizeof(SEND_TO_SCENE));
	inmediateContext->Unmap(SceneconstantBuffer, 0);

	inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &skytoShader, sizeof(SEND_TO_WORLD));
	inmediateContext->Unmap(WorldconstantBuffer, 0);

	inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
	inmediateContext->Unmap(cameraPositionBuffer, 0);

	inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
	inmediateContext->Unmap(SpotLightconstantBuffer, 0);

	inmediateContext->PSSetShader(SkypixelShader, nullptr, 0);
	inmediateContext->VSSetShader(SkyvertexShader, nullptr, 0);

	UINT sstride = sizeof(SimpleVertex);
	UINT soffset = 0;

	inmediateContext->IASetInputLayout(vertexLayout);

	inmediateContext->IASetVertexBuffers(0, 1, &SkyVertexbuffer, &sstride, &soffset);
	inmediateContext->IASetIndexBuffer(SkyIndexbuffer, DXGI_FORMAT_R16_UINT, 0);

	inmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	inmediateContext->PSSetSamplers(0, 1, &CubesTexSamplerState);
	inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[0]);

	inmediateContext->DrawIndexed(sphereIndex, 0, 0);

	//magic box
	if (shaderResourceViewMap)
	{
		inmediateContext->GSSetShader(CubeGeometryShader, nullptr, 0);
		inmediateContext->PSSetShader(noLPS, nullptr, 0);
		inmediateContext->VSSetShader(CubeVertexShader, nullptr, 0);

		inmediateContext->RSSetViewports(1, viewport);
		inmediateContext->OMSetDepthStencilState(stencilState, 1);

		inmediateContext->GSSetConstantBuffers(0, 1, &SceneconstantBuffer);

		inmediateContext->PSSetSamplers(0, 1, &samplerState);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceViewMap);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[7], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_TOFOG));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;

		inmediateContext->RSSetState(rasterState);

		inmediateContext->IASetInputLayout(vertexLayout);
		inmediateContext->IASetVertexBuffers(0, 1, &cubeVertexbuffer, &stride, &offset);
		//inmediateContext->IASetIndexBuffer(cubeIndexbuffer, DXGI_FORMAT_R16_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		inmediateContext->Draw(1, 0);

		myTime.time.x = time.TotalTime();

		if (post == true)
		{
			inmediateContext->ClearDepthStencilView(stencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			inmediateContext->PSSetShader(postNightPS, nullptr, 0);
			inmediateContext->PSSetConstantBuffers(0, 1, &timeBuffer);

			inmediateContext->Map(timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
			memcpy(Resource.pData, &myTime, sizeof(DEMO_APP::SEND_TIME));
			inmediateContext->Unmap(timeBuffer, 0);

			inmediateContext->PSSetSamplers(0, 1, &samplerState);
			inmediateContext->PSSetShaderResources(0, 1, &shaderResourceViewMap);
			inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[14]);

			inmediateContext->Draw(1, 0);
		}
	}
	//trees
	if (ObjectIndexbuffer[1] && ObjectVertexbuffer[1])
	{
		inmediateContext->VSSetConstantBuffers(2, 1, &instanceConstantBuffer);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->GSSetShader(nullptr, nullptr, 0);
		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[2], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &instanceToShader, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectNormalMappingVS, nullptr, 0);

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[1], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[1], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[3]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[5]);

		inmediateContext->DrawIndexedInstanced(indexCount[1], 100, 0, 0, 0);
	}
	//leaves
	if (ObjectIndexbuffer[0] && ObjectVertexbuffer[0])
	{
		inmediateContext->VSSetConstantBuffers(2, 1, &instanceConstantBuffer);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->RSSetState(SkyrasterState);
		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[3], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &instanceToShader, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &instanceToShader, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectNormalMappingVS, nullptr, 0);
		sstride = sizeof(SimpleVertex);
		soffset = 0;

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[0], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[0], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[4]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[6]);

		inmediateContext->DrawIndexedInstanced(indexCount[0], 70, 0, 0, 0);
	}
	//grave and zombie hands
	if (ObjectIndexbuffer[7] && ObjectVertexbuffer[7] && ObjectIndexbuffer[8] && ObjectVertexbuffer[8])
	{
		inmediateContext->VSSetConstantBuffers(2, 1, &instanceConstantBuffer);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->GSSetShader(nullptr, nullptr, 0);
		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[2], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &tombstoshader, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectNormalMappingVS, nullptr, 0);

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[7], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[7], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[19]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[20]);

		inmediateContext->DrawIndexedInstanced(indexCount[7], 100, 0, 0, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &handstoshader, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[8], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[8], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[21]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[22]);

		inmediateContext->DrawIndexedInstanced(indexCount[8], 70, 0, 0, 0);
	}
	//ground draw
	inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
	inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
	inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
	inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);
	inmediateContext->RSSetState(rasterState);

	inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
	inmediateContext->Unmap(SceneconstantBuffer, 0);

	inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &WtoShader[1], sizeof(SEND_TO_WORLD));
	inmediateContext->Unmap(WorldconstantBuffer, 0);

	inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
	inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

	inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
	inmediateContext->Unmap(PointLightconstantBuffer, 0);

	inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
	inmediateContext->Unmap(SpotLightconstantBuffer, 0);

	inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
	inmediateContext->Unmap(cameraPositionBuffer, 0);

	inmediateContext->GSSetShader(geometryshader, nullptr, 0);
	inmediateContext->PSSetShader(pixelShader, nullptr, 0);
	inmediateContext->VSSetShader(vertexShader, nullptr, 0);
	sstride = sizeof(SimpleVertex);
	soffset = 0;

	inmediateContext->IASetInputLayout(vertexLayout);

	inmediateContext->IASetVertexBuffers(0, 1, &GroundVertexbuffer, &sstride, &soffset);
	inmediateContext->IASetIndexBuffer(GroundIndexbuffer, DXGI_FORMAT_R32_UINT, 0);

	inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[1]);
	inmediateContext->VSSetShaderResources(1, 1, &shaderResourceView[2]);
	inmediateContext->VSSetSamplers(0, 1, &CubesTexSamplerState);
	inmediateContext->VSSetShaderResources(0, 1, &shaderResourceView[2]);

	inmediateContext->DrawIndexed(groundIndex, 0, 0);

	//stuff
	//clown
	if (ObjectIndexbuffer[3] && ObjectVertexbuffer[3])
	{
		inmediateContext->RSSetState(rasterState);
		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->GSSetShader(nullptr, nullptr, 0);
		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectVS, nullptr, 0);

		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[8], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &instanceToShader, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		sstride = sizeof(SimpleVertex);
		soffset = 0;

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[3], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[3], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[10]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[11]);

		inmediateContext->DrawIndexed(indexCount[3], 0, 0);
	}
//beast
	if (ObjectIndexbuffer[5] && ObjectVertexbuffer[5])
	{

		inmediateContext->RSSetState(reverserasterState);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[15]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[16]);

		inmediateContext->VSSetConstantBuffers(0, 1, &WorldconstantBuffer);
		inmediateContext->VSSetConstantBuffers(1, 1, &SceneconstantBuffer);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->GSSetShader(nullptr, nullptr, 0);
		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectVS, nullptr, 0);

		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[9], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &instanceToShader, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[5], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[5], DXGI_FORMAT_R32_UINT, 0);
		inmediateContext->IASetInputLayout(vertexLayout);
		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		inmediateContext->DrawIndexed(indexCount[5], 0, 0);
	}
	//Assaulter
	if (ObjectIndexbuffer[6] && ObjectVertexbuffer[6])
	{
		inmediateContext->RSSetState(rasterState);

		inmediateContext->VSSetConstantBuffers(0, 1, &WorldconstantBuffer);
		inmediateContext->VSSetConstantBuffers(1, 1, &SceneconstantBuffer);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->GSSetShader(nullptr, nullptr, 0);
		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectVS, nullptr, 0);

		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[10], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &instanceToShader, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		sstride = sizeof(SimpleVertex);
		soffset = 0;

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[6], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[6], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[17]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[18]);

		inmediateContext->DrawIndexed(indexCount[6], 0, 0);
	}
	//Tower
	if (ObjectIndexbuffer[4] && ObjectVertexbuffer[4])
	{
		inmediateContext->RSSetState(rasterState);
		inmediateContext->VSSetConstantBuffers(0, 1, &WorldconstantBuffer);
		inmediateContext->VSSetConstantBuffers(1, 1, &SceneconstantBuffer);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[3], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[3], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[10]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[11]);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->GSSetShader(nullptr, nullptr, 0);
		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectVS, nullptr, 0);

		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[4], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &instanceToShader, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		sstride = sizeof(SimpleVertex);
		soffset = 0;

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[4], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[4], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[12]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[13]);

		inmediateContext->DrawIndexed(indexCount[4], 0, 0);
	}
	//waster
	if (ObjectIndexbuffer[9] && ObjectVertexbuffer[9])
	{
		inmediateContext->VSSetConstantBuffers(2, 1, &instanceConstantBuffer);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->GSSetShader(nullptr, nullptr, 0);
		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[2], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &wasterTS, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectNormalMappingVS, nullptr, 0);

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[9], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[9], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[23]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[24]);

		inmediateContext->DrawIndexedInstanced(indexCount[9], 100, 0, 0, 0);

		//wave2
		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &wasterTS2, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->DrawIndexedInstanced(indexCount[9], 100, 0, 0, 0);

		//wave3
		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &wasterTS3, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->DrawIndexedInstanced(indexCount[9], 100, 0, 0, 0);
	}

	//lament
	if (ObjectIndexbuffer[10] && ObjectVertexbuffer[10])
	{
		inmediateContext->VSSetConstantBuffers(2, 1, &instanceConstantBuffer);

		inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
		inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);

		inmediateContext->GSSetShader(nullptr, nullptr, 0);
		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[2], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
		inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

		inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
		inmediateContext->Unmap(PointLightconstantBuffer, 0);

		inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(SpotLightconstantBuffer, 0);

		inmediateContext->Map(instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &lamentTS, sizeof(SEND_TOINSTANCE));
		inmediateContext->Unmap(instanceConstantBuffer, 0);

		inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
		inmediateContext->Unmap(cameraPositionBuffer, 0);

		inmediateContext->PSSetShader(objectNormalMappingPS, nullptr, 0);
		inmediateContext->VSSetShader(objectNormalMappingVS, nullptr, 0);

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[10], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[10], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[25]);
		inmediateContext->PSSetShaderResources(1, 1, &shaderResourceView[26]);

		inmediateContext->DrawIndexedInstanced(indexCount[10], 100, 0, 0, 0);
	}
	//water
	//float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };
	//inmediateContext->OMSetBlendState(Blending, blendFactor, 0xffffffff);
	//		
	//inmediateContext->PSSetConstantBuffers(0, 1, &DirectionalLightconstantBuffer);
	//inmediateContext->PSSetConstantBuffers(1, 1, &PointLightconstantBuffer);
	//inmediateContext->PSSetConstantBuffers(2, 1, &SpotLightconstantBuffer);
	//inmediateContext->PSSetConstantBuffers(4, 1, &cameraPositionBuffer);
	//inmediateContext->RSSetState(rasterState);

	//inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	//memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
	//inmediateContext->Unmap(SceneconstantBuffer, 0);

	//inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	//memcpy(Resource.pData, &WtoShader[1], sizeof(SEND_TO_WORLD));
	//inmediateContext->Unmap(WorldconstantBuffer, 0);

	//inmediateContext->Map(DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	//memcpy(Resource.pData, &directionalLight[0], sizeof(SEND_DIRECTIONAL_LIGHT));
	//inmediateContext->Unmap(DirectionalLightconstantBuffer, 0);

	//inmediateContext->Map(PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	//memcpy(Resource.pData, &PointLightToS[0], sizeof(SEND_POINT_LIGHT));
	//inmediateContext->Unmap(PointLightconstantBuffer, 0);

	//inmediateContext->Map(SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	//memcpy(Resource.pData, &SpotLightToS, sizeof(SEND_SPOT_LIGHT));
	//inmediateContext->Unmap(SpotLightconstantBuffer, 0);

	//inmediateContext->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	//memcpy(Resource.pData, &myView, sizeof(SEND_SPOT_LIGHT));
	//inmediateContext->Unmap(cameraPositionBuffer, 0);

	//inmediateContext->GSSetShader(geometryshader, nullptr, 0);
	//inmediateContext->PSSetShader(pixelShader, nullptr, 0);
	//inmediateContext->VSSetShader(multiTexturingVS, nullptr, 0);
	//sstride = sizeof(SimpleVertex);
	//soffset = 0;

	//inmediateContext->IASetInputLayout(vertexLayout);

	//inmediateContext->IASetVertexBuffers(0, 1, &GroundVertexbuffer, &sstride, &soffset);
	//inmediateContext->IASetIndexBuffer(GroundIndexbuffer, DXGI_FORMAT_R32_UINT, 0);

	//inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[9]);
	//inmediateContext->PSSetSamplers(0, 1, &CubesTexSamplerState);
	//
	//inmediateContext->DrawIndexed(groundIndex, 0, 0);

	////for opaque
	//inmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	WAIT_FOR_THREAD(&myMinimap);
	if (commandList[0])
		inmediateContext->ExecuteCommandList(commandList[0], true);
	SAFE_RELEASE(commandList[0]);

	swapchain->Present(0, 0);

	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	swapchain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(renderTargetView);
	SAFE_DELETE(renderTargetView);

	SAFE_RELEASE(swapchain);
	SAFE_DELETE(swapchain);

	SAFE_RELEASE(device);
	SAFE_DELETE(device);

	SAFE_RELEASE(inmediateContext);
	SAFE_DELETE(inmediateContext);

	SAFE_DELETE(viewport);
	SAFE_DELETE(differentViewport);
	SAFE_DELETE(RTTviewport);

	SAFE_RELEASE(vertexShader);
	SAFE_DELETE(vertexShader);

	SAFE_RELEASE(geometryshader);
	SAFE_DELETE(geometryshader);

	SAFE_RELEASE(pixelShader);
	SAFE_DELETE(pixelShader);

	SAFE_RELEASE(zBuffer);
	SAFE_DELETE(zBuffer);

	SAFE_RELEASE(stencilState);
	SAFE_DELETE(stencilState);

	SAFE_RELEASE(stencilView);
	SAFE_DELETE(stencilView);

	SAFE_RELEASE(rasterState);
	SAFE_DELETE(rasterState);

	SAFE_RELEASE(SkyvertexShader);
	SAFE_DELETE(SkyvertexShader);

	SAFE_RELEASE(SkypixelShader);
	SAFE_DELETE(SkypixelShader);

	SAFE_RELEASE(GroundVertexbuffer);
	SAFE_DELETE(GroundVertexbuffer);

	SAFE_RELEASE(GroundIndexbuffer);
	SAFE_DELETE(GroundIndexbuffer);

	SAFE_RELEASE(reverserasterState);
	SAFE_DELETE(reverserasterState);

	SAFE_RELEASE(noLPS);
	SAFE_DELETE(noLPS);
	
	SAFE_RELEASE(postPS);
	SAFE_DELETE(postPS);
	
	SAFE_RELEASE(postNightPS);
	SAFE_DELETE(postNightPS);
	
	for (int i = 0; i < 27; i++)
	{
		SAFE_RELEASE(shaderResourceView[i]);
		SAFE_DELETE(shaderResourceView[i]);
	}

	SAFE_RELEASE(WorldconstantBuffer);
	SAFE_DELETE(WorldconstantBuffer);

	SAFE_RELEASE(SceneconstantBuffer);
	SAFE_DELETE(SceneconstantBuffer);

	SAFE_RELEASE(samplerState);
	SAFE_DELETE(samplerState);

	SAFE_RELEASE(vertexLayout);
	SAFE_DELETE(vertexLayout);

	for (int i = 0; i < 11; i++)
	{
		SAFE_RELEASE(ObjectVertexbuffer[i]);
		SAFE_DELETE(ObjectVertexbuffer[i]);

		SAFE_RELEASE(ObjectIndexbuffer[i]);
		SAFE_DELETE(ObjectIndexbuffer[i]);
	}

	SAFE_RELEASE(SceneconstantBuffer);
	SAFE_DELETE(SceneconstantBuffer);

	SAFE_RELEASE(WorldconstantBuffer);
	SAFE_DELETE(WorldconstantBuffer);

	SAFE_RELEASE(samplerState);
	SAFE_DELETE(samplerState);

	SAFE_RELEASE(SkyVertexbuffer);
	SAFE_DELETE(SkyVertexbuffer);

	SAFE_RELEASE(SkyIndexbuffer);
	SAFE_DELETE(SkyIndexbuffer);

	SAFE_RELEASE(CubesTexSamplerState);
	SAFE_DELETE(CubesTexSamplerState);

	for (int i = 0; i < 2; i++)
	{
		SAFE_RELEASE(deferredcontext[i]);
		SAFE_DELETE(deferredcontext[i]);

		SAFE_RELEASE(commandList[i]);
		SAFE_DELETE(commandList[i]);
	}

	SAFE_RELEASE(SkyrasterState);
	SAFE_DELETE(SkyrasterState);

	SAFE_RELEASE(GroundVertexbuffer);
	SAFE_DELETE(GroundVertexbuffer);

	SAFE_RELEASE(GroundIndexbuffer);
	SAFE_DELETE(GroundIndexbuffer);

	SAFE_RELEASE(DirectionalLightconstantBuffer);
	SAFE_DELETE(DirectionalLightconstantBuffer);

	SAFE_RELEASE(objectNormalMappingPS);
	SAFE_DELETE(objectNormalMappingPS);

	SAFE_RELEASE(objectNormalMappingVS);
	SAFE_DELETE(objectNormalMappingVS);

	SAFE_RELEASE(SpotLightconstantBuffer);
	SAFE_DELETE(SpotLightconstantBuffer);

	SAFE_RELEASE(PointLightconstantBuffer);
	SAFE_DELETE(PointLightconstantBuffer);

	SAFE_RELEASE(cameraPositionBuffer);
	SAFE_DELETE(cameraPositionBuffer);

	SAFE_RELEASE(instanceConstantBuffer);
	SAFE_DELETE(instanceConstantBuffer);

	SAFE_RELEASE(Blending);
	SAFE_DELETE(Blending);

	SAFE_RELEASE(multiTexturingVS);
	SAFE_DELETE(multiTexturingVS);

	SAFE_RELEASE(RTTTextureMap);
	SAFE_DELETE(RTTTextureMap);

	SAFE_RELEASE(shaderResourceViewMap);
	SAFE_DELETE(shaderResourceViewMap);

	SAFE_RELEASE(RTTstencilView);
	SAFE_DELETE(RTTstencilView);

	SAFE_RELEASE(RTTrenderTargetView);
	SAFE_DELETE(RTTrenderTargetView);

	SAFE_RELEASE(RTTzBuffer);
	SAFE_DELETE(RTTzBuffer);

	SAFE_RELEASE(SpecularConstantBuffer);
	SAFE_DELETE(SpecularConstantBuffer);

	SAFE_RELEASE(timeBuffer);
	SAFE_DELETE(timeBuffer);

	SAFE_RELEASE(multiTexturingPS);
	SAFE_DELETE(multiTexturingPS);

	SAFE_RELEASE(cubeVertexbuffer);
	SAFE_DELETE(cubeVertexbuffer);

	SAFE_RELEASE(CubeVertexShader);
	SAFE_DELETE(CubeVertexShader);

	SAFE_RELEASE(CubeGeometryShader);
	SAFE_DELETE(CubeGeometryShader);

	SAFE_RELEASE(objectVS);
	SAFE_DELETE(objectVS);
		
	UnregisterClass(L"DirectXApplication", application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance, (WNDPROC)WndProc);
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
//********************* END WARNING ************************//

bool DEMO_APP::LoadObjectFromFile(string fileName, int slot)
{
	HRESULT hr = 0;

	std::wifstream fileIn(fileName.c_str());	//Open file
	std::wstring meshMatLib;					//String to hold our obj material library filename

	//Arrays to store our model's information
	std::vector<unsigned int> indices;
	std::vector<XMFLOAT3> vertPos;
	std::vector<XMFLOAT3> vertNorm;
	std::vector<XMFLOAT3> vertTexCoord;

	//Vertex definition indices
	std::vector<int> vertPosIndex;
	std::vector<int> vertNormIndex;
	std::vector<int> vertTCIndex;

	//Temp variables to store into vectors
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;		//The variable we will use to store one char from file at a time
	std::wstring face;		//Holds the string containing our face vertices
	int vIndex = 0;			//Keep track of our vertex index count
	int triangleCount = 0;	//Total Triangles
	int totalVerts = 0;
	int meshTriangles = 0;

	//Check to see if the file was opened
	if (fileIn)
	{
		while (fileIn)
		{
			checkChar = fileIn.get();	//Get next char

			switch (checkChar)
			{
			case '#':
				checkChar = fileIn.get();
				while (checkChar != '\n')
					checkChar = fileIn.get();
				break;
			case 'v':	//Get Vertex Descriptions
				checkChar = fileIn.get();
				if (checkChar == ' ')	//v - vert position
				{
					float vz, vy, vx;
					fileIn >> vx >> vy >> vz;	//Store the next three types


					vertPos.push_back(XMFLOAT3(vx, vy, vz));
				}
				if (checkChar == 't')	//vt - vert tex coords
				{
					float vtcu, vtcv;
					fileIn >> vtcu >> vtcv;		//Store next two types
					vertTexCoord.push_back(XMFLOAT3(vtcu, 1.0f - vtcv, 0.0f));	//Reverse the "v" axis

				}
				//Since we compute the normals later, we don't need to check for normals
				//In the file, but i'll do it here anyway
				if (checkChar == 'n')	//vn - vert normal
				{
					float vnx, vny, vnz;
					fileIn >> vnx >> vny >> vnz;	//Store next three types
					vertNorm.push_back(XMFLOAT3(vnx, vny, vnz));
				}
				break;

				//Get Face Index
			case 'f':	//f - defines the faces
				checkChar = fileIn.get();
				if (checkChar == ' ')
				{
					face = L"";
					std::wstring VertDef;	//Holds one vertex definition at a time
					triangleCount = 0;

					checkChar = fileIn.get();
					while (checkChar != '\n')
					{
						face += checkChar;			//Add the char to our face string
						checkChar = fileIn.get();	//Get the next Character
						if (checkChar == ' ')		//If its a space...
							triangleCount++;		//Increase our triangle count
					}

					//Check for space at the end of our face string
					if (face[face.length() - 1] == ' ')
						triangleCount--;	//Each space adds to our triangle count

					triangleCount -= 1;		//Ever vertex in the face AFTER the first two are new faces

					std::wstringstream ss(face);

					if (face.length() > 0)
					{
						int firstVIndex, lastVIndex;	//Holds the first and last vertice's index

						for (int i = 0; i < 3; ++i)		//First three vertices (first triangle)
						{
							ss >> VertDef;	//Get vertex definition (vPos/vTexCoord/vNorm)

							std::wstring vertPart;
							int whichPart = 0;		//(vPos, vTexCoord, or vNorm)

							//Parse this string
							for (int j = 0; j < VertDef.length(); ++j)
							{
								if (VertDef[j] != '/')	//If there is no divider "/", add a char to our vertPart
									vertPart += VertDef[j];

								//If the current char is a divider "/", or its the last character in the string
								if (VertDef[j] == '/' || j == VertDef.length() - 1)
								{
									std::wistringstream wstringToInt(vertPart);	//Used to convert wstring to int

									if (whichPart == 0)	//If vPos
									{
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;		//subtract one since c++ arrays start with 0, and obj start with 1

										//Check to see if the vert pos was the only thing specified
										if (j == VertDef.length() - 1)
										{
											vertNormIndexTemp = 0;
											vertTCIndexTemp = 0;
										}
									}

									else if (whichPart == 1)	//If vTexCoord
									{
										if (vertPart != L"")	//Check to see if there even is a tex coord
										{
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;	//subtract one since c++ arrays start with 0, and obj start with 1
										}
										else	//If there is no tex coord, make a default
											vertTCIndexTemp = 0;

										//If the cur. char is the second to last in the string, then
										//there must be no normal, so set a default normal
										if (j == VertDef.length() - 1)
											vertNormIndexTemp = 0;

									}
									else if (whichPart == 2)	//If vNorm
									{
										std::wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;		//subtract one since c++ arrays start with 0, and obj start with 1
									}

									vertPart = L"";	//Get ready for next vertex part
									whichPart++;	//Move on to next vertex part					
								}
							}

							////Check to make sure there is at least one subset
							//if (subsetCount == 0)
							//{
							//	subsetIndexStart.push_back(vIndex);		//Start index for this subset
							//	subsetCount++;
							//}

							//Avoid duplicate vertices
							bool vertAlreadyExists = false;
							if (totalVerts >= 3)	//Make sure we at least have one triangle to check
							{
								//Loop through all the vertices
								for (int iCheck = 0; iCheck < totalVerts; ++iCheck)
								{
									//If the vertex position and texture coordinate in memory are the same
									//As the vertex position and texture coordinate we just now got out
									//of the obj file, we will set this faces vertex index to the vertex's
									//index value in memory. This makes sure we don't create duplicate vertices
									if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
									{
										if (vertTCIndexTemp == vertTCIndex[iCheck])
										{
											indices.push_back(iCheck);		//Set index for this vertex
											vertAlreadyExists = true;		//If we've made it here, the vertex already exists
										}
									}
								}
							}

							//If this vertex is not already in our vertex arrays, put it there
							if (!vertAlreadyExists)
							{
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;	//We created a new vertex
								indices.push_back(totalVerts - 1);	//Set index for this vertex
							}

							//If this is the very first vertex in the face, we need to
							//make sure the rest of the triangles use this vertex
							if (i == 0)
							{
								firstVIndex = indices[vIndex];	//The first vertex index of this FACE

							}

							//If this was the last vertex in the first triangle, we will make sure
							//the next triangle uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
							if (i == 2)
							{
								lastVIndex = indices[vIndex];	//The last vertex index of this TRIANGLE
							}
							vIndex++;	//Increment index count
						}

						meshTriangles++;	//One triangle down

						//If there are more than three vertices in the face definition, we need to make sure
						//we convert the face to triangles. We created our first triangle above, now we will
						//create a new triangle for every new vertex in the face, using the very first vertex
						//of the face, and the last vertex from the triangle before the current triangle
						for (int l = 0; l < triangleCount - 1; ++l)	//Loop through the next vertices to create new triangles
						{
							//First vertex of this triangle (the very first vertex of the face too)
							indices.push_back(firstVIndex);			//Set index for this vertex
							vIndex++;

							//Second Vertex of this triangle (the last vertex used in the tri before this one)
							indices.push_back(lastVIndex);			//Set index for this vertex
							vIndex++;

							//Get the third vertex for this triangle
							ss >> VertDef;

							std::wstring vertPart;
							int whichPart = 0;

							//Parse this string (same as above)
							for (int j = 0; j < VertDef.length(); ++j)
							{
								if (VertDef[j] != '/')
									vertPart += VertDef[j];
								if (VertDef[j] == '/' || j == VertDef.length() - 1)
								{
									std::wistringstream wstringToInt(vertPart);

									if (whichPart == 0)
									{
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;

										//Check to see if the vert pos was the only thing specified
										if (j == VertDef.length() - 1)
										{
											vertTCIndexTemp = 0;
											vertNormIndexTemp = 0;
										}
									}
									else if (whichPart == 1)
									{
										if (vertPart != L"")
										{
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;
										}
										else
											vertTCIndexTemp = 0;
										if (j == VertDef.length() - 1)
											vertNormIndexTemp = 0;

									}
									else if (whichPart == 2)
									{
										std::wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;
									}

									vertPart = L"";
									whichPart++;
								}
							}

							//Check for duplicate vertices
							bool vertAlreadyExists = false;
							if (totalVerts >= 3)	//Make sure we at least have one triangle to check
							{
								for (int iCheck = 0; iCheck < totalVerts; ++iCheck)
								{
									if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
									{
										if (vertTCIndexTemp == vertTCIndex[iCheck])
										{
											indices.push_back(iCheck);			//Set index for this vertex
											vertAlreadyExists = true;		//If we've made it here, the vertex already exists
										}
									}
								}
							}

							if (!vertAlreadyExists)
							{
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;					//New vertex created, add to total verts
								indices.push_back(totalVerts - 1);		//Set index for this vertex
							}

							//Set the second vertex for the next triangle to the last vertex we got		
							lastVIndex = indices[vIndex];	//The last vertex index of this TRIANGLE

							meshTriangles++;	//New triangle defined
							vIndex++;
						}
					}
				}
				break;

			case 'm':	//mtllib - material library filename
				checkChar = fileIn.get();
				if (checkChar == 't')
				{
					checkChar = fileIn.get();
					if (checkChar == 'l')
					{
						checkChar = fileIn.get();
						if (checkChar == 'l')
						{
							checkChar = fileIn.get();
							if (checkChar == 'i')
							{
								checkChar = fileIn.get();
								if (checkChar == 'b')
								{
									checkChar = fileIn.get();
									if (checkChar == ' ')
									{
										//Store the material libraries file name
										fileIn >> meshMatLib;
									}
								}
							}
						}
					}
				}

				break;

			default:
				break;
			}
		}
	}
	else	//If we could not open the file
	{

		//create message
		std::wstring message = L"Could not open: ";

		MessageBox(0, message.c_str(),	//display message
			L"Error", MB_OK);

		return false;
	}

	//Close the obj file, and open the mtl file
	fileIn.close();

	std::vector<SimpleVertex> vertices;
	SimpleVertex tempVert;

	//Create our vertices using the information we got 
	//from the file and store them in a vector
	for (int j = 0; j < totalVerts; ++j)
	{
		tempVert.Pos = vertPos[vertPosIndex[j]];
		tempVert.norm = vertNorm[vertNormIndex[j]];
		tempVert.UV = vertTexCoord[vertTCIndex[j]];

		vertices.push_back(tempVert);
	}

	//////////////////////Compute Normals///////////////////////////
	//If computeNormals was set to true then we will create our own
	//normals, if it was set to false we will use the obj files normals
	std::vector<XMFLOAT3> tempNormal;

	//normalized and unnormalized normals
	XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

	//Used to get vectors (sides) from the position of the verts
	float vecX, vecY, vecZ;

	//Two edges of our triangle
	XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	//Compute face normals
	for (int i = 0; i < meshTriangles; ++i)
	{
		//Get the vector describing one edge of our triangle (edge 0,2)
		vecX = vertices[indices[(i * 3)]].Pos.x - vertices[indices[(i * 3) + 2]].Pos.x;
		vecY = vertices[indices[(i * 3)]].Pos.y - vertices[indices[(i * 3) + 2]].Pos.y;
		vecZ = vertices[indices[(i * 3)]].Pos.z - vertices[indices[(i * 3) + 2]].Pos.z;
		edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our first edge

		//Get the vector describing another edge of our triangle (edge 2,1)
		vecX = vertices[indices[(i * 3) + 2]].Pos.x - vertices[indices[(i * 3) + 1]].Pos.x;
		vecY = vertices[indices[(i * 3) + 2]].Pos.y - vertices[indices[(i * 3) + 1]].Pos.y;
		vecZ = vertices[indices[(i * 3) + 2]].Pos.z - vertices[indices[(i * 3) + 1]].Pos.z;
		edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our second edge

		//Cross multiply the two edge vectors to get the un-normalized face normal
		XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));
		tempNormal.push_back(unnormalized);			//Save unormalized normal (for normal averaging)
	}

	//Compute vertex normals (normal Averaging)
	XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	int facesUsing = 0;
	float tX;
	float tY;
	float tZ;

	//Go through each vertex
	for (int i = 0; i < totalVerts; ++i)
	{
		//Check which triangles use this vertex
		for (int j = 0; j < meshTriangles; ++j)
		{
			if (indices[j * 3] == i ||
				indices[(j * 3) + 1] == i ||
				indices[(j * 3) + 2] == i)
			{
				tX = XMVectorGetX(normalSum) + tempNormal[j].x;
				tY = XMVectorGetY(normalSum) + tempNormal[j].y;
				tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

				normalSum = XMVectorSet(tX, tY, tZ, 0.0f);	//If a face is using the vertex, add the unormalized face normal to the normalSum
				facesUsing++;
			}
		}

		//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
		normalSum = normalSum / facesUsing;

		//Normalize the normalSum vector
		normalSum = XMVector3Normalize(normalSum);

		//Store the normal in our current vertex
		vertices[i].norm.x = XMVectorGetX(normalSum);
		vertices[i].norm.y = XMVectorGetY(normalSum);
		vertices[i].norm.z = XMVectorGetZ(normalSum);

		//Clear normalSum and facesUsing for next vertex
		normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		facesUsing = 0;

	}

	//tangents
	std::vector<XMFLOAT3> tempTangent;
	XMFLOAT3 tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float tcU1, tcV1, tcU2, tcV2;

	edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < meshTriangles; ++i)
	{

		vecX = vertices[indices[(i * 3)]].Pos.x - vertices[indices[(i * 3) + 2]].Pos.x;
		vecY = vertices[indices[(i * 3)]].Pos.y - vertices[indices[(i * 3) + 2]].Pos.y;
		vecZ = vertices[indices[(i * 3)]].Pos.z - vertices[indices[(i * 3) + 2]].Pos.z;
		edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);

		vecX = vertices[indices[(i * 3) + 2]].Pos.x - vertices[indices[(i * 3) + 1]].Pos.x;
		vecY = vertices[indices[(i * 3) + 2]].Pos.y - vertices[indices[(i * 3) + 1]].Pos.y;
		vecZ = vertices[indices[(i * 3) + 2]].Pos.z - vertices[indices[(i * 3) + 1]].Pos.z;
		edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);

		XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));

		tempNormal.push_back(unnormalized);

		tcU1 = vertices[indices[(i * 3)]].UV.x - vertices[indices[(i * 3) + 2]].UV.x;
		tcV1 = vertices[indices[(i * 3)]].UV.y - vertices[indices[(i * 3) + 2]].UV.y;

		tcU2 = vertices[indices[(i * 3) + 2]].UV.x - vertices[indices[(i * 3) + 1]].UV.x;
		tcV2 = vertices[indices[(i * 3) + 2]].UV.y - vertices[indices[(i * 3) + 1]].UV.y;

		tangent.x = (tcV1 * XMVectorGetX(edge1) - tcV2 * XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
		tangent.y = (tcV1 * XMVectorGetY(edge1) - tcV2 * XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
		tangent.z = (tcV1 * XMVectorGetZ(edge1) - tcV2 * XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));

		tempTangent.push_back(tangent);
	}

	normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	facesUsing = 0;

	for (int i = 0; i < totalVerts; ++i)
	{
		for (int j = 0; j < meshTriangles; ++j)
		{
			if (indices[j * 3] == i ||
				indices[(j * 3) + 1] == i ||
				indices[(j * 3) + 2] == i)
			{
				tX = XMVectorGetX(normalSum) + tempNormal[j].x;
				tY = XMVectorGetY(normalSum) + tempNormal[j].y;
				tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

				normalSum = XMVectorSet(tX, tY, tZ, 0.0f);

				tX = XMVectorGetX(tangentSum) + tempTangent[j].x;
				tY = XMVectorGetY(tangentSum) + tempTangent[j].y;
				tZ = XMVectorGetZ(tangentSum) + tempTangent[j].z;

				tangentSum = XMVectorSet(tX, tY, tZ, 0.0f);

				facesUsing++;
			}
		}

		normalSum = normalSum / facesUsing;
		tangentSum = tangentSum / facesUsing;

		normalSum = XMVector3Normalize(normalSum);
		tangentSum = XMVector3Normalize(tangentSum);

		vertices[i].norm.x = XMVectorGetX(normalSum);
		vertices[i].norm.y = XMVectorGetY(normalSum);
		vertices[i].norm.z = XMVectorGetZ(normalSum);

		vertices[i].tangent.x = XMVectorGetX(tangentSum);
		vertices[i].tangent.y = XMVectorGetY(tangentSum);
		vertices[i].tangent.z = XMVectorGetZ(tangentSum);

		normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		facesUsing = 0;
	}

	//Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * meshTriangles * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	device->CreateBuffer(&indexBufferDesc, &iinitData, &ObjectIndexbuffer[slot]);

	//Create Vertex Buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * totalVerts;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &ObjectVertexbuffer[slot]);

	indexCount[slot] = meshTriangles * 3;

	return true;
}

void DEMO_APP::CreateSphere(int LatLines, int LongLines)
{
	HRESULT hr;
	int NumSphereVertices = ((LatLines - 2) * LongLines) + 2;
	int NumSphereFaces = ((LatLines - 3)*(LongLines)* 2) + (LongLines * 2);

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;

	std::vector<SimpleVertex> vertices(NumSphereVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].Pos.x = 0.0f;
	vertices[0].Pos.y = 0.0f;
	vertices[0].Pos.z = 1.0f;

	for (unsigned short i = 0; i < LatLines - 2; ++i)
	{
		spherePitch = (i + 1) * (3.14 / (LatLines - 1));
		Rotationx = XMMatrixRotationX(spherePitch);
		for (unsigned short j = 0; j < LongLines; ++j)
		{
			sphereYaw = j * (6.28 / (LongLines));
			Rotationy = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*LongLines + j + 1].Pos.x = XMVectorGetX(currVertPos);
			vertices[i*LongLines + j + 1].Pos.y = XMVectorGetY(currVertPos);
			vertices[i*LongLines + j + 1].Pos.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[NumSphereVertices - 1].Pos.x = 0.0f;
	vertices[NumSphereVertices - 1].Pos.y = 0.0f;
	vertices[NumSphereVertices - 1].Pos.z = -1.0f;


	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * NumSphereVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &SkyVertexbuffer);


	std::vector<unsigned short> indices(NumSphereFaces * 3);

	int k = 0;
	for (unsigned short l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = 0;
		indices[k + 1] = l + 1;
		indices[k + 2] = l + 2;
		k += 3;
	}

	indices[k] = 0;
	indices[k + 1] = LongLines;
	indices[k + 2] = 1;
	k += 3;

	for (unsigned short i = 0; i < LatLines - 3; ++i)
	{
		for (unsigned short j = 0; j < LongLines - 1; ++j)
		{
			indices[k] = i*LongLines + j + 1;
			indices[k + 1] = i*LongLines + j + 2;
			indices[k + 2] = (i + 1)*LongLines + j + 1;

			indices[k + 3] = (i + 1)*LongLines + j + 1;
			indices[k + 4] = i*LongLines + j + 2;
			indices[k + 5] = (i + 1)*LongLines + j + 2;

			k += 6; // next quad
		}

		indices[k] = (i*LongLines) + LongLines;
		indices[k + 1] = (i*LongLines) + 1;
		indices[k + 2] = ((i + 1)*LongLines) + LongLines;

		indices[k + 3] = ((i + 1)*LongLines) + LongLines;
		indices[k + 4] = (i*LongLines) + 1;
		indices[k + 5] = ((i + 1)*LongLines) + 1;

		k += 6;
	}

	for (DWORD l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = NumSphereVertices - 1;
		indices[k + 1] = (NumSphereVertices - 1) - (l + 1);
		indices[k + 2] = (NumSphereVertices - 1) - (l + 2);
		k += 3;
	}

	indices[k] = NumSphereVertices - 1;
	indices[k + 1] = (NumSphereVertices - 1) - LongLines;
	indices[k + 2] = NumSphereVertices - 2;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned short) * NumSphereFaces * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	hr = device->CreateBuffer(&indexBufferDesc, &iinitData, &SkyIndexbuffer);

	sphereIndex = NumSphereFaces * 3;
}

void DEMO_APP::Input()
{
	XMMATRIX mat = XMMatrixInverse(nullptr, StoShader[0].ViewM);

	if (GetAsyncKeyState('W'))
	{
		mat = XMMatrixTranslation(0.0f, 15.0f * dt, 0.0f) * mat;
	}
	else if (GetAsyncKeyState('S'))
	{
		mat = XMMatrixTranslation(0.0f, -15.0f * dt, 0.0f) * mat;;
	}
	else if (GetAsyncKeyState('A'))
	{
		mat = XMMatrixTranslation(-15.0f * dt, 0.0f, 0.0f) * mat;
	}
	else if (GetAsyncKeyState('D'))
	{
		mat = XMMatrixTranslation(15.0f * dt, 0.0f, 0.0f) * mat;
	}

	if (GetAsyncKeyState('U'))
	{
		mat = XMMatrixTranslation(0.0f, 0.0f, 15.0f * dt) * mat;
	}
	else if (GetAsyncKeyState('J'))
	{
		mat = XMMatrixTranslation(0.0f, 0.0f, -15.0f * dt) * mat;
	}

	else if (GetAsyncKeyState('X'))
	{
		XMFLOAT3 rot = XMFLOAT3(2.0f, 0.0f, 0.0f);
		XMVECTOR aux = XMLoadFloat3(&rot);
		mat = XMMatrixRotationAxis(aux, 1.0f *dt) * mat;
	}
	else if (GetAsyncKeyState('Z'))
	{
		XMFLOAT3 rot = XMFLOAT3(2.0f, 0.0f, 0.0f);
		XMVECTOR aux = XMLoadFloat3(&rot);
		mat = XMMatrixRotationAxis(aux, -1.0f *dt) * mat;
	}
	else if (GetAsyncKeyState('C')) //left
	{

		XMFLOAT4 pos = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR help = mat.r[3];
		mat = mat * XMMatrixRotationY(-XMConvertToRadians(1.5f + dt));
		mat.r[3] = help;
	}
	else if (GetAsyncKeyState('V')) //right
	{
		XMFLOAT4 pos = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR help = mat.r[3];
		mat = mat * XMMatrixRotationY(XMConvertToRadians(1.5f + dt));
		mat.r[3] = help;
	}
	
	else if (GetAsyncKeyState(VK_NUMPAD1) & 0x80 != 0)
	{
		post = !post;
	}

	else if (GetAsyncKeyState(VK_NUMPAD2) & 0x80 != 0)
	{
		post2 = !post2;
	}
	
	StoShader[0].ViewM = XMMatrixInverse(nullptr, mat);
}

void LoadingThread(DEMO_APP* myApp) //textures
{
	HRESULT hrT;

	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/nightsky.dds", nullptr, &myApp->shaderResourceView[0]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/woods.dds", nullptr, &myApp->shaderResourceView[1]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/noisemap.dds", nullptr, &myApp->shaderResourceView[2]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/conifer_tronco.dds", nullptr, &myApp->shaderResourceView[3]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/conifer_leaves.dds", nullptr, &myApp->shaderResourceView[4]);
	//normal maps
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/conifer_tronco_normal.dds", nullptr, &myApp->shaderResourceView[5]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/conifer_leaves_normal.dds", nullptr, &myApp->shaderResourceView[6]);
	//RTT
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/fungus.dds", nullptr, &myApp->shaderResourceView[7]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/StoneWall.dds", nullptr, &myApp->shaderResourceView[8]);
	//water
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/watar.dds", nullptr, &myApp->shaderResourceView[9]);
	//clown
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/clown.dds", nullptr, &myApp->shaderResourceView[10]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/clown_normal.dds", nullptr, &myApp->shaderResourceView[11]);
	//tower
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/tower_D.dds", nullptr, &myApp->shaderResourceView[12]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/tuwer_Normal.dds", nullptr, &myApp->shaderResourceView[13]);
	//pixel night vision
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/visionnoise.dds", nullptr, &myApp->shaderResourceView[14]);
	//beast
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/beast.dds", nullptr, &myApp->shaderResourceView[15]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/beast_normals.dds", nullptr, &myApp->shaderResourceView[16]);
	//assaulter
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/assault.dds", nullptr, &myApp->shaderResourceView[17]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/assault_normals.dds", nullptr, &myApp->shaderResourceView[18]);
	//grave
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/grave_color.dds", nullptr, &myApp->shaderResourceView[19]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/grave_normal.dds", nullptr, &myApp->shaderResourceView[20]);
	//zombiehand
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/zombie_hand.dds", nullptr, &myApp->shaderResourceView[21]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/zombie_hand_normal.dds", nullptr, &myApp->shaderResourceView[22]);
	//zombies
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/wasterdiffuse.dds", nullptr, &myApp->shaderResourceView[23]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/waster_normal.dds", nullptr, &myApp->shaderResourceView[24]);
	//lament
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/lament_diffuse.dds", nullptr, &myApp->shaderResourceView[25]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/lament_normal.dds", nullptr, &myApp->shaderResourceView[26]);
}

void StatuesLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//Bloat_Halloween.obj", 3); //clown
}

void BeastLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//beast.obj", 5); //beast
}

void ObjectLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//were.obj", 2); //werewolf
}

void FolliageLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//leaves.obj", 0);
	myApp->LoadObjectFromFile("Assets//Models//conifer.obj", 1);
}

void AssaulterLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//Assaulter.obj", 6); //Assaulter
}

void TowerLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//towur.obj", 4);
}

void GraveLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//dirt_grave.obj", 7);
}

void ZombieHandLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//zombie_hand.obj", 8);
}

void ZombieLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//WasterUnarmed.obj", 9);
}

void LamentLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//Lambent_Female.obj", 10);
}

void RRTDraw(DEMO_APP* myApp)	
{
	//RTTDraw
	XMMATRIX translation = XMMatrixIdentity();
	translation = XMMatrixTranslation(0.0f, 0.0f, 10.0f);

	XMMATRIX scaling = XMMatrixIdentity();
	scaling = XMMatrixScaling(0.2f, 0.2f, 0.2f);

	XMMATRIX rotateY = XMMatrixIdentity();
	XMMATRIX rotateZ = XMMatrixIdentity();
	XMMATRIX rotateX = XMMatrixIdentity();

	XMFLOAT3 rot = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR aux = XMLoadFloat3(&rot);
	rotateY *= XMMatrixRotationAxis(aux, myApp->dt);

	rot = XMFLOAT3(0.0f, 0.0f, 1.0f);
	aux = XMLoadFloat3(&rot);
	rotateZ *= XMMatrixRotationAxis(aux, -myApp->dt);

	rot = XMFLOAT3(1.0f, 0.0f, 0.0f);
	aux = XMLoadFloat3(&rot);
	rotateX *= XMMatrixRotationAxis(aux, 0.7f * myApp->dt);

	myApp->rotationRTT *= rotateX * rotateY *rotateZ;

	myApp->WtoShader[6].World = scaling * myApp->rotationRTT * translation;

	if (myApp->ObjectIndexbuffer[2] && myApp->ObjectVertexbuffer[2])
	{
		myApp->deferredcontext[1]->OMSetRenderTargets(1, &myApp->RTTrenderTargetView, myApp->RTTstencilView);
		myApp->deferredcontext[1]->ClearRenderTargetView(myApp->RTTrenderTargetView, Colors::CornflowerBlue);
		myApp->deferredcontext[1]->ClearDepthStencilView(myApp->RTTstencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xFF);

		myApp->deferredcontext[1]->RSSetViewports(1, myApp->RTTviewport);
		//myApp->deferredcontext[1]->OMSetDepthStencilState(myApp->stencilState, 1);

		myApp->deferredcontext[1]->PSSetShader(myApp->multiTexturingPS, nullptr, 0);
		myApp->deferredcontext[1]->VSSetShader(myApp->multiTexturingVS, nullptr, 0);

		myApp->deferredcontext[1]->VSSetConstantBuffers(0, 1, &myApp->WorldconstantBuffer);
		myApp->deferredcontext[1]->VSSetConstantBuffers(1, 1, &myApp->SceneconstantBuffer);
		//light
		myApp->deferredcontext[1]->PSSetConstantBuffers(0, 1, &myApp->DirectionalLightconstantBuffer);
		myApp->deferredcontext[1]->PSSetConstantBuffers(1, 1, &myApp->PointLightconstantBuffer);
		myApp->deferredcontext[1]->PSSetConstantBuffers(2, 1, &myApp->SpotLightconstantBuffer);
		myApp->deferredcontext[1]->PSSetConstantBuffers(3, 1, &myApp->SpecularConstantBuffer);

		myApp->deferredcontext[1]->PSSetSamplers(0, 1, &myApp->samplerState);

		D3D11_MAPPED_SUBRESOURCE  Resource;

		myApp->deferredcontext[1]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->WtoShader[6], sizeof(DEMO_APP::SEND_TO_WORLD));
		myApp->deferredcontext[1]->Unmap(myApp->WorldconstantBuffer, 0);

		myApp->deferredcontext[1]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->StoShader[3], sizeof(DEMO_APP::SEND_TO_SCENE));
		myApp->deferredcontext[1]->Unmap(myApp->SceneconstantBuffer, 0);

		myApp->deferredcontext[1]->Map(myApp->DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->directionalLight[1], sizeof(DEMO_APP::SEND_DIRECTIONAL_LIGHT));
		myApp->deferredcontext[1]->Unmap(myApp->DirectionalLightconstantBuffer, 0);

		myApp->deferredcontext[1]->Map(myApp->PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->PointLightToS[1], sizeof(DEMO_APP::SEND_POINT_LIGHT));
		myApp->deferredcontext[1]->Unmap(myApp->PointLightconstantBuffer, 0);

		myApp->deferredcontext[1]->Map(myApp->SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->SpotLightToS, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
		myApp->deferredcontext[1]->Unmap(myApp->SpotLightconstantBuffer, 0);

		myApp->deferredcontext[1]->Map(myApp->SpecularConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->speculatToShader, sizeof(DEMO_APP::SEND_TO_SPECULO));
		myApp->deferredcontext[1]->Unmap(myApp->SpecularConstantBuffer, 0);

		UINT sstride = sizeof(DEMO_APP::SimpleVertex);
		UINT soffset = 0;

		myApp->deferredcontext[1]->PSSetShaderResources(0, 1, &myApp->shaderResourceView[7]);
		myApp->deferredcontext[1]->PSSetShaderResources(1, 1, &myApp->shaderResourceView[8]);

		myApp->deferredcontext[1]->IASetVertexBuffers(0, 1, &myApp->ObjectVertexbuffer[2], &sstride, &soffset);
		myApp->deferredcontext[1]->IASetIndexBuffer(myApp->ObjectIndexbuffer[2], DXGI_FORMAT_R32_UINT, 0);

		myApp->deferredcontext[1]->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		myApp->deferredcontext[1]->IASetInputLayout(myApp->vertexLayout);

		myApp->deferredcontext[1]->DrawIndexed(myApp->indexCount[2], 0, 0);
		myApp->deferredcontext[1]->GenerateMips(myApp->shaderResourceViewMap);

		if (myApp->post2 == true)
		{
			myApp->myTime.time.x = myApp->time.TotalTime();

			myApp->deferredcontext[1]->ClearDepthStencilView(myApp->RTTstencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xFF);

			myApp->deferredcontext[1]->PSSetShader(myApp->postPS, nullptr, 0);

			myApp->deferredcontext[1]->VSSetConstantBuffers(0, 1, &myApp->WorldconstantBuffer);
			myApp->deferredcontext[1]->VSSetConstantBuffers(1, 1, &myApp->SceneconstantBuffer);
			//light
			myApp->deferredcontext[1]->PSSetConstantBuffers(0, 1, &myApp->DirectionalLightconstantBuffer);
			myApp->deferredcontext[1]->PSSetConstantBuffers(1, 1, &myApp->PointLightconstantBuffer);
			myApp->deferredcontext[1]->PSSetConstantBuffers(2, 1, &myApp->SpotLightconstantBuffer);
			myApp->deferredcontext[1]->PSSetConstantBuffers(3, 1, &myApp->SpecularConstantBuffer);
			myApp->deferredcontext[1]->PSSetConstantBuffers(4, 1, &myApp->timeBuffer);

			myApp->deferredcontext[1]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
			memcpy(Resource.pData, &myApp->WtoShader[6], sizeof(DEMO_APP::SEND_TO_WORLD));
			myApp->deferredcontext[1]->Unmap(myApp->WorldconstantBuffer, 0);

			myApp->deferredcontext[1]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
			memcpy(Resource.pData, &myApp->StoShader[3], sizeof(DEMO_APP::SEND_TO_SCENE));
			myApp->deferredcontext[1]->Unmap(myApp->SceneconstantBuffer, 0);

			myApp->deferredcontext[1]->Map(myApp->DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
			memcpy(Resource.pData, &myApp->directionalLight[1], sizeof(DEMO_APP::SEND_DIRECTIONAL_LIGHT));
			myApp->deferredcontext[1]->Unmap(myApp->DirectionalLightconstantBuffer, 0);

			myApp->deferredcontext[1]->Map(myApp->PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
			memcpy(Resource.pData, &myApp->PointLightToS[1], sizeof(DEMO_APP::SEND_POINT_LIGHT));
			myApp->deferredcontext[1]->Unmap(myApp->PointLightconstantBuffer, 0);

			myApp->deferredcontext[1]->Map(myApp->SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
			memcpy(Resource.pData, &myApp->SpotLightToS, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
			myApp->deferredcontext[1]->Unmap(myApp->SpotLightconstantBuffer, 0);

			myApp->deferredcontext[1]->Map(myApp->SpecularConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
			memcpy(Resource.pData, &myApp->speculatToShader, sizeof(DEMO_APP::SEND_TO_SPECULO));
			myApp->deferredcontext[1]->Unmap(myApp->SpecularConstantBuffer, 0);

			myApp->deferredcontext[1]->Map(myApp->timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
			memcpy(Resource.pData, &myApp->myTime, sizeof(DEMO_APP::SEND_TIME));
			myApp->deferredcontext[1]->Unmap(myApp->timeBuffer, 0);

			myApp->deferredcontext[1]->PSSetShaderResources(0, 1, &myApp->shaderResourceView[7]);
			myApp->deferredcontext[1]->PSSetShaderResources(1, 1, &myApp->shaderResourceView[8]);
			myApp->deferredcontext[1]->PSSetShaderResources(2, 1, &myApp->shaderResourceView[14]);

			myApp->deferredcontext[1]->DrawIndexed(myApp->indexCount[2], 0, 0);
			myApp->deferredcontext[1]->GenerateMips(myApp->shaderResourceViewMap);
		}

		myApp->deferredcontext[1]->FinishCommandList(true, &myApp->commandList[1]);
	}

} //working

void MinimapDraw(DEMO_APP* myApp)
{
	//viewport2
	myApp->SpotLightToS.padding = 0.0f;
	myApp->deferredcontext[0]->OMSetRenderTargets(1, &myApp->renderTargetView, myApp->stencilView);

	myApp->deferredcontext[0]->RSSetViewports(1, myApp->differentViewport);

	myApp->deferredcontext[0]->VSSetConstantBuffers(0, 1, &myApp->WorldconstantBuffer);
	myApp->deferredcontext[0]->VSSetConstantBuffers(1, 1, &myApp->SceneconstantBuffer);
	myApp->deferredcontext[0]->VSSetConstantBuffers(2, 1, &myApp->instanceConstantBuffer);

	/*myApp->deferredcontext[0]->OMSetDepthStencilState(myApp->stencilState, 0);*/
	myApp->deferredcontext[0]->PSSetSamplers(0, 1, &myApp->samplerState);

	//NOSKYBOX
	myApp->deferredcontext[0]->RSSetState(myApp->SkyrasterState);
	myApp->deferredcontext[0]->PSSetConstantBuffers(0, 1, &myApp->cameraPositionBuffer);
	D3D11_MAPPED_SUBRESOURCE  Resource;

	myApp->deferredcontext[0]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->StoShader[1], sizeof(DEMO_APP::SEND_TO_SCENE));
	myApp->deferredcontext[0]->Unmap(myApp->SceneconstantBuffer, 0);

	myApp->deferredcontext[0]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->skytoShader, sizeof(DEMO_APP::SEND_TO_WORLD));
	myApp->deferredcontext[0]->Unmap(myApp->WorldconstantBuffer, 0);

	myApp->deferredcontext[0]->Map(myApp->cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->myView, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
	myApp->deferredcontext[0]->Unmap(myApp->cameraPositionBuffer, 0);

	myApp->deferredcontext[0]->PSSetShader(myApp->SkypixelShader, nullptr, 0);
	myApp->deferredcontext[0]->VSSetShader(myApp->SkyvertexShader, nullptr, 0);
	UINT sstride = sizeof(DEMO_APP::SimpleVertex);
	UINT soffset = 0;

	myApp->deferredcontext[0]->IASetInputLayout(myApp->vertexLayout);

	myApp->deferredcontext[0]->IASetVertexBuffers(0, 1, &myApp->SkyVertexbuffer, &sstride, &soffset);
	myApp->deferredcontext[0]->IASetIndexBuffer(myApp->SkyIndexbuffer, DXGI_FORMAT_R16_UINT, 0);

	myApp->deferredcontext[0]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	myApp->deferredcontext[0]->PSSetSamplers(0, 1, &myApp->CubesTexSamplerState);
	myApp->deferredcontext[0]->PSSetShaderResources(0, 1, &myApp->shaderResourceView[0]);

	myApp->deferredcontext[0]->DrawIndexed(myApp->sphereIndex, 0, 0);

	//ground
	myApp->WtoShader[1].World *= XMMatrixScaling(0.5f, 0.5f, 0.5f);
	//ligths
	myApp->deferredcontext[0]->PSSetConstantBuffers(0, 1, &myApp->DirectionalLightconstantBuffer);
	myApp->deferredcontext[0]->PSSetConstantBuffers(1, 1, &myApp->PointLightconstantBuffer);
	myApp->deferredcontext[0]->PSSetConstantBuffers(2, 1, &myApp->SpotLightconstantBuffer);
	myApp->deferredcontext[0]->PSSetConstantBuffers(4, 1, &myApp->cameraPositionBuffer);
	myApp->deferredcontext[0]->RSSetState(myApp->rasterState);

	myApp->deferredcontext[0]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->StoShader[2], sizeof(DEMO_APP::SEND_TO_SCENE));
	myApp->deferredcontext[0]->Unmap(myApp->SceneconstantBuffer, 0);

	myApp->deferredcontext[0]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->WtoShader[1], sizeof(DEMO_APP::SEND_TO_WORLD));
	myApp->deferredcontext[0]->Unmap(myApp->WorldconstantBuffer, 0);

	myApp->deferredcontext[0]->Map(myApp->DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->directionalLight[0], sizeof(DEMO_APP::SEND_DIRECTIONAL_LIGHT));
	myApp->deferredcontext[0]->Unmap(myApp->DirectionalLightconstantBuffer, 0);

	myApp->deferredcontext[0]->Map(myApp->PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->PointLightToS[0], sizeof(DEMO_APP::SEND_POINT_LIGHT));
	myApp->deferredcontext[0]->Unmap(myApp->PointLightconstantBuffer, 0);

	myApp->deferredcontext[0]->Map(myApp->SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->SpotLightToS, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
	myApp->deferredcontext[0]->Unmap(myApp->SpotLightconstantBuffer, 0);

	myApp->deferredcontext[0]->Map(myApp->cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
	memcpy(Resource.pData, &myApp->myView, sizeof(DEMO_APP::SEND_TOFOG));
	myApp->deferredcontext[0]->Unmap(myApp->cameraPositionBuffer, 0);

	myApp->deferredcontext[0]->GSSetShader(myApp->geometryshader, nullptr, 0);
	myApp->deferredcontext[0]->PSSetShader(myApp->pixelShader, nullptr, 0);
	myApp->deferredcontext[0]->VSSetShader(myApp->vertexShader, nullptr, 0);

	myApp->deferredcontext[0]->IASetInputLayout(myApp->vertexLayout);

	myApp->deferredcontext[0]->IASetVertexBuffers(0, 1, &myApp->GroundVertexbuffer, &sstride, &soffset);
	myApp->deferredcontext[0]->IASetIndexBuffer(myApp->GroundIndexbuffer, DXGI_FORMAT_R32_UINT, 0);

	myApp->deferredcontext[0]->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	myApp->deferredcontext[0]->PSSetShaderResources(0, 1, &myApp->shaderResourceView[1]);
	myApp->deferredcontext[0]->VSSetSamplers(0, 1, &myApp->CubesTexSamplerState);
	myApp->deferredcontext[0]->VSSetShaderResources(0, 1, &myApp->shaderResourceView[2]);

	myApp->deferredcontext[0]->DrawIndexed(myApp->groundIndex, 0, 0);
	myApp->WtoShader[1].World *= XMMatrixScaling(2.0f, 2.0f, 2.0f);

	//magic box
	if (myApp->shaderResourceViewMap)
	{
		myApp->deferredcontext[0]->GSSetShader(myApp->CubeGeometryShader, nullptr, 0);
		myApp->deferredcontext[0]->PSSetShader(myApp->noLPS, nullptr, 0);
		myApp->deferredcontext[0]->VSSetShader(myApp->CubeVertexShader, nullptr, 0);

		myApp->deferredcontext[0]->GSSetConstantBuffers(0, 1, &myApp->SceneconstantBuffer);
		//light
		myApp->deferredcontext[0]->PSSetConstantBuffers(0, 1, &myApp->DirectionalLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(1, 1, &myApp->PointLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(2, 1, &myApp->SpotLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(4, 1, &myApp->cameraPositionBuffer);

		myApp->deferredcontext[0]->PSSetSamplers(0, 1, &myApp->samplerState);
		myApp->deferredcontext[0]->PSSetShaderResources(0, 1, &myApp->shaderResourceViewMap);

		myApp->deferredcontext[0]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->WtoShader[7], sizeof(DEMO_APP::SEND_TO_WORLD));
		myApp->deferredcontext[0]->Unmap(myApp->WorldconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->StoShader[0], sizeof(DEMO_APP::SEND_TO_SCENE));
		myApp->deferredcontext[0]->Unmap(myApp->SceneconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->PointLightToS[0], sizeof(DEMO_APP::SEND_POINT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->PointLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->SpotLightToS, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->SpotLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->SpecularConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->speculatToShader, sizeof(DEMO_APP::SEND_TO_SPECULO));
		myApp->deferredcontext[0]->Unmap(myApp->SpecularConstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->myView, sizeof(DEMO_APP::SEND_TOFOG));
		myApp->deferredcontext[0]->Unmap(myApp->cameraPositionBuffer, 0);

		UINT stride = sizeof(DEMO_APP::SimpleVertex);
		UINT offset = 0;

		myApp->deferredcontext[0]->RSSetState(myApp->rasterState);

		myApp->deferredcontext[0]->IASetInputLayout(myApp->vertexLayout);
		myApp->deferredcontext[0]->IASetVertexBuffers(0, 1, &myApp->cubeVertexbuffer, &stride, &offset);
		//myApp->deferredcontext[0]->IASetIndexBuffer(cubeIndexbuffer, DXGI_FORMAT_R16_UINT, 0);

		myApp->deferredcontext[0]->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		myApp->deferredcontext[0]->Draw(1, 0);
	}
	//trees
	if (myApp->ObjectIndexbuffer[1] && myApp->ObjectVertexbuffer[1])
	{
		myApp->WtoShader[2].World *= XMMatrixScaling(0.5f, 0.5f, 0.5f);
		myApp->deferredcontext[0]->PSSetConstantBuffers(0, 1, &myApp->DirectionalLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(1, 1, &myApp->PointLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(2, 1, &myApp->SpotLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(4, 1, &myApp->cameraPositionBuffer);

		myApp->deferredcontext[0]->GSSetShader(nullptr, nullptr, 0);
		myApp->deferredcontext[0]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->StoShader[2], sizeof(DEMO_APP::SEND_TO_SCENE));
		myApp->deferredcontext[0]->Unmap(myApp->SceneconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->WtoShader[2], sizeof(DEMO_APP::SEND_TO_WORLD));
		myApp->deferredcontext[0]->Unmap(myApp->WorldconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->directionalLight[0], sizeof(DEMO_APP::SEND_DIRECTIONAL_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->DirectionalLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->PointLightToS[0], sizeof(DEMO_APP::SEND_POINT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->PointLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->SpotLightToS, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->SpotLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->instanceToShader, sizeof(DEMO_APP::SEND_TOINSTANCE));
		myApp->deferredcontext[0]->Unmap(myApp->instanceConstantBuffer, 0);

		myApp->deferredcontext[0]->PSSetShader(myApp->objectNormalMappingPS, nullptr, 0);
		myApp->deferredcontext[0]->VSSetShader(myApp->objectNormalMappingVS, nullptr, 0);

		myApp->deferredcontext[0]->IASetInputLayout(myApp->vertexLayout);

		myApp->deferredcontext[0]->IASetVertexBuffers(0, 1, &myApp->ObjectVertexbuffer[1], &sstride, &soffset);
		myApp->deferredcontext[0]->IASetIndexBuffer(myApp->ObjectIndexbuffer[1], DXGI_FORMAT_R32_UINT, 0);

		myApp->deferredcontext[0]->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		myApp->deferredcontext[0]->PSSetShaderResources(0, 1, &myApp->shaderResourceView[3]);
		myApp->deferredcontext[0]->PSSetShaderResources(1, 1, &myApp->shaderResourceView[5]);

		myApp->deferredcontext[0]->DrawIndexedInstanced(myApp->indexCount[1], 100, 0, 0, 0);
	}
	//leaves
	if (myApp->ObjectIndexbuffer[0] && myApp->ObjectVertexbuffer[0])
	{
		myApp->WtoShader[3].World *= XMMatrixScaling(0.5f, 0.5f, 0.5f);
		myApp->deferredcontext[0]->PSSetConstantBuffers(0, 1, &myApp->DirectionalLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(1, 1, &myApp->PointLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(2, 1, &myApp->SpotLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(4, 1, &myApp->cameraPositionBuffer);

		myApp->deferredcontext[0]->RSSetState(myApp->SkyrasterState);
		myApp->deferredcontext[0]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->StoShader[2], sizeof(DEMO_APP::SEND_TO_SCENE));
		myApp->deferredcontext[0]->Unmap(myApp->SceneconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->WtoShader[3], sizeof(DEMO_APP::SEND_TO_WORLD));
		myApp->deferredcontext[0]->Unmap(myApp->WorldconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->directionalLight[0], sizeof(DEMO_APP::SEND_DIRECTIONAL_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->DirectionalLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->instanceToShader, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->instanceConstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->instanceToShader, sizeof(DEMO_APP::SEND_TOINSTANCE));
		myApp->deferredcontext[0]->Unmap(myApp->instanceConstantBuffer, 0);

		myApp->deferredcontext[0]->PSSetShader(myApp->objectNormalMappingPS, nullptr, 0);
		myApp->deferredcontext[0]->VSSetShader(myApp->objectNormalMappingVS, nullptr, 0);

		myApp->deferredcontext[0]->IASetInputLayout(myApp->vertexLayout);

		myApp->deferredcontext[0]->IASetVertexBuffers(0, 1, &myApp->ObjectVertexbuffer[0], &sstride, &soffset);
		myApp->deferredcontext[0]->IASetIndexBuffer(myApp->ObjectIndexbuffer[0], DXGI_FORMAT_R32_UINT, 0);

		myApp->deferredcontext[0]->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		myApp->deferredcontext[0]->PSSetShaderResources(0, 1, &myApp->shaderResourceView[4]);
		myApp->deferredcontext[0]->PSSetShaderResources(1, 1, &myApp->shaderResourceView[6]);

		myApp->deferredcontext[0]->DrawIndexedInstanced(myApp->indexCount[0], 70, 0, 0, 0);

		myApp->WtoShader[2].World *= XMMatrixScaling(2.0f, 2.0f, 2.0f);
		myApp->WtoShader[3].World *= XMMatrixScaling(2.0f, 2.0f, 2.0f);
	}

	//map objects
	/*if (ObjectIndexbuffer[3] && ObjectVertexbuffer[3])
	{
		myApp->WtoShader[8].World *= XMMatrixScaling(0.5f, 0.5f, 0.5f);
		myApp->WtoShader[4].World *= XMMatrixScaling(0.05f, 0.05f, 0.05f);

		myApp->deferredcontext[0]->PSSetConstantBuffers(0, 1, &myApp->DirectionalLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(1, 1, &myApp->PointLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(2, 1, &myApp->SpotLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(4, 1, &myApp->cameraPositionBuffer);

		myApp->deferredcontext[0]->GSSetShader(nullptr, nullptr, 0);
		myApp->deferredcontext[0]->PSSetShader(myApp->objectNormalMappingPS, nullptr, 0);
		myApp->deferredcontext[0]->VSSetShader(myApp->objectVS, nullptr, 0);

		myApp->deferredcontext[0]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->StoShader[0], sizeof(DEMO_APP::SEND_TO_SCENE));
		myApp->deferredcontext[0]->Unmap(myApp->SceneconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->WtoShader[8], sizeof(DEMO_APP::SEND_TO_WORLD));
		myApp->deferredcontext[0]->Unmap(myApp->WorldconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->directionalLight[0], sizeof(DEMO_APP::SEND_DIRECTIONAL_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->DirectionalLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->PointLightToS[0], sizeof(DEMO_APP::SEND_POINT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->PointLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->SpotLightToS, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->SpotLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->instanceToShader, sizeof(DEMO_APP::SEND_TOINSTANCE));
		myApp->deferredcontext[0]->Unmap(myApp->instanceConstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->myView, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->cameraPositionBuffer, 0);

		sstride = sizeof(DEMO_APP::SimpleVertex);
		soffset = 0;

		myApp->deferredcontext[0]->IASetInputLayout(myApp->vertexLayout);

		myApp->deferredcontext[0]->IASetVertexBuffers(0, 1, &myApp->ObjectVertexbuffer[3], &sstride, &soffset);
		myApp->deferredcontext[0]->IASetIndexBuffer(myApp->ObjectIndexbuffer[3], DXGI_FORMAT_R32_UINT, 0);

		myApp->deferredcontext[0]->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		myApp->deferredcontext[0]->PSSetShaderResources(0, 1, &myApp->shaderResourceView[10]);
		myApp->deferredcontext[0]->PSSetShaderResources(1, 1, &myApp->shaderResourceView[11]);

		myApp->deferredcontext[0]->DrawIndexed(myApp->indexCount[3], 0, 0);
	}

	if (ObjectIndexbuffer[4] && ObjectVertexbuffer[4])
	{
		myApp->deferredcontext[0]->PSSetConstantBuffers(0, 1, &myApp->DirectionalLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(1, 1, &myApp->PointLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(2, 1, &myApp->SpotLightconstantBuffer);
		myApp->deferredcontext[0]->PSSetConstantBuffers(4, 1, &myApp->cameraPositionBuffer);

		myApp->deferredcontext[0]->GSSetShader(nullptr, nullptr, 0);
		myApp->deferredcontext[0]->PSSetShader(myApp->objectNormalMappingPS, nullptr, 0);
		myApp->deferredcontext[0]->VSSetShader(myApp->objectVS, nullptr, 0);

		myApp->deferredcontext[0]->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->StoShader[0], sizeof(DEMO_APP::SEND_TO_SCENE));
		myApp->deferredcontext[0]->Unmap(myApp->SceneconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->WtoShader[4], sizeof(DEMO_APP::SEND_TO_WORLD));
		myApp->deferredcontext[0]->Unmap(myApp->WorldconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->DirectionalLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->directionalLight[0], sizeof(DEMO_APP::SEND_DIRECTIONAL_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->DirectionalLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->PointLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->PointLightToS[0], sizeof(DEMO_APP::SEND_POINT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->PointLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->SpotLightconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->SpotLightToS, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->SpotLightconstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->instanceConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->instanceToShader, sizeof(DEMO_APP::SEND_TOINSTANCE));
		myApp->deferredcontext[0]->Unmap(myApp->instanceConstantBuffer, 0);

		myApp->deferredcontext[0]->Map(myApp->cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->myView, sizeof(DEMO_APP::SEND_SPOT_LIGHT));
		myApp->deferredcontext[0]->Unmap(myApp->cameraPositionBuffer, 0);

		sstride = sizeof(DEMO_APP::SimpleVertex);
		soffset = 0;

		myApp->deferredcontext[0]->IASetInputLayout(myApp->vertexLayout);

		myApp->deferredcontext[0]->IASetVertexBuffers(0, 1, &myApp->ObjectVertexbuffer[4], &sstride, &soffset);
		myApp->deferredcontext[0]->IASetIndexBuffer(myApp->ObjectIndexbuffer[4], DXGI_FORMAT_R32_UINT, 0);

		myApp->deferredcontext[0]->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		myApp->deferredcontext[0]->PSSetShaderResources(0, 1, &myApp->shaderResourceView[12]);
		myApp->deferredcontext[0]->PSSetShaderResources(1, 1, &myApp->shaderResourceView[13]);

		myApp->deferredcontext[0]->DrawIndexed(myApp->indexCount[4], 0, 0);

		myApp->WtoShader[4].World *= XMMatrixScaling(20.0f, 20.0f, 20.0f);
		myApp->WtoShader[8].World *= XMMatrixScaling(2.0f, 2.0f, 2.0f);
	}*/

	myApp->deferredcontext[0]->FinishCommandList(true, &myApp->commandList[0]);
}
