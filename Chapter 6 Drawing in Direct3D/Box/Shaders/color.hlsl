//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

// const buffer:常量缓冲区
// 会隐式填充为 256B
// 将常量缓冲区资源绑定到 b(常量缓冲区视图)的 0 号寄存器 
//		除了b 外， 还有 t（着色器资源视图）、s（采样器）、u（无序访问视图）
cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;	// 4x4 矩阵
	
	// 这里手动显示填充一下
    float4x4 _Pad0;
    float4x4 _Pad1;
    float4x4 _Pad2;

	
};

struct VertexIn
{
	float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;	// SV: system value
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;	// 待返回的点结构体
	
	// Transform to homogeneous clip space.
	// 把顶点 变换到齐次裁剪空间
	// gWorldViewProj 为 4x4 矩阵
	// 1.0f 表示点， 0表示向量（齐次坐标）
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


