#include "shader.hpp"

HRESULT __cdecl hkCPostEffectsRender(int arg1, float arg2, float arg3, int arg4, float arg5, float arg6) 
{
    const uint32_t pixelShader[] = { 0x12345678, 0x8765432 };

    void* pixelShaderHandle = nullptr;

    HRESULT result = g_rwD3D9CreatePixelShader(pixelShader, &pixelShaderHandle);
    if (SUCCEEDED(result))
        result = g_rwD3D9SetPixelShader(pixelShaderHandle);
    else
        return g_CPostEffectsRender(arg1, arg2, arg3, arg4, arg5, arg6);
    if (SUCCEEDED(result)) 
        g_rwD3D9DrawPrimitive(D3DPT_TRIANGLELIST, 0, 6);
    else
        return g_CPostEffectsRender(arg1, arg2, arg3, arg4, arg5, arg6);
    
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
