#pragma once

#include <stdint.h>
#include <d3d11.h>

namespace DX11VTable
{
#if _WIN64
    typedef uint64_t tableEntry;
#else
    typedef uint32_t tableEntry;
#endif

    // Base ID3D11Device vtable - 43 entries
    struct ID3D11DeviceVTable
    {
        tableEntry QueryInterface;       // 0
        tableEntry AddRef;               // 1
        tableEntry Release;              // 2
        tableEntry CreateBuffer;         // 3
        tableEntry CreateTexture1D;      // 4
        tableEntry CreateTexture2D;      // 5
        tableEntry CreateTexture3D;      // 6
        tableEntry CreateShaderResourceView; // 7
        tableEntry CreateUnorderedAccessView; // 8
        tableEntry CreateRenderTargetView;    // 9
        tableEntry CreateDepthStencilView;    // 10
        tableEntry CreateInputLayout;         // 11
        tableEntry CreateVertexShader;        // 12
        tableEntry CreateGeometryShader;      // 13
        tableEntry CreateGeometryShaderWithStreamOutput; // 14
        tableEntry CreatePixelShader;         // 15
        tableEntry CreateComputeShader;       // 16
        tableEntry CreateDomainShader;        // 17
        tableEntry CreateHullShader;          // 18
        tableEntry CreateRasterizerState;     // 19
        tableEntry CreateDepthStencilState;   // 20
        tableEntry CreateBlendState;          // 21
        tableEntry CreateSamplerState;        // 22
        tableEntry CreateQuery;               // 23
        tableEntry CreatePredicate;           // 24
        tableEntry CreateCounter;             // 25
        tableEntry CreateDeferredContext;     // 26
        tableEntry OpenSharedResource;        // 27
        tableEntry CheckFeatureSupport;       // 28
        tableEntry CheckMultisampleQualityLevels; // 29
        tableEntry CheckSharedResourceHandle; // 30
        tableEntry CheckSharedResourceHandleForD3D; // 31
        tableEntry CheckSharedHandle;         // 32
        tableEntry GetPrivateData;            // 33
        tableEntry SetPrivateData;            // 34
        tableEntry SetPrivateDataInterface;   // 35
        tableEntry SetName;                   // 36
        tableEntry GetFeatureLevel;           // 37
        tableEntry GetCreationFlags;          // 38
        tableEntry GetDeviceRemovedReason;    // 39
        tableEntry SetExceptionMode;          // 40
        tableEntry GetExceptionMode;          // 41
        tableEntry GetImmediateContext;       // 42  <- Hook index 42
    };

    // IDXGISwapChain vtable - 22 entries
    struct IDXGISwapChainVTable
    {
        tableEntry QueryInterface;   // 0
        tableEntry AddRef;           // 1
        tableEntry Release;          // 2
        tableEntry SetParent;        // 3
        tableEntry SetFullscreenState; // 4
        tableEntry GetFullscreenState; // 5
    #if defined(_MSC_VER) && _MSC_VER >= 1900
        tableEntry SetWindowed;      // 6
        tableEntry SetResizeBuffers; // 7
        tableEntry SetResizeTarget; // 8
        tableEntry GetFullscreenDesc; // 9
        tableEntry GetPrimitiveTopology; // 10
        tableEntry SetPrimitiveTopology; // 11
        tableEntry GetBuffer;        // 12
        tableEntry SetCallback;      // 13
        tableEntry GetDesc;          // 14 (deprecated)
        tableEntry Present;          // 15 - THIS IS THE ONE WE NEED - Hook index 15
    #else
        tableEntry Present;          // 15
    #endif
    };
}
