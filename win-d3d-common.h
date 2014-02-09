#define SAFE_RELEASE(p)		{ if(p) { (p)->Release(); (p)=NULL; } }

#ifdef __cplusplus
extern "C" {
#endif
struct TLVERTEX
{
        D3DXVECTOR3 Position;       // vertex position
        D3DCOLOR    Diffuse;
        D3DXVECTOR2 TexCoord;       // texture coords
};

 // Projection matrices
static D3DXMATRIX			m_matProj;
static D3DXMATRIX			m_matWorld;
static D3DXMATRIX			m_matView;

static D3DXMATRIX			m_matPreProj  ;
static D3DXMATRIX			m_matPreView  ;
static D3DXMATRIX			m_matPreWorld  ;

// Pixel shader
static LPDIRECT3DTEXTURE9		lpWorkTexture1 ;
static LPDIRECT3DTEXTURE9		lpWorkTexture2 ;
static LPDIRECT3DVOLUMETEXTURE9	lpHq2xLookupTexture ;

#define D3DFVF_TLVERTEX D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1

static bool psActive ;
const double dwTexHeight = 2048;
const double dwTexWidth = 2048;
const double dwScaledWidth = 2048;
const double dwScaledHeight = 2048;
const double dwWidth = 2048 ;
const double dwHeight = 2048 ;

#ifdef __cplusplus
}
#endif
