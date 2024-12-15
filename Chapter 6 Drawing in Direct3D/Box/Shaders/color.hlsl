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
	float4x4 gWorldViewProj0;	// 4x4 矩阵
	float4x4 gWorldViewProj1;	// 4x4 矩阵
	
    float gTime;
	
	// 这里手动显示填充一下
//    float4x4 _Pad0;
//    float4x4 _Pad1;
//    float4x4 _Pad2;

	
};

struct VertexIn
{
    uint Index	 : INDEX;
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
	
	// 使几个体的形状随正弦函数周期性发生变化
    vin.PosL.xy += 0.5 * sin(vin.PosL.x) * sin(3.0f * gTime);
    vin.PosL.z *= 0.6f + 0.4 * sin(2.0f * gTime);
	
	// Transform to homogeneous clip space.
	// 把顶点 变换到齐次裁剪空间
	// gWorldViewProj 为 4x4 矩阵
	// 1.0f 表示点， 0表示向量（齐次坐标）
    if (vin.Index == 0)
    {
		vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj0);
    }
    else
    {
		vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj1);
    }
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


