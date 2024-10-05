//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "d3dUtil.h"
#include "GameTimer.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

// 基础的Direct3D应用程序类     
class D3DApp
{
protected:

    D3DApp(HINSTANCE hInstance);
    
    // 唯一性，不允许赋值与拷贝构造
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;

    // 若会被派生，析构函数必须是虚函数
    virtual ~D3DApp();

public:

    static D3DApp* GetApp();
    
	HINSTANCE AppInst()const;   // 简单的存取函数
	HWND      MainWnd()const;   // 简单的存取函数
	float     AspectRatio()const; // 计算横纵比
        
    bool Get4xMsaaState()const;
    void Set4xMsaaState(bool value);

	int Run();
 

// ================================
// 六个框架方法，派生类需重写
    virtual bool Initialize();
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // 消息处理过程

protected:
    virtual void CreateRtvAndDsvDescriptorHeaps();
	virtual void OnResize();    // Windows消息队列接收到 WM_SIZE 时调用
	virtual void Update(const GameTimer& gt)=0; // 纯虚函数，绘制每一帧都要调用 \
          用来更新随时间的推移而变化的3D程序（呈现动画、移动摄像机、碰撞检测、检查用户输入）
    virtual void Draw(const GameTimer& gt)=0; // 纯虚函数，绘制每一帧都要调用 \
          在该方法中发出渲染命令，将当前帧真正地绘制到后台缓冲区 \
          完成帧的绘制后，再调用 IDXGISwapChain::Present方法将后台缓冲区的内容显示到屏幕

// =================================

	// Convenience overrides for handling mouse input.
    // 便于重写鼠标输入消息的处理流程
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	bool InitMainWindow();
	bool InitDirect3D();
    // 创建命令队列、命令列表分配器和命令列表
	void CreateCommandObjects(); 
    void CreateSwapChain();

    // 强制 CPU 等待 GPU处理完队列中的所有命令
	void FlushCommandQueue();   

	ID3D12Resource* CurrentBackBuffer()const;   // 返回交换链当前后台缓冲区的资源
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

	void CalculateFrameStats(); // 计算每秒的平均帧数以及每帧平均的毫秒时长

    void LogAdapters(); // 枚举系统中所有适配器
    void LogAdapterOutputs(IDXGIAdapter* adapter); // 枚举指定适配器的全部显示输出 
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format); // 枚举
                    // 某个显示输出对特定格式支持的所有显示模式

protected:

    static D3DApp* mApp;

    HINSTANCE mhAppInst = nullptr; // application instance handle 应用程序句柄
    HWND      mhMainWnd = nullptr; // main window handle 主窗口句柄
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?大小调整栏是否拖拽
    bool      mFullscreenState = false;// fullscreen enabled 是否开启全屏

	// Set true to use 4X MSAA (?.1.8).  The default is false.
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// 计时器
	GameTimer mTimer;
	
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;
	
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

    D3D12_VIEWPORT mScreenViewport; 
    D3D12_RECT mScissorRect;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	// Derived class should set these in derived constructor to customize starting values.
	// 用户应该在派生类的派生构造函数中自定义这些初始值
    std::wstring mMainWndCaption = L"d3d App";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 800;
	int mClientHeight = 600;
};

