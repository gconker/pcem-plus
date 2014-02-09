#include <stdint.h>
#include <d3d9.h>
#include <d3dx9.h>

#ifdef __cplusplus
extern "C" {
#endif

    HRESULT ScalingEffectLoadEffect(LPDIRECT3DDEVICE9 i_pd3dDevice, const char *filename);

     // Does it have a preprocess step
    BOOL m_hasPreprocess();
    BOOL m_hasPreprocess2();

    // Set The Textures
    HRESULT ScalingEffectSetTextures( LPDIRECT3DTEXTURE9 lpSource, LPDIRECT3DTEXTURE9 lpWorking1,
		LPDIRECT3DTEXTURE9 lpWorking2, LPDIRECT3DVOLUMETEXTURE9 lpHq2xLookupTexture );

    // Set the Matrices for this frame
    HRESULT ScalingEffectSetMatrices( D3DXMATRIX &matProj, D3DXMATRIX &matView, D3DXMATRIX &matWorld );

    enum Pass { Preprocess1, Preprocess2, Combine };
    // Begin the technique
    // Returns Number of passes for this technique
    HRESULT ScalingEffectBegin(Pass pass, UINT* pPasses);

    // Render Pass in technique
    HRESULT ScalingEffectBeginPass(UINT Pass);
    HRESULT ScalingEffectEndPass();

    // End rendering of this technique
    HRESULT ScalingEffectEnd();

    // Validates the effect
    HRESULT ScalingEffectValidate();

    HRESULT ScalingEffectParseParameters(LPD3DXEFFECTCOMPILER lpEffectCompiler);

    void ScalingEffectKillThis(char* src);

    void PrintD3DMatrix(const char* name, D3DXMATRIX &mat);
    HRESULT ScalingEffectCommitChanges();
#ifdef __cplusplus
}
#endif

