#pragma once
#include <iostream>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

// book practice: chapter 2, practice 19
namespace p19 {
#include "plugin_xmdef.h"
    constexpr float epison = 1e-6;
    template<typename T> inline T _abs(T number) { return number > 0 ? number : -1 * number; }

    struct Matrix4x4
    {
        union {
            struct {
                float _a00, _a01, _a02, _a03;
                float _a10, _a11, _a12, _a13;
                float _a20, _a21, _a22, _a23;
                float _a30, _a31, _a32, _a33;
            };
            float array[4][4];
        };

        float operator() (size_t i, size_t j) const { return array[i][j]; }
        Matrix4x4() {}
        Matrix4x4(float a00, float a01, float a02, float a03,
            float a10, float a11, float a12, float a13,
            float a20, float a21, float a22, float a23,
            float a30, float a31, float a32, float a33)
            : _a00(a00), _a01(a01), _a02(a02), _a03(a03),
            _a10(a10), _a11(a11), _a12(a12), _a13(a13),
            _a20(a20), _a21(a21), _a22(a22), _a23(a23),
            _a30(a30), _a31(a31), _a32(a32), _a33(a33)
        {}
        explicit Matrix4x4(const CXMMATRIX m) : Matrix4x4() {
            for (int i = 0; i != 4; ++i) {
                array[i][0] = XMVectorGetX(m.r[i]);
                array[i][1] = XMVectorGetY(m.r[i]);
                array[i][2] = XMVectorGetZ(m.r[i]);
                array[i][3] = XMVectorGetW(m.r[i]);
            }
        }
    };

    float MatrixDeterminant(const Matrix4x4& m) {
        float det = 0.0f;
        det += m._a00 * (m._a11 * (m._a22 * m._a33 - m._a32 * m._a23) -
            m._a12 * (m._a21 * m._a33 - m._a31 * m._a23) +
            m._a13 * (m._a21 * m._a32 - m._a31 * m._a22));
        det -= m._a01 * (m._a10 * (m._a22 * m._a33 - m._a32 * m._a23) -
            m._a12 * (m._a20 * m._a33 - m._a30 * m._a23) +
            m._a13 * (m._a20 * m._a32 - m._a30 * m._a22));
        det += m._a02 * (m._a10 * (m._a21 * m._a33 - m._a31 * m._a23) -
            m._a11 * (m._a20 * m._a33 - m._a30 * m._a23) +
            m._a13 * (m._a20 * m._a31 - m._a30 * m._a21));
        det -= m._a03 * (m._a10 * (m._a21 * m._a32 - m._a31 * m._a22) -
            m._a11 * (m._a20 * m._a32 - m._a30 * m._a22) +
            m._a12 * (m._a20 * m._a31 - m._a30 * m._a21));

        return det;
    }

    // ¼ÆËã°éËæ¾ØÕó
    // get adjoint matrix
    Matrix4x4 CalculateAdjoint(const Matrix4x4& matrix)
    {
        Matrix4x4 adjoint;
        adjoint._a00 = matrix._a11 * (matrix._a22 * matrix._a33 - matrix._a32 * matrix._a23) -
            matrix._a12 * (matrix._a21 * matrix._a33 - matrix._a31 * matrix._a23) +
            matrix._a13 * (matrix._a21 * matrix._a32 - matrix._a31 * matrix._a22);

        adjoint._a01 = -matrix._a01 * (matrix._a22 * matrix._a33 - matrix._a32 * matrix._a23) +
            matrix._a02 * (matrix._a21 * matrix._a33 - matrix._a31 * matrix._a23) -
            matrix._a03 * (matrix._a21 * matrix._a32 - matrix._a31 * matrix._a22);

        adjoint._a02 = matrix._a01 * (matrix._a12 * matrix._a33 - matrix._a32 * matrix._a13) -
            matrix._a02 * (matrix._a11 * matrix._a33 - matrix._a31 * matrix._a13) +
            matrix._a03 * (matrix._a11 * matrix._a32 - matrix._a31 * matrix._a12);

        adjoint._a03 = -matrix._a01 * (matrix._a12 * matrix._a23 - matrix._a22 * matrix._a13) +
            matrix._a02 * (matrix._a11 * matrix._a23 - matrix._a21 * matrix._a13) -
            matrix._a03 * (matrix._a11 * matrix._a22 - matrix._a21 * matrix._a12);

        adjoint._a10 = -matrix._a10 * (matrix._a22 * matrix._a33 - matrix._a32 * matrix._a23) +
            matrix._a12 * (matrix._a20 * matrix._a33 - matrix._a30 * matrix._a23) -
            matrix._a13 * (matrix._a20 * matrix._a32 - matrix._a30 * matrix._a22);

        adjoint._a11 = matrix._a00 * (matrix._a22 * matrix._a33 - matrix._a32 * matrix._a23) -
            matrix._a02 * (matrix._a20 * matrix._a33 - matrix._a30 * matrix._a23) +
            matrix._a03 * (matrix._a20 * matrix._a32 - matrix._a30 * matrix._a22);

        adjoint._a12 = -matrix._a00 * (matrix._a12 * matrix._a33 - matrix._a32 * matrix._a13) +
            matrix._a02 * (matrix._a10 * matrix._a33 - matrix._a30 * matrix._a13) -
            matrix._a03 * (matrix._a10 * matrix._a32 - matrix._a30 * matrix._a12);

        adjoint._a13 = matrix._a00 * (matrix._a12 * matrix._a23 - matrix._a22 * matrix._a13) -
            matrix._a02 * (matrix._a10 * matrix._a23 - matrix._a20 * matrix._a13) +
            matrix._a03 * (matrix._a10 * matrix._a22 - matrix._a20 * matrix._a12);

        adjoint._a20 = matrix._a10 * (matrix._a21 * matrix._a33 - matrix._a31 * matrix._a23) -
            matrix._a11 * (matrix._a20 * matrix._a33 - matrix._a30 * matrix._a23) +
            matrix._a13 * (matrix._a20 * matrix._a31 - matrix._a30 * matrix._a21);

        adjoint._a21 = -matrix._a00 * (matrix._a21 * matrix._a33 - matrix._a31 * matrix._a23) +
            matrix._a01 * (matrix._a20 * matrix._a33 - matrix._a30 * matrix._a23) -
            matrix._a03 * (matrix._a20 * matrix._a31 - matrix._a30 * matrix._a21);

        adjoint._a22 = matrix._a00 * (matrix._a11 * matrix._a33 - matrix._a31 * matrix._a13) -
            matrix._a01 * (matrix._a10 * matrix._a33 - matrix._a30 * matrix._a13) +
            matrix._a03 * (matrix._a10 * matrix._a31 - matrix._a30 * matrix._a11);

        adjoint._a23 = -matrix._a00 * (matrix._a11 * matrix._a23 - matrix._a21 * matrix._a13) +
            matrix._a01 * (matrix._a10 * matrix._a23 - matrix._a20 * matrix._a13) -
            matrix._a03 * (matrix._a10 * matrix._a21 - matrix._a20 * matrix._a11);

        adjoint._a30 = -matrix._a10 * (matrix._a21 * matrix._a32 - matrix._a31 * matrix._a22) +
            matrix._a11 * (matrix._a20 * matrix._a32 - matrix._a30 * matrix._a22) -
            matrix._a12 * (matrix._a20 * matrix._a31 - matrix._a30 * matrix._a21);

        adjoint._a31 = matrix._a00 * (matrix._a21 * matrix._a32 - matrix._a31 * matrix._a22) -
            matrix._a01 * (matrix._a20 * matrix._a32 - matrix._a30 * matrix._a22) +
            matrix._a02 * (matrix._a20 * matrix._a31 - matrix._a30 * matrix._a21);

        adjoint._a32 = -matrix._a00 * (matrix._a11 * matrix._a32 - matrix._a31 * matrix._a12) +
            matrix._a01 * (matrix._a10 * matrix._a32 - matrix._a30 * matrix._a12) -
            matrix._a02 * (matrix._a10 * matrix._a31 - matrix._a30 * matrix._a11);

        adjoint._a33 = matrix._a00 * (matrix._a11 * matrix._a22 - matrix._a21 * matrix._a12) -
            matrix._a01 * (matrix._a10 * matrix._a22 - matrix._a20 * matrix._a12) +
            matrix._a02 * (matrix._a10 * matrix._a21 - matrix._a20 * matrix._a11);

        return adjoint;
    }

    // ¼ÆËãÄæ¾ØÕó
    // get inverse matrix
    Matrix4x4 CalculateInverse(const Matrix4x4& matrix)
    {
        float determinant = MatrixDeterminant(matrix);
        if (_abs(determinant) < epison)
        {
            std::cout << "Matrix is not invertible." << std::endl;
            return matrix;
        }

        float invDet = 1.0f / determinant;
        Matrix4x4 inverse = CalculateAdjoint(matrix);
        inverse._a00 *= invDet;
        inverse._a01 *= invDet;
        inverse._a02 *= invDet;
        inverse._a03 *= invDet;
        inverse._a10 *= invDet;
        inverse._a11 *= invDet;
        inverse._a12 *= invDet;
        inverse._a13 *= invDet;
        inverse._a20 *= invDet;
        inverse._a21 *= invDet;
        inverse._a22 *= invDet;
        inverse._a23 *= invDet;
        inverse._a30 *= invDet;
        inverse._a31 *= invDet;
        inverse._a32 *= invDet;
        inverse._a33 *= invDet;

        return inverse;
    }

}

namespace p19 {
#include "plugin_xmdef.h"
    // ÖØÔØ << ÒÔÊä³öÀàMatrix4x4
    // override << to output class Matrix4x4
    std::ostream& operator<< (std::ostream& os, Matrix4x4 m) {
        for (int i = 0; i != 4; ++i) {
            for (int j = 0; j != 4; ++j) {
                os << m(i, j) << '\t';
                if (j == 3) os << '\n';
            }
        }
        return os;
    }

    void XM_CALLCONV test(FXMMATRIX m) {
        Matrix4x4 m_inv = CalculateInverse(Matrix4x4(m));
        std::cout << "from test:" << '\n';
        std::cout << m_inv << std::endl;
    }

    void XM_CALLCONV _test(FXMMATRIX m) {
        Matrix4x4 m_inv = CalculateInverse(Matrix4x4(m));
    }

    void hello() {
        std::cout << "hello?" << std::endl;
    }
}