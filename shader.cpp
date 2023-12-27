#include "shader.hpp"

const char* HLSL =
"float4 main() : COLOR0\n"
"{\n"
"    return float4(1.0, 0.0, 0.0, 1.0); // RED \n"
"}\n";

IDirect3DPixelShader9* CompilePixelShader(const char* HLSL) 
{
    IDirect3DPixelShader9* shader = nullptr;
    ID3DXBuffer* shaderBuffer = nullptr;
    ID3DXBuffer* errorBuffer = nullptr;

    HRESULT hr = D3DXCompileShader(HLSL, strlen(HLSL), nullptr, nullptr, "main", "ps_2_0", 0, &shaderBuffer, &errorBuffer, nullptr);
    if (FAILED(hr)) 
    {
        if (errorBuffer) 
        {
            const char* errorMsg = static_cast<const char*>(errorBuffer->GetBufferPointer());
            MessageBoxA(nullptr, errorMsg, "Shader Compilation Error", MB_OK | MB_ICONERROR);
            errorBuffer->Release();
        }
        return nullptr;
    }

    hr = g_rwD3D9CreatePixelShader(reinterpret_cast<const uint32_t>(shaderBuffer->GetBufferPointer()), &shader);
    if (FAILED(hr)) {
        if (shaderBuffer)
            shaderBuffer->Release();
        return nullptr;
    }

    if (shaderBuffer)
        shaderBuffer->Release();
    if (errorBuffer)
        errorBuffer->Release();

    return shader;
}

HRESULT __cdecl hkUnderwaterEffect(int arg1, float arg2, float arg3, int arg4, float arg5, float arg6) 
{
    IDirect3DPixelShader9* pixelShader = CompilePixelShader(HLSL);

    if (!pixelShader)
        return g_UnderwaterEffect(arg1, arg2, arg3, arg4, arg5, arg6);

    memset(reinterpret_cast<void*>(0x7FB824), 0x90, 5);
    g_rwD3D9SetPixelShader(pixelShader);
    pixelShader->Release();

    return D3D_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) 
{
    if (fdwReason) 
    {
        DisableThreadLibraryCalls(hModule);

        DWORD hkCall_UnderwaterEffect = 0x70529C;

        *(BYTE*)hkCall_UnderwaterEffect = 0xE8;
        *(DWORD*)(hkCall_UnderwaterEffect + 1) = (DWORD)hkUnderwaterEffect - hkCall_UnderwaterEffect - 5;
    }
    return TRUE;
}
