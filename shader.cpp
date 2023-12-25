#include "shader.hpp"

const char* HLSL =
"float4 main() : COLOR0\n"
"{\n"
"    return float4(1.0, 0.0, 0.0, 1.0); // RED \n"
"}\n";

IDirect3DPixelShader9* CompilePixelShader(IDirect3DDevice9* pDevice, const char* HLSL) {
    IDirect3DPixelShader9* pixelShader = nullptr;
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

HRESULT __cdecl hkCPostEffectsRender(int arg1, float arg2, float arg3, int arg4, float arg5, float arg6) 
{
    IDirect3DDevice9* pDevice = *(IDirect3DDevice9**)(0xC97C28);
    IDirect3DPixelShader9* pixelShader = CompilePixelShader(pDevice, HLSL);

    if (pixelShader) 
        pDevice->SetPixelShader(pixelShader); // +
    //  g_rwD3D9SetPixelShader(pixelShader); // -
    return S_OK;
}

void hkInstall() 
{
    while (*reinterpret_cast<uint32_t*>(0xC8D4C0) < 9)  // Is game initialized?
        std::this_thread::sleep_for(std::chrono::milliseconds{100});

    DWORD hkCall_CPostEffectsRender = 0x70529C; // CPostEffects::Render caller

    *(BYTE*)hkCall_CPostEffectsRender = 0xE8;
    *(DWORD*)(hkCall_CPostEffectsRender + 1) = (DWORD)hkCPostEffectsRender - hkCall_CPostEffectsRender - 5;
 
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) 
{
    if (fdwReason) 
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)hkInstall, NULL, NULL, NULL);
    }
    return TRUE;
}
