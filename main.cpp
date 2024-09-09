#include "injector\hooking.hpp"
#include "RenderWare.h"

DWORD PixelShader[] =
{
	0x0003FFFF, 0xFEFF002E, 0x43424142, 0x1C000000,
	0x83000000, 0x03FFFF02, 0x0000001C, 0x00000000,
	0x0100007C, 0x00000044, 0x00000002, 0x00000001,
	0x00000048, 0x00000058, 0x00000068, 0x00030000,
	0x01000200, 0x6C000000, 0x00000000, 0x00000063,
	0x3000AB01, 0x00030001, 0x00040001, 0x00000000,
	0x00000000, 0x9A99993E, 0x3D0A173F, 0xAE47E13D,
	0x0000803F, 0x743000AB, 0x04000C00, 0x01000100,
	0x01000000, 0x00000000, 0x0070735F, 0x335F3000,
	0x4D696372, 0x6F736F66, 0x74202028, 0x52292048,
	0x4C534C20, 0x53686164, 0x65722043, 0x6F6D7069,
	0x6C657220, 0x392E3239, 0x2E393532, 0x2E333131,
	0x3100001F, 0x00000205, 0x00008000, 0x0003901F,
	0x00000200, 0x00000090, 0x00080FA0, 0x42000003,
	0x00000F80, 0x0000E490, 0x0008E4A0, 0x08000003,
	0x00000180, 0x0000E480, 0x0000E4A0, 0x01000002,
	0x00000280, 0x0000FF80, 0x01000002, 0x00080F80,
	0x00004080, 0xFFFF0000
};

DWORD g_Shader;
RwRaster* g_Raster = NULL;

void (*fnrender)();

class CRGBA
{
unsigned char r,g,b,a;
public:
	CRGBA(unsigned char _1, unsigned char _2, unsigned char _3, unsigned char _4) : r(_1), g(_2), b(_3), a(_4)  {}
};

class CRect
{
float left, bottom, right, top;
public:
	CRect(float _1, float _2, float _3, float _4): left(_1), top(_2), right(_3), bottom(_4)  {}
};

#define GameCamera (*(RwCamera **)0xC1703C)
#define RwRenderStateSet(state, value) (((void (*)(RwRenderState, unsigned int))0x7FE420)(state, value))
#define RwD3D9SetPixelShader(shader) (((void (*)(DWORD))0x7F9FF0)(shader))
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

void SetShaderCB(DWORD shader)
{
	RwD3D9SetPixelShader(g_Shader);
}

void Render()
{
	fnrender();
	RwD3D9CreatePixelShader(PixelShader, &g_Shader);
	if (GameCamera->frameBuffer)
	{
		auto width = GameCamera->frameBuffer->width;
		auto height = GameCamera->frameBuffer->height;
		auto depth = GameCamera->frameBuffer->depth;
		if (g_Raster && (width != g_Raster->width || height != g_Raster->height || depth != g_Raster->depth))
		{
			RwRasterDestroy(g_Raster);
			g_Raster = nullptr;
		}
		if (!g_Raster)
			g_Raster = RwRasterCreate(width, height, depth, rwRASTERTYPECAMERATEXTURE);

		RwCameraEndUpdate(GameCamera);
		RwRasterPushContext(g_Raster);
		RwRasterRenderFast(GameCamera->frameBuffer, 0, 0);
		RwRasterPopContext();
		RwCameraBeginUpdate(GameCamera);
	}
    injector::MakeCALL(0x7FB824, SetShaderCB);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, 5);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, 6);
    void *pRaster = &g_Raster;
    CSprite2d__Draw(&pRaster, CRect(1.0f, 0.5f, g_Raster->width + 1.0f, g_Raster->height + 0.5f), CRGBA(255, 255, 255, 255));
	injector::MakeCALL(0x7FB824, 0x7F9FF0);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		memcpy(reinterpret_cast<void*>(0x705125), "\xE9\x7A\x01\x00\x00\x90\x90\x90", 8);
		fnrender = injector::MakeCALL(0x705116, Render).get();
	}
    return TRUE;
}