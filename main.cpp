#include "injector\hooking.hpp"
#include "RenderWare.h"

DWORD PixelShader[] = 
{
   0xffff0200, 0x005fffe0, 0x00110000, 0x00000004,
   0x800f0800, 0x90e40000, 0xa0e40000, 0x800f0800,
   0x80e40000, 0x00000001, 0x800f0800, 0x80e40000,
   0x00000042, 0xb0e40000, 0x800f0800, 0x00000043,
   0x800f0800, 0x90e40000, 0xa0e40001, 0x0000ffff
};

DWORD g_Shader;
RwRaster* g_Raster = NULL;

void (*fnrender)();

void Render();

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
		if (g_Raster && (GameCamera->frameBuffer->cFormat != g_Raster->cFormat || GameCamera->frameBuffer->cType != g_Raster->cType))
		{
			RwRasterDestroy(g_Raster);
			g_Raster = nullptr;
		}
		if (!g_Raster)
			g_Raster = RwRasterCreate(width, height, depth, GameCamera->frameBuffer->cFormat |  GameCamera->frameBuffer->cType);
	}
	RwCameraEndUpdate(GameCamera);
	RwRasterPushContext(g_Raster);
	RwRasterRenderFast(GameCamera->frameBuffer, 0, 0);
	RwRasterPopContext();
	RwCameraBeginUpdate(GameCamera);
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
	fnrender = injector::MakeCALL(0x70529C, Render).get();
    return TRUE;
}