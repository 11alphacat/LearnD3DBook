//***************************************************************************************
// BoxApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//
// Shows how to draw a box in Direct3D 12.
//
// Controls:
//   Hold the left mouse button down and move the mouse to rotate.
//   Hold the right mouse button down and move the mouse to zoom in and out.
//***************************************************************************************

#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"

#include <Windows.h>
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
    //XMCOLOR Color;
};

// 常量对象结构体
// 绘制物体所用对象的常量数据
struct ObjectConstants
{
    XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();   // 初始化为 4x4 的单位矩阵
    float Time;
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
    BoxApp(const BoxApp& rhs) = delete;
    BoxApp& operator=(const BoxApp& rhs) = delete;
	~BoxApp();

	virtual bool Initialize()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void BuildDescriptorHeaps();
	void BuildConstantBuffers();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildBoxGeometry();
    void BuildPSO();

private:
    
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

    std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    ComPtr<ID3D12PipelineState> mPSO = nullptr;

    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f*XM_PI;
    float mPhi = XM_PIDIV4;
    float mRadius = 4.0f;   // 原版为 5

    POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    try
    {
        BoxApp theApp(hInstance);
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

BoxApp::BoxApp(HINSTANCE hInstance)
: D3DApp(hInstance) 
{
}

BoxApp::~BoxApp()
{
}

bool BoxApp::Initialize()
{
    if(!D3DApp::Initialize())
		return false;
		
    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
 
    // 创建描述符堆
    BuildDescriptorHeaps();
    // 创建常量缓冲区
	BuildConstantBuffers();
    // 创建根签名（若Shader是一个函数，输入资源为函数参数，则根签名定义了函数签名）
    BuildRootSignature();
    // 构建着色器和输入布局
    BuildShadersAndInputLayout();
    // 构建box几何形体
    BuildBoxGeometry();
    // 构建流水线状态对象
    BuildPSO();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

// 每一帧都要调用Update函数
void BoxApp::Update(const GameTimer& gt)
{
    // Convert Spherical to Cartesian coordinates.
    // 由球面坐标转为笛卡尔坐标
    float x = mRadius*sinf(mPhi)*cosf(mTheta); // x = r * sinφ * cosθ
    float z = mRadius*sinf(mPhi)*sinf(mTheta); // z = r * sinφ * sinθ
    float y = mRadius*cosf(mPhi);              // y = r * cosφ

    /*
    *                    
                        Y    
                        ^   Z  
                        |   ^
                        |  /
                        | /
                        |/
            - - - - - - - - - - - -> X
                       /|
                      / |
                     /  |
                        |
    */

    // Build the view matrix.
    // 构建观察矩阵 
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);          // 虚拟摄像机坐标
    XMVECTOR target = XMVectorZero();                   // 观测世界空间的原点
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  // 向上的向量为 (0,1,0)

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, view);

    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    //XMMATRIX worldViewProj = world*view*proj;   // 世界-观察-投影

    // 绕 Y 轴旋转矩阵
static float angle = 0.00f;   // not a good idea, use GameTimer may be better
    
    XMMATRIX rotationY = XMMatrixRotationY(angle);
    angle = (angle > 180.0f) ? 0.0f : angle + 0.01f;

    XMMATRIX worldViewProj = world * rotationY * view * proj;   // 世界-观察-投影

	// Update the constant buffer with the latest worldViewProj matrix.
    // 用最新的 worldViewProj 矩阵来更新常量区
	ObjectConstants objConstants;
    XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
    objConstants.Time = gt.TotalTime();

    mObjectCB->CopyData(0, objConstants);
}

void BoxApp::Draw(const GameTimer& gt)
{
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

    /*
        设置渲染目标的视口（Viewports）信息
        视口定义了渲染目标上的一个矩形区域，用于指定将 3D 场景投影到 2D 屏幕上的具体区域。
        视口通常由以下几个属性定义：
        1.位置和大小：通过指定视口在渲染目标上的左上角坐标（TopLeftX、TopLeftY）
        以及宽度（Width）和高度（Height）来确定视口的矩形区域。
        2.最小深度和最大深度：使用MinDepth和MaxDepth来定义视口的深度范围。深度值通常用于表示物体在场景中的前后关系，范围通常在 0.0 到 1.0 之间
    
        用途：可以设置多个视口，实现同时在不同区域进行独立的渲染、
            可以使用多个视口实现分屏多人游戏模式
    */         
    mCommandList->RSSetViewports(1, &mScreenViewport);
    
    /*
        设置渲染目标的裁剪矩形（Scissor Rectangles）信息

        裁剪矩形定义了渲染目标上的一个矩形区域（D3D12_RECT），
        在这个区域内的像素将被渲染，而区域外的像素将被忽略

        用途：优化性能，避免对屏幕上不需要渲染的区域进行不必要的计算和绘制
            实现特定的渲染效果，如分屏显示、窗口特效等。
    */
    mCommandList->RSSetScissorRects(1, &mScissorRect); 

    // Indicate a state transition on the resource usage.
    // 将当前后缓冲区的状态从呈现状态转换为渲染目标状态
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    /*
        清除渲染目标视图（Render Target View）的特定颜色

        用途：1.初始化渲染目标为一个已知的值
            在每一帧开始渲染之前，通常需要清除渲染目标，以确保上一帧的内容不会影响当前帧的渲染结果
            2.实现特定的视觉效果：可以使用不同的颜色值来清除渲染目标，以实现特定的视觉效果
            3.部分清除：通过指定矩形区域，可以选择性地清除渲染目标的一部分
    */
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    
    /*
        清除深度 / 模板视图（Depth-Stencil View)

        用途：1.初始化深度 / 模板缓冲区：在每一帧开始渲染之前，通常需要清除深度 / 模板缓冲区，
                以确保上一帧的深度和模板信息不会影响当前帧的渲染结果
            2.设置特定的深度和模板值：可以使用特定的深度值和模板值来清除深度 / 模板缓冲区，
               以实现特定的渲染效果
    */
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
    // Specify the buffers we are going to render to.
    /*
        设置输出合并阶段（Output Merger Stage）的渲染目标

        用途：1.指定渲染目标：该函数允许你指定一个或多个渲染目标，
            以便在图形渲染管线的输出合并阶段将像素数据写入这些目标
            2.深度 / 模板视图设置：除了设置渲染目标，你还可以通过该函数指定一个深度 / 模板视图
            
         此处将rtv和dsv设置为图形渲染管线的输出目标
    */
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    /*
        设置图形命令列表使用的描述符堆（Descriptor Heaps）

        描述符堆是 DirectX 12 中用于存储描述符（Descriptors）的资源。
        描述符是一种数据结构，用于描述资源（如纹理、缓冲区等）在图形管线中的使用方式，按类型可分为
           cbv（常量缓冲区）、srv（着色器资源）、uav（无序访问）

        用途：1.资源绑定：通过设置描述符堆，图形命令列表可以访问和绑定各种资源，
            如纹理、缓冲区、采样器等。描述符堆中的描述符提供了资源的地址和属性信息，
            使得图形管线能够正确地访问和使用这些资源。

            2.提高性能：使用描述符堆可以提高资源绑定的效率。
            在 DirectX 12 中，资源绑定是一个相对昂贵的操作，
            通过将多个资源的描述符存储在一个描述符堆中，并一次性设置描述符堆，
            可以减少资源绑定的开销，提高性能。

            3.动态资源更新：描述符堆可以在运行时动态更新。
            这意味着可以在图形命令列表执行过程中，根据需要更新描述符堆中的描述符，
            以实现动态资源绑定和更新。例如，可以在每一帧中更新纹理描述符，以实现动画效果。
    */
	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);


    /*
        设置图形命令列表的根签名（Root Signature）
    
        根签名定义了图形管线可以访问的资源，如常量缓冲区、纹理、采样器等。
        它描述了资源在内存中的布局和访问方式，以及哪些着色器阶段可以访问这些资源。
        根签名是 DirectX 12 中资源绑定的重要组成部分，它允许开发者\
        在不同的渲染场景中灵活地配置资源的访问方式。

        用途：1.资源绑定控制：通过设置根签名，图形命令列表可以确定哪些资源可以被图形管线访问。
            根签名定义了资源的类型、数量和在内存中的布局，使得着色器能够正确地访问所需的资源。
            
            2.提高性能和灵活性：合理设计根签名可以提高资源绑定的效率，减少不必要的资源绑定操作。
            同时，根签名可以根据不同的渲染场景进行动态切换，提供了更大的灵活性和性能优化空间。
    */
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    /*
        设置输入装配阶段（Input Assembler Stage）的顶点缓冲区（Vertex Buffers）

        用途：1.顶点数据输入；2.多顶点缓冲区支持；3.动态顶点数据更新
    */
	mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());

    /*
        设置输入装配阶段（Input Assembler Stage）的索引缓冲区（Index Buffer）

        用途：1.索引数据输入：索引缓冲区用于存储一组索引值，这些索引值用于指定顶点缓冲区中
        的顶点如何组成三角形或其他几何图形。
            2.减少顶点重复
    */
	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
    
    /*
        设置输入装配阶段（Input Assembler Stage）的图元拓扑（Primitive Topology）

        用途：1.定义几何图形类型;2.优化渲染性能：选择合适的图元拓扑可以提高渲染性能
            3.动态切换图元拓扑
    */
    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    //mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    
    /*
        设置图形命令列表的根签名中的描述符表（Descriptor Table）

        参数解释
RootParameterIndex：根签名中描述符表参数的索引。根签名由一系列根参数组成，每个根参数可以是常量缓冲区、描述符表或根常量等。
        这个参数指定要设置的描述符表对应的根参数的索引。
BaseDescriptor：描述符表的起始 GPU 描述符句柄。描述符表是一组描述符的集合，用于描述资源在图形管线中的使用方式。
        这个参数指定了描述符表的起始位置，图形管线将从这个位置开始访问描述符表中的描述符。
    */
    mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

    /*
        绘制带有索引的几何图形的多个实例。
        参数解释可查询Doubao

        用途:1.实例化渲染，可高效地绘制多个相同的几何图形实例，例如树木、建筑；
            2.减少内存开销和处理开销，只需要存储一份几何图形数据（顶点和索引缓冲区）
             只需要对几何图形进行一次顶点和像素处理，然后通过实例化参数（如变换矩阵、颜色等）来区分不同的实例
    */
    mCommandList->DrawIndexedInstanced(
		mBoxGeo->DrawArgs["box"].IndexCount,    // 每个实例要绘制的索引数量
		1, 0, 0, 0);
	
    // Indicate a state transition on the resource usage.
    // 从渲染目标状态转换为呈现状态
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
	ThrowIfFailed(mCommandList->Close());
 
    // Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));   // 在这一步看到了渲染结果
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        // 根据鼠标的移动距离计算旋转角度，令每个像素按此角度的 1/4 进行旋转（这里是假定半径为 4 ？ 因为 dx = r * dθ）
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        // 更新相机角度
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        // 限制角度范围为 0.1 ~ pi-0.1 (单位：弧度)
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        // 使场景中的每个像素按鼠标移动距离的0.005倍进行缩放
        float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void BoxApp::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&mCbvHeap)));
}

void BoxApp::BuildConstantBuffers()
{
    // 此常量缓冲区存储了绘制 1 个物体所需的常量数据
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    // 缓冲区的起始地址
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();

    // Offset to the ith object constant buffer in the buffer.
    // 偏移到常量缓冲区中绘制第 i 个物体所需的常量数据
    int boxCBufIndex = 0;
	cbAddress += boxCBufIndex*objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    // 创建缓冲区常量视图
	md3dDevice->CreateConstantBufferView(
		&cbvDesc,
		mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
    // 根参数可以是描述符表、根描述符或根常量
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
    // 创建一个只存有一个 CBV 的根描述符表
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        1,              // 表中的描述符数量
        0);             // 将这段描述符区域绑定至此基准着色器描述符（base shader register）

	slotRootParameter[0].InitAsDescriptorTable(
        1,              // 描述符区域的数量 
        &cbvTable);     // 指向描述符表的指针

	// A root signature is an array of root parameters.
    // 根签名由一组根参数构成
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	// 创建仅含一个槽位的根签名
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void BoxApp::BuildShadersAndInputLayout()
{
    HRESULT hr = S_OK;
    
	mvsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void BoxApp::BuildBoxGeometry()
{
    // 创建存有立方体 8 个顶点的默认缓冲区，并为每个顶点赋予不同的颜色
    
    std::array<Vertex, 8> vertices =
    {
        Vertex({ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT4(Colors::Magenta) })
    };

    // 顶点索引数组（以默认时针绘制三角形）
	std::array<std::uint16_t, 36> indices =
	{
		// front face
        // 立方体前表面三角形
		0, 1, 2,
		0, 2, 3,

		// back face
		// 后表面
        4, 6, 5,
		4, 7, 6,

		// left face
        // 左表面
		4, 5, 1,
		4, 1, 0,

		// right face
        // 右表面
		3, 2, 6,
		3, 6, 7,

		// top face
        // 上表面
		1, 5, 6,
		1, 6, 2,

		// bottom face
        // 下表面
		4, 0, 3,
		4, 3, 7
	};

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    // 使用辅助函数来创建默认缓冲区
	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;
}

void BoxApp::BuildPSO()
{
    /*
        流水线状态对象(Pipeline State Object)定义了图形渲染管线的各个阶段的状态
    
        包括顶点输入布局、顶点着色器、像素着色器、几何着色器（可选）、
        hull 着色器（可选）、domain 着色器（可选）、光栅化状态、混合状态、深度模板状态等。
    
        通过设置不同的流水线状态对象，可以快速切换不同的渲染设置，提高渲染效率。
    
    */

    // 填写结构体实例
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    psoDesc.pRootSignature = mRootSignature.Get();
    psoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()), 
		mvsByteCode->GetBufferSize() 
	};
    psoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()), 
		mpsByteCode->GetBufferSize() 
	};
    //psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    D3D12_RASTERIZER_DESC rsDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    //rsDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;    // 线框模式渲染立方体
    ////rsDesc.CullMode = D3D12_CULL_MODE_NONE;         // 禁用背面剔除
    //rsDesc.CullMode = D3D12_CULL_MODE_FRONT;         // 正面剔除

    psoDesc.RasterizerState = rsDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;     // 图元拓扑类型
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = mDepthStencilFormat;

    // 创建对象
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}