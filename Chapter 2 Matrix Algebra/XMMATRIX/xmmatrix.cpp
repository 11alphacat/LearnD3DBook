#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>
#include <string>
#include "practice.h"
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// Overload the  "<<" operators so that we can use cout to 
// output XMVECTOR and XMMATRIX objects.
ostream& XM_CALLCONV operator << (ostream& os, FXMVECTOR v)
{
    XMFLOAT4 dest;
    XMStoreFloat4(&dest, v);

    os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ", " << dest.w << ")";
    return os;
}

ostream& XM_CALLCONV operator << (ostream& os, FXMMATRIX m)
{
    for (int i = 0; i < 4; ++i)
    {
        os << XMVectorGetX(m.r[i]) << "\t";
        os << XMVectorGetY(m.r[i]) << "\t";
        os << XMVectorGetZ(m.r[i]) << "\t";
        os << XMVectorGetW(m.r[i]) << '\n';
    }
    return os;
}

#include <chrono>
namespace {
    using func_t = void (*)();
    // 函数执行时间检测函数
    double measureExecutionTime(func_t f, int numExecutions)
    {
        double totalTime = 0.0; // 总时间

        for (int i = 0; i < numExecutions; ++i)
        {
            // 获取当前时间点
            auto startTime = std::chrono::high_resolution_clock::now();

            // 执行函数
            f();

            // 获取当前时间点
            auto endTime = std::chrono::high_resolution_clock::now();

            // 计算函数执行时间（单位：毫秒）
            std::chrono::duration<double, std::milli> duration = endTime - startTime;
            double executionTime = duration.count();

            totalTime += executionTime;
        }

        // 计算平均时间
        //double averageTime = totalTime / numExecutions;

        return totalTime / 1000;
    }
}

namespace {
    XMMATRIX _A(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f,
        1.0f, 2.0f, 3.0f, 1.0f);
    void myFun() {
        p19::_test(_A); // complete by chat-gpt
    }

    void officialFun() {
        XMVECTOR det = XMMatrixDeterminant(_A);
        XMMATRIX E = XMMatrixInverse(&det, _A);
    }
}

int main()
{
    // Check support for SSE2 (Pentium4, AMD K8, and above).
    if (!XMVerifyCPUSupport())
    {
        cout << "directx math not supported" << endl;
        return 0;
    }

    XMMATRIX A(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f,
        1.0f, 2.0f, 3.0f, 1.0f);

    XMMATRIX B = XMMatrixIdentity();

    XMMATRIX C = A * B;

    XMMATRIX D = XMMatrixTranspose(A);

    XMVECTOR det = XMMatrixDeterminant(A);
    float _det = XMVectorGetX(det);
    XMMATRIX E = XMMatrixInverse(&det, A);
    XMMATRIX F = A * E;

    cout << "A = " << endl << A << endl;
    cout << "B = " << endl << B << endl;
    cout << "C = A*B = " << endl << C << endl;
    cout << "D = transpose(A) = " << endl << D << endl;
    cout << "det = determinant(A) = " << _det << endl << endl;
    cout << "E = inverse(A) = " << endl << E << endl;
    p19::hello();
    p19::test(A);
    cout << "F = A*E = " << endl << F << endl;

    cout << std::string(5, '=') << "Profiling Start" << std::string(5, '=') << '\n';
    constexpr int c_repeat = (int)1e7;
    cout << "Time1 = " << measureExecutionTime(myFun, c_repeat) << " s" << '\n';
    cout << "Time2 = " << measureExecutionTime(officialFun, c_repeat) << " s" << endl;
    cout << std::string(5, '=') << "Profiling End" << std::string(5, '=') << endl;

    return 0;
}


