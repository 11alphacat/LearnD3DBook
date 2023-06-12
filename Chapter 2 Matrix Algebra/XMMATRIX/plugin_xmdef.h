// DirectX12 常用类型与函数别名定义
// 'usings' for DirectX12 type and function

/*		
*		README
* 
*		You should add
* 
*		=> '#include <DirectXMath.h>' 
*			and 
*		=> '#include <DirectXPackedVector.h>'
* 
*		in your code.
* 
*		Then you can use 
*
*		=> '#include "plugin_xmdef.h"'
* 
*		in your own namespace.
*/


/*
	XM_CALLCONV		for fast parameters pass declaration
*/

/* Vector */

	// Calling convetion
using FXMVECTOR = DirectX::FXMVECTOR;	// 1, 2, 3
using GXMVECTOR = DirectX::GXMVECTOR;	// 4
using HXMVECTOR = DirectX::HXMVECTOR;	// 5, 6

	// for calculator
using XMVECTOR = DirectX::XMVECTOR;	// it's a typedef name of __m128

	// for data member in a structure or class
using XMFLOAT2 = DirectX::XMFLOAT2;
using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;

	// load from XMFLOATn to XMVECTOR
using DirectX::XMLoadFloat2;
using DirectX::XMLoadFloat3;
using DirectX::XMLoadFloat4;

	// store XMVECTOR to XMFLOATn
using DirectX::XMStoreFloat2;
using DirectX::XMStoreFloat3;
using DirectX::XMStoreFloat4;

using DirectX::XMVectorGetX;
using DirectX::XMVectorGetY;
using DirectX::XMVectorGetZ;
using DirectX::XMVectorGetW;


/* Matrix */
	//Calling convention
using FXMMATRIX = DirectX::FXMMATRIX;	// 	1
using CXMMATRIX = DirectX::CXMMATRIX;	// 	others

	// for calculator
using XMMATRIX = DirectX::XMMATRIX;

	// for data member in a structrue or class
using XMFLOAT4X4 = DirectX::XMFLOAT4X4;

	// load from XMFLOATnXn to XMMATRIX
using DirectX::XMLoadFloat4x4;

	// store XMVECTOR to XMFLOATnXn
using DirectX::XMStoreFloat4x4;