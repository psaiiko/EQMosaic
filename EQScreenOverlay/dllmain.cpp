// -------------------------------------------------------------------------------------------------
// Includes
// -------------------------------------------------------------------------------------------------

#include "pch.h"
#include "LogConsole.h"
#include <d3d11.h>
#include <dxgi.h>
#include <stdint.h>
#include "MinHook.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <string>
#include <type_traits>
#include <utility>
#include <d3dcompiler.h>
#include "AutoLogin/AutoLogin.h"
#include "AutoLogin/EQGameInterface.h"
#include "FileRedirection.h"
#include "Config.h"

// -------------------------------------------------------------------------------------------------
// Linking
// -------------------------------------------------------------------------------------------------

#pragma comment(lib, "d3dcompiler.lib")

// -------------------------------------------------------------------------------------------------
// Defines and type aliases
// -------------------------------------------------------------------------------------------------

#ifdef _UNICODE
# define DEFINE_TEXT(text) L##text
#else
# define DEFINE_TEXT(text) text
#endif

#if _WIN64
typedef uint64_t ptrType;
#else
typedef uint32_t ptrType;
#endif

#pragma pack (push)
#pragma pack (4)
struct MappedFileData
{
    unsigned int frameOffset;
    unsigned int textureHandleLow;
    unsigned int textureHandleHigh;
    unsigned int commandID;
};
#pragma pack (pop)

// -------------------------------------------------------------------------------------------------
// Scope guard helper
// -------------------------------------------------------------------------------------------------

template<typename F>
class scope_guard {
    F f_;
    bool dismissed_ = false;
public:
    scope_guard(F f) : f_(std::move(f)) {}
    ~scope_guard() { if (!dismissed_) f_(); }
    void dismiss() { dismissed_ = true; }
    void fire() { if (!dismissed_) { f_(); dismissed_ = true; } }
};

template<typename F>
scope_guard<typename std::decay<F>::type> make_scope_guard(F&& f) {
    return scope_guard<typename std::decay<F>::type>(std::forward<F>(f));
}

// -------------------------------------------------------------------------------------------------
// Hook function types and pointers
// -------------------------------------------------------------------------------------------------

typedef HRESULT(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present s_originalPresentPtr = NULL;

typedef HRESULT(__stdcall* ResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
static ResizeBuffers s_originalResizeBuffersPtr = NULL;

HWND GetGameWindowHandle(DWORD processId);


// -------------------------------------------------------------------------------------------------
// Globals
// -------------------------------------------------------------------------------------------------

static ptrType* s_swapChainVtable = NULL;
HINSTANCE DllHandle;

// Textures for downscaling and reading back pixels
ID3D11Texture2D* pCaptureResultRT = NULL;       // Small GPU-shared texture (750x375)
ID3D11UnorderedAccessView* pUAV = NULL;           // Compute output destination
ID3D11ComputeShader* pComputeShader = NULL;       // Scaling shader kernel
ID3D11SamplerState* pLinearSampler = NULL;        // Hardware linear filter
ID3D11Texture2D* pBackBufferCopy = NULL;
UINT cachedWidth = 0;
UINT cachedHeight = 0;
DXGI_FORMAT cachedFormat = DXGI_FORMAT_UNKNOWN;
ID3D11Device* gCachedDevice = nullptr;

HANDLE hMapFile = INVALID_HANDLE_VALUE;
MappedFileData* pMappedFileData = nullptr;
int g_PresentedFrames = 0;

#define SCREENSHOT_WIDTH 750

HANDLE g_sharedHandle = NULL;

static AutoLogin g_autoLogin;
static wchar_t g_dllPath[MAX_PATH] = {};
static uint64_t g_lastWindowTitleChange = GetTickCount64();


// -------------------------------------------------------------------------------------------------
// Compute shader source
// -------------------------------------------------------------------------------------------------

// Raw HLSL Compute Shader string that samples the backbuffer linearly using UVs
const char* c_ScalingShader =
"Texture2D<float4> InputTex : register(t0);\n"
"RWTexture2D<float4> OutputTex : register(u0);\n"
"SamplerState LinearSampler : register(s0);\n"
"[numthreads(16, 16, 1)]\n"
"void main(uint3 DTid : SV_DispatchThreadID)\n"
"{\n"
"    uint width, height;\n"
"    OutputTex.GetDimensions(width, height);\n"
"    if (DTid.x >= width || DTid.y >= height) return;\n"
"    float2 uv = float2((float)DTid.x / (width - 1), (float)DTid.y / (height - 1));\n"
"    float4 color = InputTex.SampleLevel(LinearSampler, uv, 0);\n"
"    color.a = 1.0f;\n"
"    OutputTex[DTid.xy] = color;\n"
"}\n";

// -------------------------------------------------------------------------------------------------
// DX object management
// -------------------------------------------------------------------------------------------------

void ReleaseInternalDXObjects()
{
    if (pCaptureResultRT) { pCaptureResultRT->Release(); pCaptureResultRT = NULL; }
    if (pUAV) { pUAV->Release(); pUAV = NULL; }
    if (pComputeShader) { pComputeShader->Release(); pComputeShader = NULL; }
    if (pLinearSampler) { pLinearSampler->Release(); pLinearSampler = NULL; }
    if (pBackBufferCopy) { pBackBufferCopy->Release(); pBackBufferCopy = NULL; }

    cachedWidth = 0;
    cachedHeight = 0;
    cachedFormat = DXGI_FORMAT_UNKNOWN;
    g_sharedHandle = NULL;
}

// -------------------------------------------------------------------------------------------------
// Utility functions
// -------------------------------------------------------------------------------------------------

HWND GetGameWindowHandle(DWORD processId)
{
    struct Context
    {
        DWORD pid;
        HWND hwnd{};
    };

    Context ctx{ processId };

    EnumWindows(
        [](HWND hWnd, LPARAM lParam) -> BOOL
        {
            auto& ctx = *reinterpret_cast<Context*>(lParam);

            DWORD pid = 0;
            GetWindowThreadProcessId(hWnd, &pid);

            if (pid != ctx.pid)
                return TRUE;

            char className[256]{};

            GetClassNameA(
                hWnd,
                className,
                sizeof(className));

            if (std::string_view(className) == "_EverQuestwndclass")
            {
                ctx.hwnd = hWnd;
                return FALSE;
            }

            return TRUE;
        },
        reinterpret_cast<LPARAM>(&ctx));

    return ctx.hwnd;
}

void OverrideWindowTitle(DWORD processId, const std::string& playerName)
{
    HWND hwnd = GetGameWindowHandle(processId);
    if (hwnd == nullptr)
        return;

    uint64_t now = GetTickCount64();
    uint64_t elapsed = now - g_lastWindowTitleChange;
    if (elapsed < 1000)
        return;

    g_lastWindowTitleChange = now;

    std::string title = "Everquest - " + playerName;
    char currentTile[1024];
    if (GetWindowTextA(hwnd, currentTile, sizeof(currentTile)) <= 0)
        return;

    if (title!= currentTile)
        SetWindowTextA(hwnd, title.c_str());
}

// -------------------------------------------------------------------------------------------------
// Hook: IDXGISwapChain::Present
// -------------------------------------------------------------------------------------------------
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    HRESULT hr = s_originalPresentPtr(pSwapChain, SyncInterval, Flags);

    if (Flags & DXGI_PRESENT_TEST) // or simply: if (Flags == 1)
        return hr;

    if (!pMappedFileData || !pSwapChain)
        return hr;

    ID3D11Device* device = NULL;
    if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device)))
        return hr;

    bool deviceChanged = (device != gCachedDevice);
    if (deviceChanged)
    {
        ConsolePrintf("[Overlay] Device has changed!!!!\n");

        ReleaseInternalDXObjects();

        if (gCachedDevice)
            gCachedDevice->Release();

        gCachedDevice = device;
        gCachedDevice->AddRef();
    }


    ID3D11DeviceContext* context = NULL;
    device->GetImmediateContext(&context);
    if (!context)
    {
        device->Release();
        return hr;
    }

    ID3D11Texture2D* backBuffer = NULL;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
    {
        context->Release();
        device->Release();
        return hr;
    }

    // --- NEW: FETCH THE CURRENT BACKBUFFER SPECS EVERY FRAME ---
    D3D11_TEXTURE2D_DESC backBufferDesc;
    backBuffer->GetDesc(&backBufferDesc);

    // --- DYNAMIC RE-INITIALIZATION TRIGGER ---
    // If anything shifted (Screen change, resolution scale, full-screen swap), dump resources!
    if (pCaptureResultRT != NULL &&
        (backBufferDesc.Width != cachedWidth ||
            backBufferDesc.Height != cachedHeight ||
            backBufferDesc.Format != cachedFormat))
    {
        // Call your global cleanup function to cleanly wipe out old pointers
        ReleaseInternalDXObjects();

        ConsolePrintf("[Overlay] Backbuffer format changed. Format: %d\n", backBufferDesc.Format);
    }

    float backBufferAspect = backBufferDesc.Width / float(backBufferDesc.Height);
    int destHeight = int(SCREENSHOT_WIDTH / backBufferAspect);

    // Allocate/Re-allocate resources safely if empty
    if (pCaptureResultRT == NULL)
    {
        // Cache the new game engine properties
        cachedWidth = backBufferDesc.Width;
        cachedHeight = backBufferDesc.Height;
        cachedFormat = backBufferDesc.Format;

        // 1. Create a matching auxiliary texture matching current frame specifications
        D3D11_TEXTURE2D_DESC copyDesc = backBufferDesc;
        copyDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        copyDesc.Usage = D3D11_USAGE_DEFAULT;
        copyDesc.CPUAccessFlags = 0;
        copyDesc.MiscFlags = 0;

        if (FAILED(device->CreateTexture2D(&copyDesc, NULL, &pBackBufferCopy))) goto cleanup;

        // 2. Create Small Destination Texture with UAV flags allowed
        D3D11_TEXTURE2D_DESC rtDesc = {};
        rtDesc.Width = SCREENSHOT_WIDTH;
        rtDesc.Height = destHeight;
        rtDesc.MipLevels = 1;
        rtDesc.ArraySize = 1;
        rtDesc.Format = cachedFormat;
        rtDesc.SampleDesc.Count = 1;
        rtDesc.Usage = D3D11_USAGE_DEFAULT;
        rtDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        rtDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

        if (FAILED(device->CreateTexture2D(&rtDesc, NULL, &pCaptureResultRT))) goto cleanup;

        // Grab the shared handle for cross-process texture access
        {
            IDXGIResource* dxgiRes = NULL;
            if (SUCCEEDED(pCaptureResultRT->QueryInterface(__uuidof(IDXGIResource), (void**)&dxgiRes)))
            {
                dxgiRes->GetSharedHandle(&g_sharedHandle);
                dxgiRes->Release();
            }
        }

        // 3. Create the UAV tied to the small texture
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = rtDesc.Format;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        if (FAILED(device->CreateUnorderedAccessView(pCaptureResultRT, &uavDesc, &pUAV))) goto cleanup;

        // 4. Compile and spin up the micro Compute Shader dynamically
        ID3DBlob* shaderBlob = NULL;
        ID3DBlob* errorBlob = NULL;
        HRESULT hr = D3DCompile(c_ScalingShader, strlen(c_ScalingShader), NULL, NULL, NULL, "main", "cs_5_0", 0, 0, &shaderBlob, &errorBlob);
        if (FAILED(hr))
        {
            if (errorBlob) {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            goto cleanup;
        }
        device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &pComputeShader);
        shaderBlob->Release();

        // 5. Create basic bilinear sampler
        D3D11_SAMPLER_DESC sampDesc = {};
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        device->CreateSamplerState(&sampDesc, &pLinearSampler);
    }

    // High speed GPU-to-GPU copy into our shader-friendly buffer
    context->CopyResource(pBackBufferCopy, backBuffer);
    backBuffer->Release();
    backBuffer = NULL;



    // --- ACCELERATED GPU SCALING PIPELINE RUN ---
    {
        D3D11_TEXTURE2D_DESC copyDesc;
        pBackBufferCopy->GetDesc(&copyDesc);

        ID3D11ShaderResourceView* pSRV = NULL;
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        // Dynamic SRV mapping block to capture format shifts automatically
        if (copyDesc.Format == DXGI_FORMAT_R8G8B8A8_TYPELESS)          srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        else if (copyDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)   srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        else if (copyDesc.Format == DXGI_FORMAT_B8G8R8A8_TYPELESS)      srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        else if (copyDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)   srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        else                                                           srvDesc.Format = copyDesc.Format;

        if (SUCCEEDED(device->CreateShaderResourceView(pBackBufferCopy, &srvDesc, &pSRV)))
        {
            // Bind scaling shader setup
            context->CSSetShader(pComputeShader, NULL, 0);
            context->CSSetShaderResources(0, 1, &pSRV);
            context->CSSetUnorderedAccessViews(0, 1, &pUAV, NULL);
            context->CSSetSamplers(0, 1, &pLinearSampler);

            // Execute GPU core scale matrix
            UINT dispatchX = (SCREENSHOT_WIDTH + 15) / 16;
            UINT dispatchY = (destHeight + 15) / 16;
            context->Dispatch(dispatchX, dispatchY, 1);
            context->Flush();

            // Declare your clean-slate null pointers once
            ID3D11ComputeShader* nullCS = NULL;
            ID3D11ShaderResourceView* nullSRV = NULL;
            ID3D11UnorderedAccessView* nullUAV = NULL;
            ID3D11SamplerState* nullSamp = NULL;
            UINT zeroCount = 0;

            // Unbind our resources by overriding slots with NULL
            context->CSSetShader(nullCS, NULL, 0);
            context->CSSetShaderResources(0, 1, &nullSRV);
            context->CSSetUnorderedAccessViews(0, 1, &nullUAV, &zeroCount);
            context->CSSetSamplers(0, 1, &nullSamp);

            // Release the actual view you created at the start of the block
            pSRV->Release();
        }
    }

    // Write header to shared memory for the box tool
    {
        pMappedFileData->frameOffset = g_PresentedFrames++;
        uint64_t handleVal = (uint64_t)(uintptr_t)g_sharedHandle;
        pMappedFileData->textureHandleLow = (unsigned int)(handleVal & 0xFFFFFFFF);
        pMappedFileData->textureHandleHigh = (unsigned int)(handleVal >> 32);
    }

    g_autoLogin.Update();

cleanup:
    if (backBuffer != NULL) backBuffer->Release();
    if (context != NULL) context->Release();
    if (device != NULL) device->Release();

    return hr;
}

// -------------------------------------------------------------------------------------------------
// Hook: IDXGISwapChain::ResizeBuffers
// -------------------------------------------------------------------------------------------------

HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    // Wipe out your internal DX textures and views completely
    ReleaseInternalDXObjects();

    ConsolePrintf("[Overlay] Device/Buffer reset detected. Flushed custom DX objects.\n");

    // Call the original game function and catch the result
    HRESULT hr = s_originalResizeBuffersPtr(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    if (FAILED(hr))
    {
        ConsolePrintf("[Overlay] s_originalResizeBuffersPtr FAILED! HRESULT: 0x%08X\n", hr);
    }
    else
    {
        ConsolePrintf("[Overlay] s_originalResizeBuffersPtr succeeded cleanly.\n");
    }

    return hr;
}

// -------------------------------------------------------------------------------------------------
// D3D11 device and swap chain discovery
// -------------------------------------------------------------------------------------------------

bool FetchDirectXObject()
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = DefWindowProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.lpszClassName = DEFINE_TEXT("EQOverlayClass");

    ::RegisterClassEx(&windowClass);

    HWND window = ::CreateWindow(windowClass.lpszClassName, DEFINE_TEXT("Kiero DirectX Window"),
        WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

    HMODULE libDXGI = ::GetModuleHandle(DEFINE_TEXT("dxgi.dll"));
    if (!libDXGI)
    {
        ::DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    void* d3d11CreateDeviceAndSwapChain = ::GetProcAddress(GetModuleHandle(L"d3d11.dll"), "D3D11CreateDeviceAndSwapChain");
    if (!d3d11CreateDeviceAndSwapChain)
    {
        ::DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 100;
    sd.BufferDesc.Height = 100;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    ID3D11Device* device = NULL;
    ID3D11DeviceContext* immediateContext = NULL;
    IDXGISwapChain* swapChain = NULL;

    auto guard = make_scope_guard([&]() {
        if (device) device->Release();
        if (immediateContext) immediateContext->Release();
        if (swapChain) swapChain->Release();
        ::DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        });

    D3D11_CREATE_DEVICE_FLAG createDeviceFlags = (D3D11_CREATE_DEVICE_FLAG)0;
#ifdef _DEBUG
    createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = ((HRESULT(__stdcall*)(
        IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL*, UINT,
        UINT, const DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**
        ))d3d11CreateDeviceAndSwapChain)(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
            featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd, &swapChain, &device, NULL, &immediateContext);

    if (FAILED(hr))
        return false;

    // Extract Swap Chain's Virtual Table (Contains 18 functions for IDXGISwapChain)
    // Present() is at index 8 of the standard IDXGISwapChain interface.
    auto vtable = *(ptrType**)swapChain;
    if (!vtable)
        return false;

    s_swapChainVtable = (ptrType*)::calloc(1, 18 * sizeof(ptrType));
    if (!s_swapChainVtable)
        return false;

    memcpy(s_swapChainVtable, vtable, 18 * sizeof(ptrType));

    guard.fire();

    return true;
}

// -------------------------------------------------------------------------------------------------
// Thread and cleanup utilities
// -------------------------------------------------------------------------------------------------

DWORD __stdcall EjectThread(LPVOID lpParameter) {
    Sleep(100);
    FreeLibraryAndExitThread(DllHandle, 0);
    return 0;
}

void CloseMappedFile()
{
    UnmapViewOfFile(pMappedFileData);
    CloseHandle(hMapFile);
    hMapFile = INVALID_HANDLE_VALUE;
    pMappedFileData = NULL;
}

void shutdown()
{
    ReleaseInternalDXObjects();
    RemoveFileHooks();
    MH_Uninitialize();
    CloseMappedFile();
    CreateThread(0, 0, EjectThread, 0, 0, 0);
    return;
}

// -------------------------------------------------------------------------------------------------
// Hook binding helpers
// -------------------------------------------------------------------------------------------------

bool bind(uint16_t _index, void** _original, void* _function)
{
    void* target = (void*)s_swapChainVtable[_index];
    if (MH_CreateHook(target, _function, _original) != MH_OK || MH_EnableHook(target) != MH_OK)
    {
        return false;
    }
    return true;
}

void unbind(uint16_t _index)
{
    MH_DisableHook((void*)s_swapChainVtable[_index]);
}

// -------------------------------------------------------------------------------------------------
// Shared memory helpers
// -------------------------------------------------------------------------------------------------

#define BUF_SIZE (256)
bool CreateMappedFile()
{
    std::wstring MappedFileName = std::wstring(L"OverlayData_") + std::to_wstring(GetCurrentProcessId());

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, MappedFileName.c_str());

    if (hMapFile == NULL) return false;

    pMappedFileData = (MappedFileData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);

    if (pMappedFileData != nullptr)
    {
        memset((void*)pMappedFileData, 0, BUF_SIZE);
    }

    return pMappedFileData != NULL;
}

// -------------------------------------------------------------------------------------------------
// Entry point
// -------------------------------------------------------------------------------------------------

int mainThread()
{
    OpenConsole();

    bool dxInjectionDone = false;
    for (int i = 0; i < 100; ++i)
    {
        dxInjectionDone = FetchDirectXObject();
        if (dxInjectionDone)
            break;
        Sleep(50);
    }

    if (!dxInjectionDone) {
        ConsolePrintf("Cannot fetch Direct X Objects\n");
        CloseConsole();
        return 0;
    }

    // Bind functions
    ConsolePrintf("Binding DX hooks\n");
    if (!bind(8, (void**)&s_originalPresentPtr, hkPresent) == true)
    {
        ConsolePrintf("Cannot bind Present()\n");
        CloseConsole();
        return 0;
    }

    if (!bind(13, (void**)&s_originalResizeBuffersPtr, hkResizeBuffers) == true)
    {
        ConsolePrintf("Cannot bind ResizeBuffers()\n");
        CloseConsole();
        return 0;
    }

    ConsolePrintf("Creating mapped file\n");
    if (!CreateMappedFile())
    {
        ConsolePrintf("Cannot create mapped file\n");
        CloseConsole();
        return 0;
    }

    ConsolePrintf("Init EQ Interface\n");
    InitEQInterface();

    // Note: InstallFileHooksWithCredentials was already called from DllMain,
    // so the CreateFileW hook is active and eqclient.ini redirection is enabled.

    bool versionOk = CheckEQVersion();
    bool credsLoaded = versionOk && g_autoLogin.IsValid();

    if (credsLoaded)
    {
        ConsolePrintf("[AutoLogin] Credentials loaded, starting\n");
        g_autoLogin.Start();
    }
    else
    {
        ConsolePrintf("[AutoLogin] No credentials file%s\n",
            versionOk ? "" : " (version mismatch)");
    }

    std::string playerName = g_autoLogin.GetPlayerName();

#ifdef ENABLE_UNLOAD
    ConsolePrintf("[0] Exit\n");
#endif

    while (true)
    {
#ifdef ENABLE_UNLOAD
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && ((GetAsyncKeyState(VK_NUMPAD0) & 1))|| (GetAsyncKeyState(VK_INSERT) & 1))
            break;
#endif
        OverrideWindowTitle(GetCurrentProcessId(), playerName);
        Sleep(1000);
    }

    unbind(13);
    unbind(8);

    ConsolePrintf("Bailing out !!!!\n");

    CloseConsole();

    shutdown();
    return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DllHandle = hModule;
        GetModuleFileNameW(hModule, g_dllPath, MAX_PATH);

        // Initialize MinHook early and install the CreateFileW hook now,
        // while the process is still suspended. This ensures we catch
        // eqclient.ini reads during EQ's startup.
        MH_Initialize();

        // Capture the credentials from the command line, as we need the account name for the file redirection
        if (g_autoLogin.LoadCredentialsFromCommandLine())
        {
            InstallFileHooksWithCredentials(g_dllPath, g_autoLogin.GetProfile().accountName);
        }

        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mainThread, NULL, 0, NULL);
        break;
    }
    return TRUE;
}
