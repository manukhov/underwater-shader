#pragma once

#include "plugin.h"
#include "RenderWare.h"
#include "CSprite2d.h"

#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")

LPD3DXBUFFER pCode;
LPD3DXBUFFER pErrors;

IDirect3DDevice9* pDevice = nullptr;
ID3DXBuffer* pDisassembly = nullptr;

IDirect3DPixelShader9* g_Shader = nullptr;
IDirect3DPixelShader9* g_CurrentShader = nullptr;

RwRaster* workBuffer = nullptr;


bool bShaderCompiled = false;
bool bShaderSetSuccessfully = false;
bool bFirstRun = true;

void (*fnrender)();

CSprite2d Sprite2d;

#define GameCamera (*(RwCamera**)0xC1703C)

const char* HLSL =
"sampler2D tex0 : register(s0); \n"
"float4 c0 = float4(0.30f, 0.59f, 0.11f, 1.0f); \n"
"\n"
"float4 main(float2 texCoord : TEXCOORD0) : COLOR \n"
"{ \n"
"    float4 texColor = tex2D(tex0, texCoord); \n"
"    float gray = dot(texColor.rgb, c0.rgb); \n"
"    return float4(gray, gray, gray, texColor.a); \n"
"} \n"
"\n"
"technique GrayscaleScreen \n"
"{ \n"
"    pass P0 \n"
"    { \n"
"        PixelShader = compile ps_3_0 main(); \n"
"    } \n"
"} \n";


void Debug(const char* message)
{
    if (bFirstRun)
    {
        std::remove("debug.txt");
        bFirstRun = false;
    }
    std::ofstream logFile("debug.txt", std::ios_base::app);
    logFile << message << std::endl;
}

void CompileShader()
{
    HRESULT CompileResult = D3DXCompileShader(HLSL, strlen(HLSL), NULL, NULL, "main", "ps_3_0", 0, &pCode, &pErrors, NULL);

    if (FAILED(CompileResult))
    {
        Debug("Shader compilation failed");

        if (pErrors)
        {
            std::string errorMsg = "Shader Compilation Error: ";
            errorMsg += static_cast<char*>(pErrors->GetBufferPointer());
            Debug(errorMsg.c_str());
            pErrors->Release();
        }
    }
    else
    {
        Debug("Shader compilation succeeded");

        if (pCode)
        {
            std::string shaderCodeSizeMsg = "Code size: ";
            shaderCodeSizeMsg += std::to_string(pCode->GetBufferSize());
            Debug(shaderCodeSizeMsg.c_str());
        }
        if (!RwD3D9CreatePixelShader((RwUInt32*)pCode->GetBufferPointer(), &g_Shader))
            Debug("Shader creation failed");
        else
        {
            Debug("Shader creation succeeded");

            HRESULT DisasmResult = D3DXDisassembleShader((DWORD*)pCode->GetBufferPointer(), FALSE, nullptr, &pDisassembly);
            if (SUCCEEDED(DisasmResult))
            {
                if (pDisassembly && pDisassembly->GetBufferSize() > 0)
                {
                    const char* shaderText = (const char*)pDisassembly->GetBufferPointer();
                    Debug(shaderText);
                }
                else
                    Debug("D3DXDisassembleShader: Buffer is empty");
            }
            else
                Debug("D3DXDisassembleShader failed");
        }
        bShaderCompiled = true;
        pDisassembly->Release();
        pCode->Release();
    }
}

void SetShaderCB()
{
    _rwD3D9SetPixelShader(g_Shader);

    pDevice = *(IDirect3DDevice9**)(0xC97C28);

    if (!bShaderSetSuccessfully)
    {
        g_CurrentShader = nullptr;

        pDevice->GetPixelShader(&g_CurrentShader);

        if (g_CurrentShader == g_Shader)
        {
            Debug("Pixel shader set successfully");
            bShaderSetSuccessfully = true;
        }
        else
            Debug("Failed to set pixel shader");

        if (g_CurrentShader)
            g_CurrentShader->Release();
    }
}

void Render()
{
    fnrender();

    if (!bShaderCompiled)
        CompileShader();

    if (GameCamera && GameCamera->frameBuffer)
    {
        RwUInt32 width = GameCamera->frameBuffer->width;
        RwUInt32 height = GameCamera->frameBuffer->height;
        RwUInt32 depth = GameCamera->frameBuffer->depth;

        if (workBuffer && (width != workBuffer->width || height != workBuffer->height || depth != workBuffer->depth))
        {
            Debug("Destroying old raster");
            RwRasterDestroy(workBuffer);
            workBuffer = NULL;
        }
        if (workBuffer == NULL)
        {
            workBuffer = RwRasterCreate(GameCamera->frameBuffer->width, GameCamera->frameBuffer->height, GameCamera->frameBuffer->depth, rwRASTERTYPECAMERATEXTURE);
            if (workBuffer == NULL)
            {
                Debug("Failed to create raster");
                return;
            }
            Debug("Raster created successfully");
        }
    }
    else
    {
        Debug("Game camera or frame buffer is invalid");
        return;
    }

    RwCameraEndUpdate(GameCamera);
    RwRasterPushContext(workBuffer);
    RwRasterRenderFast(GameCamera->frameBuffer, 0, 0);
    RwRasterPopContext();
    RwCameraBeginUpdate(GameCamera);

    injector::MakeCALL(0x7FB824, SetShaderCB);


    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)workBuffer);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);


    // Настройки для рендера квадрата
    float x1 = 1.0f;
    float y1 = 0.5f;
    float x2 = workBuffer->width + 1.0f;
    float y2 = workBuffer->height + 0.5f;
    CRGBA color(255, 255, 255, 255);

    // Отрисовка спрайта с растром
    Sprite2d.Draw(x1, y1, x2, y2, color);

    // Возвращаем настройки после отрисовки
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nullptr);

    injector::MakeCALL(0x7FB824, 0x7F9FF0);
}



BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        memcpy(reinterpret_cast<void*>(0x705125), "\xE9\x7A\x01\x00\x00\x90\x90\x90", 8);
        fnrender = injector::MakeCALL(0x53EAD3, Render).get();
    }
    return TRUE;
}
