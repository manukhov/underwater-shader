#include "injector\hooking.hpp"
#include "RenderWare.h"

#include <d3dx9.h>
#include <fstream>
#include <sstream>
#pragma comment(lib, "d3dx9.lib")

LPD3DXBUFFER pCode;
LPD3DXBUFFER pErrors;

DWORD g_Shader = NULL;
RwRaster* g_Raster = NULL;

void (*fnrender)();

class CRGBA
{
    unsigned char r, g, b, a;
public:
    CRGBA(unsigned char _1, unsigned char _2, unsigned char _3, unsigned char _4) : r(_1), g(_2), b(_3), a(_4) {}
};

class CRect
{
    float left, bottom, right, top;
public:
    CRect(float _1, float _2, float _3, float _4) : left(_1), top(_2), right(_3), bottom(_4) {}
};

#define GameCamera (*(RwCamera **)0xC1703C)
#define RwRenderStateSet(state, value) (((void (*)(RwRenderState, unsigned int))0x7FE420)(state, value))
#define RwD3D9SetPixelShader(shader) (((void (*)(DWORD))0x7F9FF0)(shader))
#define RwD3D9SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount) (((HRESULT (__cdecl *)(UINT, const float *, UINT))0x7FAD00)(StartRegister, pConstantData, Vector4fCount))
#define RwD3D9CreatePixelShader(pfunction, shader) (((bool (*)(DWORD *, DWORD *))0x7FACC0)(pfunction, shader))
#define RwD3D9DeletePixelShader(shader) (((void (*)(DWORD))0x7FACF0)(shader))
#define RwRasterCreate(width, height, depth, flags) ((RwRaster *(__cdecl *)(int, int, int, int))0x7FB230)(width, height, depth, flags)
#define RwRasterDestroy(raster) ((void (__cdecl *)(RwRaster *))0x7FB020)(raster)
#define RwRasterRenderFast(raster, x, y) ((RwRaster * (__cdecl *)(RwRaster *, int, int))0x7FAF50)(raster, x, y)
#define RwRasterPushContext(raster) ((RwRaster * (__cdecl *)(RwRaster *))0x7FB060)(raster)
#define RwRasterPopContext() ((RwRaster * (__cdecl *)())0x7FB110)()
#define RwCameraEndUpdate(camera) ((RwCamera *(__cdecl *)(RwCamera *))0x7EE180)(camera)
#define RwCameraBeginUpdate(camera) ((RwCamera *(__cdecl *)(RwCamera *))0x7EE190)(camera)
#define CSprite2d__Draw(sprite, rect, color) (((void (__thiscall *)(void *, CRect&, CRGBA&))0x728350)(sprite, rect, color))

const char* shaderHLSL =
"float4 main(float2 texCoord : TEXCOORD) : COLOR {\n"
"    return float4(0.0f, 0.0f, 1.0f, 1.0f); \n"
"}\n"
"technique BlueScreen {\n"
"    pass P0 {\n"
"        PixelShader = compile ps_3_0 main();\n"
"    }\n"
"}\n";


void LogMessage(const char* message)
{
    std::ofstream logFile("debug.txt", std::ios_base::app);
    logFile << message << std::endl;
}

void CompileShader()
{
    LogMessage("Starting shader compilation...");

    HRESULT hr = D3DXCompileShader(shaderHLSL, strlen(shaderHLSL), NULL, NULL, "main", "ps_3_0", 0, &pCode, &pErrors, NULL);

    if (FAILED(hr))
    {
        LogMessage("Shader compilation failed.");
        if (pErrors)
        {
            std::string errorMsg = "Shader Compilation Error: ";
            errorMsg += static_cast<char*>(pErrors->GetBufferPointer());
            LogMessage(errorMsg.c_str());
            pErrors->Release();
        }
    }
    else
    {
        LogMessage("Shader compilation succeeded.");

        if (pCode)
        {
            std::string shaderCodeSizeMsg = "Shader code size: ";
            shaderCodeSizeMsg += std::to_string(pCode->GetBufferSize());
            LogMessage(shaderCodeSizeMsg.c_str());
        }

        if (!RwD3D9CreatePixelShader((DWORD*)pCode->GetBufferPointer(), &g_Shader))
            LogMessage("Shader creation failed.");
        else
            LogMessage("Shader creation succeeded.");

        pCode->Release();
    }
}


void SetShaderCB(DWORD shader)
{
    RwD3D9SetPixelShader(g_Shader);

    float distortionIntensity[2] = { 0.05f, 0.05f };

    HRESULT result = RwD3D9SetPixelShaderConstantF(0, distortionIntensity, 1);
    if (SUCCEEDED(result))
    {
        LogMessage("Pixel shader constant set successfully.");
    }
    else
    {
        LogMessage("Failed to set pixel shader constant.");
    }
}

void Render()
{
    fnrender();

    CompileShader();

    if (GameCamera->frameBuffer)
    {
        auto width = GameCamera->frameBuffer->width;
        auto height = GameCamera->frameBuffer->height;
        auto depth = GameCamera->frameBuffer->depth;

        if (g_Raster && (width != g_Raster->width || height != g_Raster->height || depth != g_Raster->depth))
        {
            LogMessage("Destroying old raster.");
            RwRasterDestroy(g_Raster);
            g_Raster = NULL;
        }

        if (!g_Raster)
        {
            LogMessage("Creating new raster.");
            g_Raster = RwRasterCreate(width, height, depth, rwRASTERTYPECAMERATEXTURE);
        }

        RwCameraEndUpdate(GameCamera);
        RwRasterPushContext(g_Raster);
        RwRasterRenderFast(GameCamera->frameBuffer, 0, 0);
        RwRasterPopContext();
        RwCameraBeginUpdate(GameCamera);
    }
    SetShaderCB(g_Shader);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, 5); // D3DBLEND_SRCALPHA
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, 6); // D3DBLEND_INVSRCALPHA
    void* pRaster = &g_Raster;
    CSprite2d__Draw(&pRaster, CRect(1.0f, 0.5f, g_Raster->width + 1.0f, g_Raster->height + 0.5f), CRGBA(255, 255, 255, 255));
    injector::MakeCALL(0x7FB824, 0x7F9FF0);
}



BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        memcpy(reinterpret_cast<void*>(0x705125), "\xE9\x7A\x01\x00\x00\x90\x90\x90", 8);
        fnrender = injector::MakeCALL(0x53E170, Render).get();
    }
    return TRUE;
}
