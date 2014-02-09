/*
rewritten to C
*/

#define C_D3DSHADERS

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "ScalingEffect.h"


extern "C" void fatal(const char *format, ...);
extern "C" void pclog(const char *format, ...);




    float				m_scale;
    const char*				m_strName;
    LPDIRECT3DDEVICE9		m_pd3dDevice = NULL;

 //   LPD3DXEFFECT			m_pEffect = NULL;

    // Matrix Handles
    D3DXHANDLE m_MatWorldEffectHandle = NULL;
    D3DXHANDLE m_MatViewEffectHandle = NULL;
    D3DXHANDLE m_MatProjEffectHandle = NULL;
    D3DXHANDLE m_MatWorldViewEffectHandle = NULL;
    D3DXHANDLE m_MatViewProjEffectHandle = NULL;
    D3DXHANDLE m_MatWorldViewProjEffectHandle = NULL;

    // Texture Handles
    D3DXHANDLE m_SourceDimsEffectHandle = NULL;
    D3DXHANDLE m_TexelSizeEffectHandle = NULL;
    D3DXHANDLE m_SourceTextureEffectHandle = NULL;
    D3DXHANDLE m_WorkingTexture1EffectHandle = NULL;
    D3DXHANDLE m_WorkingTexture2EffectHandle = NULL;
    D3DXHANDLE m_Hq2xLookupTextureHandle = NULL;

    // Technique stuff
    D3DXHANDLE	m_PreprocessTechnique1EffectHandle = NULL;
    D3DXHANDLE	m_PreprocessTechnique2EffectHandle = NULL;
    D3DXHANDLE	m_CombineTechniqueEffectHandle = NULL;

    //extern LPCSTR getName() { return m_strName; }
    //extern float getScale() { return m_scale; }

    LPD3DXEFFECT			m_pEffect ;
    D3DXEFFECT_DESC			m_EffectDesc ;




#define SAFE_RELEASE(p)		{ if(p) { (p)->Release(); (p)=NULL; } }

void ScalingEffectKillThis(char* src)
{
    //pclog("ScalingEffectKillThis from %s\n", src);
    SAFE_RELEASE(m_pEffect);
    //m_strName = "Unnamed";

    m_MatWorldEffectHandle = 0;
    m_MatViewEffectHandle = 0;
    m_MatProjEffectHandle = 0;
    m_MatWorldViewEffectHandle = 0;
    m_MatViewProjEffectHandle = 0;
    m_MatWorldViewProjEffectHandle = 0;

    // Source Texture Handles
    m_SourceDimsEffectHandle = 0;
    m_TexelSizeEffectHandle = 0;
    m_SourceTextureEffectHandle = 0;
    m_WorkingTexture1EffectHandle = 0;
    m_WorkingTexture2EffectHandle = 0;
    m_Hq2xLookupTextureHandle = 0;

    // Technique stuff
    m_PreprocessTechnique1EffectHandle = 0;
    m_PreprocessTechnique2EffectHandle = 0;
    m_CombineTechniqueEffectHandle = 0;
}

BOOL m_hasPreprocess() { return m_PreprocessTechnique1EffectHandle!=0; }
BOOL m_hasPreprocess2() { return m_PreprocessTechnique2EffectHandle!=0; }

HRESULT ScalingEffectLoadEffect(LPDIRECT3DDEVICE9 i_pd3dDevice, const char *filename)
{
    ScalingEffectKillThis("ScalingEffectLoadEffect");

    m_pd3dDevice = i_pd3dDevice;

    LPD3DXBUFFER		lpBufferEffect = 0;
    LPD3DXBUFFER		lpErrors = 0;
    LPD3DXEFFECTCOMPILER	lpEffectCompiler = 0;


    // First create an effect compiler
    HRESULT hr = D3DXCreateEffectCompilerFromFile(filename, NULL, NULL, 0,
						&lpEffectCompiler, &lpErrors);

    // Errors...
    if(FAILED(hr))
    {
            pclog("D3DXCreateEffectCompilerFromFile failed\n");
            if(lpErrors) {
                    pclog("Unable to create effect compiler from %s,%s\n", filename, (char*) lpErrors->GetBufferPointer());
                }
    }

    if(SUCCEEDED(hr))
    {
        #ifdef C_D3DSHADERS_COMPILE_WITH_DEBUG
        hr = lpEffectCompiler->CompileEffect(D3DXSHADER_DEBUG, &lpBufferEffect, &lpErrors);
        #else
        hr = lpEffectCompiler->CompileEffect(D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, &lpBufferEffect, &lpErrors);
        #endif

        // Errors...
        if(FAILED(hr)){
                pclog("CompileEffect failed\n");
                if(lpErrors) {
                    pclog("Unable to create effect compiler from %s,%s\n", filename, (char*) lpErrors->GetBufferPointer());
                    SAFE_RELEASE(lpErrors);
                }
            }
    }

    if(SUCCEEDED(hr))
    {
            hr = D3DXCreateEffect(  m_pd3dDevice,
                                    lpBufferEffect->GetBufferPointer(),
                                    lpBufferEffect->GetBufferSize(),
                                    NULL, NULL,
                                    0,
                                    NULL, &m_pEffect, &lpErrors);

            // Errors...
            if(FAILED(hr)) {
                    pclog("D3DXCreateEffect failed\n");
                    if(lpErrors) {
                        pclog("Unable to create effect compiler from %s,%s\n", filename, (char*) lpErrors->GetBufferPointer());
                        SAFE_RELEASE(lpErrors);
                    }
            }
    }

    if(SUCCEEDED(hr)) {
        m_pEffect->GetDesc(&m_EffectDesc);
        hr = ScalingEffectParseParameters(lpEffectCompiler);
    }

    SAFE_RELEASE(lpErrors);
    SAFE_RELEASE(lpBufferEffect);
    SAFE_RELEASE(lpEffectCompiler);
    pclog("Loaded effect: %s\n", filename);
    return hr;
}

HRESULT ScalingEffectParseParameters(LPD3DXEFFECTCOMPILER lpEffectCompiler)
{
    HRESULT hr = S_OK;


    if(m_pEffect == NULL)
        return E_FAIL;


    // Look at parameters for semantics and annotations that we know how to interpret
    D3DXPARAMETER_DESC ParamDesc;
    D3DXPARAMETER_DESC AnnotDesc;
    D3DXHANDLE hParam;
    D3DXHANDLE hAnnot;
    LPDIRECT3DBASETEXTURE9 pTex = NULL;

    for(UINT iParam = 0; iParam < m_EffectDesc.Parameters; iParam++) {
        LPCSTR pstrName = NULL;
        LPCSTR pstrFunction = NULL;
        LPCSTR pstrTarget = NULL;
        LPCSTR pstrTextureType = NULL;
        INT Width = D3DX_DEFAULT;
        INT Height= D3DX_DEFAULT;
        INT Depth = D3DX_DEFAULT;

        hParam = m_pEffect->GetParameter(NULL, iParam);
        m_pEffect->GetParameterDesc(hParam, &ParamDesc);

	if(ParamDesc.Semantic != NULL) {
	    if(ParamDesc.Class == D3DXPC_MATRIX_ROWS || ParamDesc.Class == D3DXPC_MATRIX_COLUMNS) {
		if(strcmpi(ParamDesc.Semantic, "world") == 0)
		    m_MatWorldEffectHandle = hParam;
		else if(strcmpi(ParamDesc.Semantic, "view") == 0)
		    m_MatViewEffectHandle = hParam;
		else if(strcmpi(ParamDesc.Semantic, "projection") == 0)
		    m_MatProjEffectHandle = hParam;
		else if(strcmpi(ParamDesc.Semantic, "worldview") == 0)
		    m_MatWorldViewEffectHandle = hParam;
		else if(strcmpi(ParamDesc.Semantic, "viewprojection") == 0)
		    m_MatViewProjEffectHandle = hParam;
		else if(strcmpi(ParamDesc.Semantic, "worldviewprojection") == 0)
		    m_MatWorldViewProjEffectHandle = hParam;
	    }

	    else if(ParamDesc.Class == D3DXPC_VECTOR && ParamDesc.Type == D3DXPT_FLOAT) {
		if(strcmpi(ParamDesc.Semantic, "sourcedims") == 0)
		    m_SourceDimsEffectHandle = hParam;
		else if(strcmpi(ParamDesc.Semantic, "texelsize") == 0)
		    m_TexelSizeEffectHandle = hParam;
	    }

	    else if(ParamDesc.Class == D3DXPC_SCALAR && ParamDesc.Type == D3DXPT_FLOAT) {
		if(strcmpi(ParamDesc.Semantic, "SCALING") == 0)
		    m_pEffect->GetFloat(hParam, &m_scale);
	    }

	    else if(ParamDesc.Class == D3DXPC_OBJECT && ParamDesc.Type == D3DXPT_TEXTURE) {
		if(strcmpi(ParamDesc.Semantic, "SOURCETEXTURE") == 0)
		    m_SourceTextureEffectHandle = hParam;
		if(strcmpi(ParamDesc.Semantic, "WORKINGTEXTURE") == 0)
		    m_WorkingTexture1EffectHandle = hParam;
		if(strcmpi(ParamDesc.Semantic, "WORKINGTEXTURE1") == 0)
		    m_WorkingTexture2EffectHandle = hParam;
		if(strcmpi(ParamDesc.Semantic, "HQ2XLOOKUPTEXTURE") == 0)
		    m_Hq2xLookupTextureHandle = hParam;
	    }

	    else if(ParamDesc.Class == D3DXPC_OBJECT && ParamDesc.Type == D3DXPT_STRING) {
	        LPCSTR pstrTechnique = NULL;

		if(strcmpi(ParamDesc.Semantic, "COMBINETECHNIQUE") == 0) {
		    m_pEffect->GetString(hParam, &pstrTechnique);
		    m_CombineTechniqueEffectHandle = m_pEffect->GetTechniqueByName(pstrTechnique);
		}
		else if(strcmpi(ParamDesc.Semantic, "PREPROCESSTECHNIQUE") == 0) {
		    m_pEffect->GetString(hParam, &pstrTechnique);
		    m_PreprocessTechnique1EffectHandle = m_pEffect->GetTechniqueByName(pstrTechnique);
		}
		else if(strcmpi(ParamDesc.Semantic, "PREPROCESSTECHNIQUE1") == 0) {
		    m_pEffect->GetString(hParam, &pstrTechnique);
		    m_PreprocessTechnique2EffectHandle = m_pEffect->GetTechniqueByName(pstrTechnique);
		}
		else if(strcmpi(ParamDesc.Semantic, "NAME") == 0)
		    m_pEffect->GetString(hParam, &m_strName);

	    }


	}
	for(UINT iAnnot = 0; iAnnot < ParamDesc.Annotations; iAnnot++) {
            hAnnot = m_pEffect->GetAnnotation (hParam, iAnnot);
            m_pEffect->GetParameterDesc(hAnnot, &AnnotDesc);
            if(strcmpi(AnnotDesc.Name, "name") == 0)
                m_pEffect->GetString(hAnnot, &pstrName);
            else if(strcmpi(AnnotDesc.Name, "function") == 0)
                m_pEffect->GetString(hAnnot, &pstrFunction);
            else if(strcmpi(AnnotDesc.Name, "target") == 0)
                m_pEffect->GetString(hAnnot, &pstrTarget);
            else if(strcmpi(AnnotDesc.Name, "width") == 0)
                m_pEffect->GetInt(hAnnot, &Width);
            else if(strcmpi(AnnotDesc.Name, "height") == 0)
                m_pEffect->GetInt(hAnnot, &Height);
            else if(strcmpi(AnnotDesc.Name, "depth") == 0)
                m_pEffect->GetInt(hAnnot, &Depth);
            else if(strcmpi(AnnotDesc.Name, "type") == 0)
                m_pEffect->GetString(hAnnot, &pstrTextureType);

        }
	if(pstrFunction != NULL) {
	    LPD3DXBUFFER pTextureShader = NULL;
	    LPD3DXBUFFER lpErrors = 0;

	    if(pstrTarget == NULL || strcmp(pstrTarget,"tx_1_1"))
                pstrTarget = "tx_1_0";

	    if(SUCCEEDED(hr = lpEffectCompiler->CompileShader(
				pstrFunction, pstrTarget,
				0, &pTextureShader, &lpErrors, NULL))) {
		SAFE_RELEASE(lpErrors);

		if(Width == D3DX_DEFAULT)
                    Width = 64;
		if(Height == D3DX_DEFAULT)
                    Height = 64;
		if(Depth == D3DX_DEFAULT)
                    Depth = 64;

#if D3DX_SDK_VERSION >= 22
		LPD3DXTEXTURESHADER ppTextureShader;
		D3DXCreateTextureShader((DWORD *)pTextureShader->GetBufferPointer(), &ppTextureShader);
#endif

		if(pstrTextureType != NULL) {
                    if(strcmpi(pstrTextureType, "volume") == 0) {
                        LPDIRECT3DVOLUMETEXTURE9 pVolumeTex = NULL;
                        if(SUCCEEDED(hr = D3DXCreateVolumeTexture(m_pd3dDevice,
                        	Width, Height, Depth, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pVolumeTex))) {

                    	    if(SUCCEEDED(hr = D3DXFillVolumeTextureTX(pVolumeTex, ppTextureShader))) {
                                pTex = pVolumeTex;
                            }
                        }
                    } else if(strcmpi(pstrTextureType, "cube") == 0) {
                        LPDIRECT3DCUBETEXTURE9 pCubeTex = NULL;
                        if(SUCCEEDED(hr = D3DXCreateCubeTexture(m_pd3dDevice,
                        	Width, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pCubeTex))) {
#if D3DX_SDK_VERSION >= 22
                            if(SUCCEEDED(hr = D3DXFillCubeTextureTX(pCubeTex, ppTextureShader))) {
#endif
                                pTex = pCubeTex;
                            }
                        }
                    }
		} else {
                    LPDIRECT3DTEXTURE9 p2DTex = NULL;
                    if(SUCCEEDED(hr = D3DXCreateTexture(m_pd3dDevice, Width, Height,
                    	    D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &p2DTex))) {
#if D3DX_SDK_VERSION >= 22
                        if(SUCCEEDED(hr = D3DXFillTextureTX(p2DTex, ppTextureShader))) {
#endif
                            pTex = p2DTex;
                        }
                    }
		}
		m_pEffect->SetTexture(m_pEffect->GetParameter(NULL, iParam), pTex);
		SAFE_RELEASE(pTex);
		SAFE_RELEASE(pTextureShader);
#if D3DX_SDK_VERSION >= 22
		SAFE_RELEASE(ppTextureShader);
#endif
	    } else {
            if(lpErrors) {
                pclog("Could not compile texture shader %s\n", (char*) lpErrors->GetBufferPointer());
            }
            SAFE_RELEASE(lpErrors);
            return hr;
            }
        }
    }
    return S_OK;
}

// Set Source texture
HRESULT ScalingEffectSetTextures(LPDIRECT3DTEXTURE9 lpSource, LPDIRECT3DTEXTURE9 lpWorking1,
			LPDIRECT3DTEXTURE9 lpWorking2, LPDIRECT3DVOLUMETEXTURE9 lpHq2xLookupTexture)
{
    if(!m_SourceTextureEffectHandle) {
        pclog("Texture with SOURCETEXTURE semantic not found\n");
        return E_FAIL;
    }

    HRESULT hr = m_pEffect->SetTexture(m_SourceTextureEffectHandle, lpSource);

    if(FAILED(hr)) {
        pclog("Unable to set SOURCETEXTURE\n");
        return hr;
    }

    if(m_WorkingTexture1EffectHandle) {
        hr = m_pEffect->SetTexture(m_WorkingTexture1EffectHandle, lpWorking1);

        if(FAILED(hr)) {
            pclog("Unable to set WORKINGTEXTURE1\n");
            return hr;
        }
    }

    if(m_WorkingTexture2EffectHandle) {
        hr = m_pEffect->SetTexture(m_WorkingTexture2EffectHandle, lpWorking2);

        if(FAILED(hr)) {
            pclog("Unable to set WORKINGTEXTURE2\n");
            return hr;
        }
    }

    if(m_Hq2xLookupTextureHandle) {
        hr = m_pEffect->SetTexture(m_Hq2xLookupTextureHandle, lpHq2xLookupTexture);

        if(FAILED(hr)) {
            pclog("Unable to set HQ2XLOOKUPTEXTURE\n");
            return hr;
        }
    }

    D3DXVECTOR4 fDims(256,256,1,1), fTexelSize(1,1,1,1);

    if(lpSource) {
        D3DSURFACE_DESC Desc;
        lpSource->GetLevelDesc(0, &Desc);
        fDims[0] = (FLOAT) Desc.Width;
        fDims[1] = (FLOAT) Desc.Height;
    }

    fTexelSize[0] = 1/fDims[0];
    fTexelSize[1] = 1/fDims[1];

    if(m_SourceDimsEffectHandle) {
        hr = m_pEffect->SetVector(m_SourceDimsEffectHandle, &fDims);

        if(FAILED(hr)) {
            pclog("Unable to set SOURCEDIMS\n");
            return hr;
        }
    }

    if(m_TexelSizeEffectHandle) {
        hr = m_pEffect->SetVector(m_TexelSizeEffectHandle, &fTexelSize);

        if(FAILED(hr)) {
            pclog("Unable to set TEXELSIZE\n");
            return hr;
        }
    }

    return hr;
}

void PrintD3DMatrix(const char* name, D3DXMATRIX &mat)
{
	char buf[300];
	pclog("Printing D3D Matrix: - %s\n"
	"\t %f \t %f \t %f \t %f\n"
	"\t %f \t %f \t %f \t %f\n"
	"\t %f \t %f \t %f \t %f\n"
	"\t %f \t %f \t %f \t %f\n", name,
	mat._11, mat._12, mat._13, mat._14,
	mat._21, mat._22, mat._23, mat._24,
	mat._31, mat._32, mat._33, mat._34,
	mat._41, mat._42, mat._43, mat._44);
}


// Set the Matrices for this frame
HRESULT ScalingEffectSetMatrices(D3DXMATRIX &matProj, D3DXMATRIX &matView, D3DXMATRIX &matWorld)
{
    HRESULT hr = S_OK;
    if(m_MatWorldEffectHandle != NULL) {
        hr = m_pEffect->SetMatrix(m_MatWorldEffectHandle, &matWorld);
        //PrintD3DMatrix("matWorld",matWorld);
        if(FAILED(hr)) {
            pclog("Unable to set WORLD matrix\n");
            return hr;
        }
    }

    if(m_MatViewEffectHandle != NULL) {
        hr = m_pEffect->SetMatrix(m_MatViewEffectHandle, &matView);
        //PrintD3DMatrix("matView",matView);
        if(FAILED(hr)) {
            pclog("Unable to set VIEW matrix\n");
            return hr;
        }
    }

    if(m_MatProjEffectHandle != NULL) {
        hr = m_pEffect->SetMatrix(m_MatProjEffectHandle, &matProj);
        //PrintD3DMatrix("matProj",matProj);
        if(FAILED(hr)) {
            pclog("Unable to set PROJECTION matrix\n");
            return hr;
        }
    }

    if(m_MatWorldViewEffectHandle != NULL) {
        D3DXMATRIX matWorldView = matWorld * matView;
        hr = m_pEffect->SetMatrix(m_MatWorldViewEffectHandle, &matWorldView);
        //PrintD3DMatrix("matWorldView = matWorld * matView",matWorldView);
        if(FAILED(hr)) {
            pclog("Unable to set WORLDVIEW matrix\n");
            return hr;
        }
    }

    if(m_MatViewProjEffectHandle != NULL) {
        D3DXMATRIX matViewProj = matView * matProj;
        hr = m_pEffect->SetMatrix(m_MatViewProjEffectHandle, &matViewProj);
        //PrintD3DMatrix("matViewProj = matView * matProj",matViewProj);
        if(FAILED(hr)) {
            pclog("Unable to set VIEWPROJECTION matrix\n");
            return hr;
        }
    }

    if(m_MatWorldViewProjEffectHandle != NULL) {
        D3DXMATRIX matWorldViewProj = matWorld * matView * matProj;
        hr = m_pEffect->SetMatrix(m_MatWorldViewProjEffectHandle, &matWorldViewProj);
        //PrintD3DMatrix("matWorldViewProj = matWorld * matView * matProj",matWorldViewProj);
        if(FAILED(hr)) {
            pclog("Unable to set WORLDVIEWPROJECTION matrix\n");
            return hr;
        }
    }

    return hr;
}


// Returns Number of passes for this technique
HRESULT ScalingEffectBegin(Pass pass, UINT* pPasses)
{
    HRESULT hr = S_OK;
    if (m_pEffect==NULL) E_FAIL;

    switch (pass) {
        case Preprocess1:
            //pclog("SetTechnique Preprocess1\n");
            hr = m_pEffect->SetTechnique(m_PreprocessTechnique1EffectHandle);
            break;
        case Preprocess2:
            //pclog("SetTechnique Preprocess2\n");
            hr = m_pEffect->SetTechnique(m_PreprocessTechnique2EffectHandle);
            break;
        case Combine:
            //pclog("SetTechnique Combine\n");
            hr = m_pEffect->SetTechnique(m_CombineTechniqueEffectHandle);
            break;
    }

    if(FAILED(hr)) {
        pclog("SetTechnique failed %s, %s\n",DXGetErrorString9(hr), DXGetErrorDescription9(hr));
        return E_FAIL;
    }

    return m_pEffect->Begin(pPasses, D3DXFX_DONOTSAVESTATE|D3DXFX_DONOTSAVESHADERSTATE);
}

// Render Pass in technique
HRESULT ScalingEffectBeginPass(UINT Pass)
{
    return m_pEffect->BeginPass(Pass);
}

// End Pass of this technique
HRESULT ScalingEffectEndPass()
{
    return m_pEffect->EndPass();
}

// End rendering of this technique
HRESULT ScalingEffectEnd()
{
    return m_pEffect->End();
}

HRESULT ScalingEffectCommitChanges()
{
    return m_pEffect->CommitChanges();
}

// Validates the effect
HRESULT ScalingEffectValidate()
{
    HRESULT hr;

    // Now we check to see if they all exist
    if(!m_MatWorldEffectHandle || !m_MatWorldViewEffectHandle ||
		!m_MatWorldViewProjEffectHandle) {
        pclog("Effect doesn't have any world matrix handles\n");
        return E_FAIL;
    }

    if(!m_MatViewEffectHandle || !m_MatWorldViewEffectHandle ||
		!m_MatViewProjEffectHandle || !m_MatWorldViewProjEffectHandle) {
        pclog("Effect doesn't have any view matrix handles\n");
        return E_FAIL;
    }

    if(!m_MatProjEffectHandle || !m_MatViewProjEffectHandle ||
		!m_MatWorldViewProjEffectHandle) {
        pclog("Effect doesn't have any projection matrix handles\n");
        return E_FAIL;
    }

    if(!m_SourceTextureEffectHandle) {
        pclog("Effect doesn't have a SOURCETEXTURE handle\n");
        return E_FAIL;
    }

    if(!m_CombineTechniqueEffectHandle) {
        pclog("Effect doesn't have a COMBINETECHNIQUE handle\n");
        return E_FAIL;
    }

    if(!m_WorkingTexture1EffectHandle && m_PreprocessTechnique1EffectHandle) {
        pclog("Effect doesn't have a WORKINGTEXTURE handle but uses preprocess steps\n");
        return E_FAIL;
    }

    if(!m_WorkingTexture2EffectHandle && m_PreprocessTechnique2EffectHandle) {
        pclog("Effect doesn't have a WORKINGTEXTURE1 handle but uses 2 preprocess steps\n");
        return E_FAIL;
    }

    // Validate this
    if(m_PreprocessTechnique1EffectHandle) {
        hr = m_pEffect->ValidateTechnique(m_PreprocessTechnique1EffectHandle);

	if(FAILED(hr)) {
            pclog("ValidateTechnique for PREPROCESSTECHNIQUE failed\n");
            return hr;
        }
    }

    if(m_PreprocessTechnique2EffectHandle) {
        hr = m_pEffect->ValidateTechnique(m_PreprocessTechnique2EffectHandle);

        if(FAILED(hr)) {
            pclog("ValidateTechnique for PREPROCESSTECHNIQUE1 failed\n");
            return hr;
        }
    }

    hr = m_pEffect->ValidateTechnique(m_CombineTechniqueEffectHandle);

    if(FAILED(hr)) {
        pclog("ValidateTechnique for COMBINETECHNIQUE failed\n");
        return hr;
    }

    return S_OK;
}

