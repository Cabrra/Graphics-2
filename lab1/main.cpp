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
//#include <DirectXPackedVector.h>
//#include <DirectXCollision.h>

#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include "Sky_PS.csh"
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

#define BACKBUFFER_WIDTH	1280
#define BACKBUFFER_HEIGHT	768

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

	ID3D11VertexShader*				vertexShader;
	ID3D11PixelShader*				pixelShader;
	ID3D11VertexShader*				SkyvertexShader;
	ID3D11PixelShader*				SkypixelShader;

	ID3D11Texture2D*				zBuffer;
	ID3D11DepthStencilState *		stencilState;
	ID3D11DepthStencilView*			stencilView;
	ID3D11RasterizerState*			rasterState;
	ID3D11RasterizerState*			SkyrasterState;

	ID3D11SamplerState*				samplerState;
	ID3D11Buffer*					WorldconstantBuffer;
	ID3D11Buffer*					SceneconstantBuffer;

	ID3D11InputLayout*				vertexLayout;

	ID3D11Buffer*					SkyVertexbuffer;
	ID3D11Buffer*					SkyIndexbuffer;
	ID3D11Buffer*					GroundVertexbuffer;
	ID3D11Buffer*					GroundIndexbuffer;
	ID3D11Buffer*					ObjectVertexbuffer[6];
	ID3D11Buffer*					ObjectIndexbuffer[6];
	ID3D11ShaderResourceView*		shaderResourceView[6];
	ID3D11SamplerState*				CubesTexSamplerState;
	ID3D11DeviceContext*			deferredcontext;
	ID3D11CommandList*				commandList;

	XTime							time;
	XMMATRIX						rotation;
	XMMATRIX						Rotationx;
	XMMATRIX						Rotationy;
	XMMATRIX						Rotationz;
	float							dt;
	int								sphereIndex;
	int								groundIndex;

	//object loader

	int indexCount[6]; // one for each object type
	int objectcount; //keep tracjk of indexes

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;

	float camYaw = 0.0f;
	float camPitch = 0.0f;

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


	SEND_TO_WORLD					WtoShader[6];
	SEND_TO_WORLD					skytoShader;
	SEND_TO_SCENE					StoShader[2];
	SEND_DIRECTIONAL_LIGHT			directionalLight; //lights


public:
	friend void LoadingThread(DEMO_APP* myApp);
	friend void DrawingThread(DEMO_APP* myApp);
	friend void StatuesLoadingThread(DEMO_APP* myApp);
	friend void FolliageLoadingThread(DEMO_APP* myApp);
	friend void LeavesLoadingThread(DEMO_APP* myApp);
	friend void ObjectLoadingThread(DEMO_APP* myApp);

	struct SimpleVertex
	{
		XMFLOAT3 UV;
		XMFLOAT3 norm;
		XMFLOAT3 Pos;
	};
	
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	bool LoadObjectFromFile(string fileName);
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
    ZeroMemory( &wndClass, sizeof( wndClass ) );
    wndClass.cbSize         = sizeof( WNDCLASSEX );             
    wndClass.lpfnWndProc    = appWndProc;						
    wndClass.lpszClassName  = L"DirectXApplication";            
	wndClass.hInstance      = application;		               
    wndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );    
    wndClass.hbrBackground  = ( HBRUSH )( COLOR_WINDOWFRAME ); 

    RegisterClassEx( &wndClass );

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(	L"DirectXApplication", L"Lab 1a Line Land",	WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME|WS_MAXIMIZEBOX), 
							CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,					
							NULL, NULL,	application, this );												

    ShowWindow( window, SW_SHOW );
	//********************* END WARNING ************************//
		
	objectcount = 0;

	DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width =	BACKBUFFER_WIDTH;
    sd.BufferDesc.Height =	BACKBUFFER_HEIGHT;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window;
	sd.Windowed = false;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, 
		nullptr, 0, D3D11_SDK_VERSION, &sd, &swapchain, &device, nullptr, &inmediateContext); 
	 ID3D11Texture2D* backBuffer = nullptr;
    hr = swapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &backBuffer ) );
	
    hr = device->CreateRenderTargetView( backBuffer, nullptr, &renderTargetView );
    backBuffer->Release();

	std::thread myLoadingThread = std::thread(LoadingThread, this);
	//std::thread myStatLoadingThread = std::thread(StatuesLoadingThread, this);
	std::thread myFollLoadingThread = std::thread(FolliageLoadingThread, this);
	std::thread myLeavLoadingThread = std::thread(LeavesLoadingThread, this);

	viewport = new D3D11_VIEWPORT;
    viewport->Width =	(FLOAT)sd.BufferDesc.Width;
    viewport->Height =	(FLOAT)sd.BufferDesc.Height;
    viewport->MinDepth = 0.0f;
	viewport->MaxDepth = 1.0f;
    viewport->TopLeftX = 0;
    viewport->TopLeftY = 0;

	hr = device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), nullptr, &vertexShader);
	hr = device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), nullptr, &pixelShader);
	hr = device->CreateVertexShader(Sky_VS, sizeof(Sky_VS), nullptr, &SkyvertexShader);
	hr = device->CreatePixelShader(Sky_PS, sizeof(Sky_PS), nullptr, &SkypixelShader);

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

	//STELCIL
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateDepthStencilState(&dsDesc, &stencilState);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;

	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(zBuffer, NULL, &stencilView);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//Ground
	std::vector<SimpleVertex> myground;

	XMFLOAT3 initial = XMFLOAT3(-150.0f, 0.0f, 150.0f);

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
	for (int z = 0; z<300; z++)
	{
		for (int x = 0; x<300; x++)
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

	//Create the Sample State
	hr = device->CreateSamplerState(&sampDesc, &CubesTexSamplerState);

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

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
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
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	hr = device->CreateRasterizerState(&rasterDesc, &rasterState);

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

	hr = device->CreateSamplerState(&sampd, &samplerState);

	hr = device->CreateDeferredContext(0, &deferredcontext);

	//D3D11_BUFFER_DESC Ibd;
	//ZeroMemory(&Ibd, sizeof(Ibd));
	//Ibd.Usage = D3D11_USAGE_DEFAULT;
	//Ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//Ibd.CPUAccessFlags = NULL;
	//Ibd.ByteWidth = sizeof(unsigned short) * 120;
	//Ibd.MiscFlags = 0; //unused
	//Ibd.StructureByteStride = sizeof(unsigned short);

	//D3D11_SUBRESOURCE_DATA indexInitData;
	//ZeroMemory(&indexInitData, sizeof(indexInitData));
	//indexInitData.pSysMem = &myIndex;
	//indexInitData.SysMemPitch = 0;
	//indexInitData.SysMemSlicePitch = 0;

	//hr = device->CreateBuffer(&Ibd, &indexInitData, &indexbuffer);
	StoShader[0].ViewM = XMMatrixIdentity();
	StoShader[0].ProjectM = XMMatrixPerspectiveFovLH(1.30899694f, BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT, 0.1f, 300.0f);
	StoShader[1].ViewM = XMMatrixIdentity();
	StoShader[1].ProjectM = XMMatrixPerspectiveFovLH(1.30899694f, BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT, 0.1f, 300.0f);


	XMFLOAT4 vec = XMFLOAT4(0.0f, 20.0f, -15.0f, 0.0f);
	camPosition = XMLoadFloat4(&vec);
	
	XMFLOAT4 vec2 = XMFLOAT4(0.0f, 15.0f, 0.0f, 0.0f);
	camTarget = XMLoadFloat4(&vec2);
	XMFLOAT4 vec3 = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	camUp = XMLoadFloat4(&vec3);

	StoShader[0].ViewM *= XMMatrixLookAtLH(camPosition, camTarget, camUp);
	CreateSphere(20, 20);

	skytoShader.World = XMMatrixIdentity();
	skytoShader.World *= XMMatrixScaling(100.0f, 100.0f, 100.0f);
	for (int i = 0; i < 6; i++)
		WtoShader[i].World = XMMatrixIdentity();

	//WtoShader[1].World = XMMatrixTranslation(15.0f, 0.0f, 15.0f);
	WtoShader[0].World = XMMatrixScaling(0.5f, 0.5f, 0.5f);

	//WtoShader[1].World *= XMMatrixScaling(0.5f, 0.5f, 0.5f);
	WtoShader[2].World *= XMMatrixScaling(0.05f, 0.05f, 0.05f);
	WAIT_FOR_THREAD(&myLoadingThread);
	WAIT_FOR_THREAD(&myLeavLoadingThread);
	WAIT_FOR_THREAD(&myFollLoadingThread);
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	time.Signal();
	dt = time.Delta();

	Input();

	if (shaderResourceView[0] && shaderResourceView[1])
	{
		inmediateContext->OMSetRenderTargets(1, &renderTargetView, stencilView);

		inmediateContext->RSSetViewports(1, viewport);

		inmediateContext->ClearRenderTargetView(renderTargetView, Colors::MidnightBlue);
		inmediateContext->VSSetConstantBuffers(0, 1, &WorldconstantBuffer);
		inmediateContext->VSSetConstantBuffers(1, 1, &SceneconstantBuffer);

		inmediateContext->OMSetDepthStencilState(stencilState, 0);
		inmediateContext->PSSetSamplers(0, 1, &samplerState);
		
		inmediateContext->RSSetState(SkyrasterState);
		inmediateContext->ClearDepthStencilView(stencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xFF);
		
		skytoShader.World = XMMatrixIdentity();
		XMMATRIX Scale = XMMatrixScaling(100.0f, 100.0f, 100.0f);
		XMMATRIX Translation = XMMatrixTranslation(XMVectorGetX(StoShader[0].ViewM.r[0]), XMVectorGetY(StoShader[0].ViewM.r[1]), XMVectorGetZ(StoShader[0].ViewM.r[2]));
		skytoShader.World = Scale * Translation;
		
		//infinite skybox
		XMMATRIX aux = StoShader[0].ViewM;
		aux.r[3].m128_f32[0] = aux.r[3].m128_f32[1] = aux.r[3].m128_f32[2] = 0.0f;
		StoShader[1].ViewM = aux;
		//skybox
		D3D11_MAPPED_SUBRESOURCE  Resource;
		
		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[1], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);
		
		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &skytoShader, sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);
		
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

		//ground

		inmediateContext->RSSetState(rasterState);

		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[1], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->PSSetShader(pixelShader, nullptr, 0);
		inmediateContext->VSSetShader(vertexShader, nullptr, 0);
		sstride = sizeof(SimpleVertex);
		soffset = 0;

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &GroundVertexbuffer, &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(GroundIndexbuffer, DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[1]);

		inmediateContext->DrawIndexed(groundIndex, 0, 0);
		//tree
		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[2], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->PSSetShader(pixelShader, nullptr, 0);
		inmediateContext->VSSetShader(vertexShader, nullptr, 0);
		sstride = sizeof(SimpleVertex);
		soffset = 0;

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[1], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[1], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[3]);

		inmediateContext->DrawIndexed(indexCount[1], 0, 0);

		//leaves
		inmediateContext->RSSetState(SkyrasterState);
		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[2], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->PSSetShader(pixelShader, nullptr, 0);
		inmediateContext->VSSetShader(vertexShader, nullptr, 0);
		sstride = sizeof(SimpleVertex);
		soffset = 0;

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[0], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[0], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[4]);

		inmediateContext->DrawIndexed(indexCount[0], 0, 0);

		//leave
		
		//object
		//monks
		/*inmediateContext->RSSetState(rasterState);

		inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		inmediateContext->Unmap(SceneconstantBuffer, 0);

		inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &WtoShader[0], sizeof(SEND_TO_WORLD));
		inmediateContext->Unmap(WorldconstantBuffer, 0);

		inmediateContext->PSSetShader(pixelShader, nullptr, 0);
		inmediateContext->VSSetShader(vertexShader, nullptr, 0);
		sstride = sizeof(SimpleVertex);
		soffset = 0;

		inmediateContext->IASetInputLayout(vertexLayout);

		inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[0], &sstride, &soffset);
		inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[0], DXGI_FORMAT_R32_UINT, 0);

		inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		inmediateContext->PSSetSamplers(0, 1, &CubesTexSamplerState);
		inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[2]);

		inmediateContext->DrawIndexed(indexCount[0], 0, 0);*/

		//Folliage

		//inmediateContext->ClearDepthStencilView(stencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xFF);
		//inmediateContext->Map(SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		//memcpy(Resource.pData, &StoShader[0], sizeof(SEND_TO_SCENE));
		//inmediateContext->Unmap(SceneconstantBuffer, 0);

		//inmediateContext->Map(WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		//memcpy(Resource.pData, &WtoShader[0], sizeof(SEND_TO_WORLD));
		//inmediateContext->Unmap(WorldconstantBuffer, 0);

		//inmediateContext->PSSetShader(pixelShader, nullptr, 0);
		//inmediateContext->VSSetShader(vertexShader, nullptr, 0);
		//sstride = sizeof(SimpleVertex);
		//soffset = 0;

		//inmediateContext->IASetInputLayout(vertexLayout);

		//inmediateContext->IASetVertexBuffers(0, 1, &ObjectVertexbuffer[1], &sstride, &soffset);
		//inmediateContext->IASetIndexBuffer(ObjectIndexbuffer[1], DXGI_FORMAT_R32_UINT, 0);

		//inmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//inmediateContext->PSSetSamplers(0, 1, &CubesTexSamplerState);
		//inmediateContext->PSSetShaderResources(0, 1, &shaderResourceView[1]);

		//inmediateContext->DrawIndexed(indexCount[1], 0, 0);
	}

	/*WAIT_FOR_THREAD(&myDrawingThread);
	if (commandList)
		inmediateContext->ExecuteCommandList(commandList, true);
	SAFE_RELEASE(commandList);*/

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

	//SAFE_RELEASE(viewport);
	SAFE_DELETE(viewport);

	SAFE_RELEASE(vertexShader);
	SAFE_DELETE(vertexShader);

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
	
	for (int i = 0; i < 5; i++)
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

	for (int i = 0; i < 2; i++)
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

	SAFE_RELEASE(deferredcontext);
	SAFE_DELETE(deferredcontext);

	SAFE_RELEASE(SkyrasterState);
	SAFE_DELETE(SkyrasterState);

	SAFE_RELEASE(GroundVertexbuffer);
	SAFE_DELETE(GroundVertexbuffer);

	SAFE_RELEASE(GroundIndexbuffer);
	SAFE_DELETE(GroundIndexbuffer);

	UnregisterClass(L"DirectXApplication", application); 
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!
	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance,(WNDPROC)WndProc);	
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	myApp.ShutDown(); 
	return 0; 
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
//********************* END WARNING ************************//

bool DEMO_APP::LoadObjectFromFile(string fileName)
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
	device->CreateBuffer(&indexBufferDesc, &iinitData, &ObjectIndexbuffer[objectcount]);

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
	hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &ObjectVertexbuffer[objectcount]);

	indexCount[objectcount] = meshTriangles * 3;
	objectcount++;

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
		mat = XMMatrixRotationAxis(aux, -1.0f *dt)* mat;
	}
	else if (GetAsyncKeyState('C')) //left
	{
		XMFLOAT3 rot = XMFLOAT3(0.0f, 2.0f, 0.0f);
		XMVECTOR aux = XMLoadFloat3(&rot);
		mat = mat * XMMatrixRotationAxis(aux, -1.0f *dt);
	}
	else if (GetAsyncKeyState('V')) //right
	{
		XMFLOAT3 rot = XMFLOAT3(0.0f, 2.0f, 0.0f);
		XMVECTOR aux = XMLoadFloat3(&rot);
		mat = mat * XMMatrixRotationAxis(aux, 1.0f *dt);
	}

	StoShader[0].ViewM = XMMatrixInverse(nullptr, mat);
}

void LoadingThread(DEMO_APP* myApp)
{
	HRESULT hrT;

	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/Skybox.dds", nullptr, &myApp->shaderResourceView[0]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/woods.dds", nullptr, &myApp->shaderResourceView[1]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/StoneWall.dds", nullptr, &myApp->shaderResourceView[2]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/conifer_tronco.dds", nullptr, &myApp->shaderResourceView[3]);
	hrT = CreateDDSTextureFromFile(myApp->device, L"Assets/Textures/conifer_leaves.dds", nullptr, &myApp->shaderResourceView[4]);

}

void StatuesLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//monkstatue.obj");
}

void ObjectLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//LowPoly_Male_MKC_3D_ARTS.obj");
}

void FolliageLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//conifer.obj");
}

void LeavesLoadingThread(DEMO_APP* myApp)
{
	myApp->LoadObjectFromFile("Assets//Models//leaves.obj");
}

void DrawingThread(DEMO_APP* myApp)
{
	if (myApp->shaderResourceView[0] && myApp->shaderResourceView[1])
	{
		myApp->deferredcontext->OMSetRenderTargets(1, &myApp->renderTargetView, myApp->stencilView);
		myApp->deferredcontext->OMSetDepthStencilState(myApp->stencilState, 1);
		myApp->deferredcontext->RSSetViewports(1, myApp->viewport);
		myApp->deferredcontext->ClearRenderTargetView(myApp->renderTargetView, Colors::MidnightBlue);
		myApp->deferredcontext->ClearDepthStencilView(myApp->stencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xFF);

		myApp->deferredcontext->VSSetConstantBuffers(0, 1, &myApp->WorldconstantBuffer);
		myApp->deferredcontext->VSSetConstantBuffers(1, 1, &myApp->SceneconstantBuffer);

		D3D11_MAPPED_SUBRESOURCE  Resource;

		myApp->deferredcontext->Map(myApp->WorldconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->WtoShader[0], sizeof(DEMO_APP::SEND_TO_WORLD));
		myApp->deferredcontext->Unmap(myApp->WorldconstantBuffer, 0);

		myApp->deferredcontext->Map(myApp->SceneconstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		memcpy(Resource.pData, &myApp->StoShader[0], sizeof(DEMO_APP::SEND_TO_SCENE));
		myApp->deferredcontext->Unmap(myApp->SceneconstantBuffer, 0);

		UINT sstride = sizeof(DEMO_APP::SimpleVertex);
		UINT soffset = 0;
		myApp->deferredcontext->IASetVertexBuffers(0, 1, &myApp->SkyVertexbuffer, &sstride, &soffset);
		myApp->deferredcontext->IASetIndexBuffer(myApp->SkyIndexbuffer, DXGI_FORMAT_R16_UINT, 0);

		myApp->deferredcontext->IASetInputLayout(myApp->vertexLayout);
		myApp->deferredcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		myApp->deferredcontext->PSSetShader(myApp->pixelShader, nullptr, 0);
		myApp->deferredcontext->VSSetShader(myApp->vertexShader, nullptr, 0);

		myApp->deferredcontext->PSSetSamplers(0, 1, &myApp->samplerState);
		myApp->deferredcontext->PSSetShaderResources(0, 1, &myApp->shaderResourceView[0]);

		myApp->deferredcontext->DrawIndexed(36, 0, 0);

		myApp->deferredcontext->FinishCommandList(true, &myApp->commandList);

	}
}