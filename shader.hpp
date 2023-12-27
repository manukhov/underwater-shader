#include <d3d9.h>
#include <d3dx9.h>
#include <thread>

#define rwTEXTUREBASENAMELENGTH     32

struct RwLLLink
{
    RwLLLink* next;
    RwLLLink* prev;
};

struct RwObject
{
    uint8_t type;
    uint8_t subType;
    uint8_t flags;
    uint8_t privateFlags;
    void* parent;
};

struct RwLinkList
{
    RwLLLink link;
};

struct RwTexDictionary
{
    RwObject            object;
    RwLinkList          texturesInDict;
    RwLLLink            lInInstance;
};

struct RwRaster
{
    RwRaster* parent;
    uint8_t* cpPixels;
    uint8_t* palette;
    int32_t             width, height, depth;
    int32_t             stride;
    int16_t             nOffsetX, nOffsetY;
    uint8_t             cType;
    uint8_t             cFlags;
    uint8_t             privateFlags;
    uint8_t             cFormat;

    uint8_t* originalPixels;
    int32_t             originalWidth;
    int32_t             originalHeight;
    int32_t             originalStride;
};

struct RwTexture
{
    RwRaster* raster;
    RwTexDictionary* dict;
    RwLLLink            lInDictionary;

    char              name[rwTEXTUREBASENAMELENGTH];
    char              mask[rwTEXTUREBASENAMELENGTH];
    uint32_t          filterAddressing;
    uint32_t          refCount;
};

typedef HRESULT(__cdecl* UnderwaterEffect)(uint32_t arg1, float arg2, float arg3, uint32_t arg4, float arg5, float arg6);
UnderwaterEffect g_UnderwaterEffect = (UnderwaterEffect)0x7039C0;

typedef HRESULT(__cdecl* rwD3D9RWSetRenderState)(uint32_t state, uint32_t value);
rwD3D9RWSetRenderState g_rwD3D9RWSetRenderState = (rwD3D9RWSetRenderState)0x7FE420; // 0x7FC2D0 ?

typedef HRESULT(__cdecl* rwD3D9DrawPrimitive)(uint32_t primitiveType, uint32_t startVertex, uint32_t primitiveCount);
rwD3D9DrawPrimitive g_rwD3D9DrawPrimitive = (rwD3D9DrawPrimitive)0x7FA360;

typedef HRESULT(__cdecl* rwD3D9DrawIndexedPrimitive)(uint32_t primitiveType, uint32_t baseVertexIndex, uint32_t minIndex, uint32_t numVertices, uint32_t startIndex, uint32_t primitiveCount);
rwD3D9DrawIndexedPrimitive g_rwD3D9DrawIndexedPrimitive = (rwD3D9DrawIndexedPrimitive)0x7FA320;

typedef HRESULT(__cdecl* rwD3D9CreatePixelShader)(const uint32_t function, void* shader);
rwD3D9CreatePixelShader g_rwD3D9CreatePixelShader = (rwD3D9CreatePixelShader)0x7FACC0;

typedef HRESULT(__cdecl* rwD3D9SetPixelShader)(void* shader);
rwD3D9SetPixelShader g_rwD3D9SetPixelShader = (rwD3D9SetPixelShader)0x7F9FF0;

typedef HRESULT(__cdecl* rwD3D9SetPixelShaderConstant)(uint32_t registerAddress, const void* constantData, uint32_t constantCount);
rwD3D9SetPixelShaderConstant g_rwD3D9SetPixelShaderConstant = (rwD3D9SetPixelShaderConstant)0x7FAD00;

typedef HRESULT(__cdecl* rwD3D9SetTexture)(RwTexture* texture, uint32_t stage);
rwD3D9SetTexture g_rwD3D9SetTexture = (rwD3D9SetTexture)0x7FDE70;