#pragma once
// for verfifying
#include <iostream>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

namespace p19 {
#include "plugin_xmdef.h" 
    struct Matrix4x4;

    float MatrixDeterminant(const Matrix4x4& m);

    // 计算伴随矩阵
    Matrix4x4 CalculateAdjoint(const Matrix4x4& matrix);

    // 计算逆矩阵
    Matrix4x4 CalculateInverse(const Matrix4x4& matrix);

}

namespace p19 {
#include "plugin_xmdef.h"
    // 重载 << 以输出类Matrix4x4
    std::ostream& operator<< (std::ostream& os, Matrix4x4 m);

    void XM_CALLCONV test(FXMMATRIX m);
    void XM_CALLCONV _test(FXMMATRIX m);
    void hello();
}