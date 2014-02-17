#include <stdint.h>
#define BITMAP WINDOWS_BITMAP
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#undef BITMAP
#include "resources.h"
#include "video.h"
#include "win-d3d-fs.h"
#include "win.h"
#include "ScalingEffect.h"
#include "hq2x_d3d.h"
#include "win-d3d-common.h"
#define SAFE_RELEASE(p)		{ if(p) { (p)->Release(); (p)=NULL; } }

extern "C" void fatal(const char *format, ...);
extern "C" void pclog(const char *format, ...);

extern "C" void device_force_redraw();

static void d3d_fs_init_objects();
static void d3d_fs_close_objects();
static void d3d_fs_blit_memtoscreen(int x, int y, int y1, int y2, int w, int h);
static void d3d_fs_blit_memtoscreen_8(int x, int y, int w, int h);

static LPDIRECT3D9             d3d        = NULL;
static LPDIRECT3DDEVICE9       d3ddev     = NULL;
static LPDIRECT3DVERTEXBUFFER9 v_buffer   = NULL;
static LPDIRECT3DTEXTURE9      d3dTexture = NULL;
static D3DPRESENT_PARAMETERS d3dpp;

bool D3DSwapBuffers_fs(void);
HRESULT SetEffectMatrices_fs(void);
HRESULT CreateEffectTextures_fs(void);
HRESULT SetVertexData_fs(float sizex, float sizey);
void SetupSceneScaled_fs(int w, int h);
void loadScalingEffect_fs();

static HWND d3d_hwnd;
static HWND d3d_device_window;

static int d3d_fs_w, d3d_fs_h;

static PALETTE cgapal=
{
        {0,0,0},{0,42,0},{42,0,0},{42,21,0},
        {0,0,0},{0,42,42},{42,0,42},{42,42,42},
        {0,0,0},{21,63,21},{63,21,21},{63,63,21},
        {0,0,0},{21,63,63},{63,21,63},{63,63,63},

        {0,0,0},{0,0,42},{0,42,0},{0,42,42},
        {42,0,0},{42,0,42},{42,21,00},{42,42,42},
        {21,21,21},{21,21,63},{21,63,21},{21,63,63},
        {63,21,21},{63,21,63},{63,63,21},{63,63,63},

        {0,0,0},{0,21,0},{0,0,42},{0,42,42},
        {42,0,21},{21,10,21},{42,0,42},{42,0,63},
        {21,21,21},{21,63,21},{42,21,42},{21,63,63},
        {63,0,0},{42,42,0},{63,21,42},{41,41,41},

        {0,0,0},{0,42,42},{42,0,0},{42,42,42},
        {0,0,0},{0,42,42},{42,0,0},{42,42,42},
        {0,0,0},{0,63,63},{63,0,0},{63,63,63},
        {0,0,0},{0,63,63},{63,0,0},{63,63,63},
};

static uint32_t pal_lookup[256];

void d3d_fs_init(HWND h)
{
        pclog("init device fullscreen\n");
        int c;
        HRESULT hr;

        d3d_fs_w = GetSystemMetrics(SM_CXSCREEN);
        d3d_fs_h = GetSystemMetrics(SM_CYSCREEN);

        // init variables
        psActive = true;

        for (c = 0; c < 256; c++)
            pal_lookup[c] = makecol(cgapal[c].r << 2, cgapal[c].g << 2, cgapal[c].b << 2);

        d3d_hwnd = h;

        d3d_device_window = CreateWindowEx (
                0,
                szSubClassName,
                "PCem v8.1",
                WS_POPUP,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                d3d_fs_w,
                d3d_fs_h,
                HWND_DESKTOP,
                NULL,
                NULL,
                NULL
        );

        d3d = Direct3DCreate9(D3D_SDK_VERSION);

        memset(&d3dpp, 0, sizeof(d3dpp));

        d3dpp.Flags                  = 0;
        d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow          = d3d_device_window;
        d3dpp.BackBufferCount        = 1;
        d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;
        d3dpp.MultiSampleQuality     = 0;
        d3dpp.EnableAutoDepthStencil = false;
        d3dpp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
        d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
        d3dpp.Windowed               = false;
        d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
        d3dpp.BackBufferWidth        = d3d_fs_w;
        d3dpp.BackBufferHeight       = d3d_fs_h;


        D3DCAPS9 d3dCaps;
        // get device capabilities
        ZeroMemory(&d3dCaps, sizeof(d3dCaps));
        if(FAILED(d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps)))
        {
           pclog("Failed to get Device caps");
           return ;
        }
        // Check if hardware vertex processing is available
        if(d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
            hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, h, D3DCREATE_HARDWARE_VERTEXPROCESSING|0x00000800L|D3DCREATE_FPU_PRESERVE, &d3dpp, &d3ddev);
        }
        else {
            hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, h, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
        }

        d3d_fs_init_objects();

        video_blit_memtoscreen   = d3d_fs_blit_memtoscreen;
        video_blit_memtoscreen_8 = d3d_fs_blit_memtoscreen_8;
        if (psActive)
        {
            loadScalingEffect_fs();
            CreateEffectTextures_fs();
		}
}
static void d3d_fs_close_objects()
{
        if (d3ddev) d3ddev->SetStreamSource(0, NULL, 0, 0);
        SAFE_RELEASE(d3dTexture);
        SAFE_RELEASE(v_buffer);
}

static void d3d_fs_init_objects()
{
        HRESULT hr;
        D3DLOCKED_RECT dr;
        int y;
        RECT r;

        hr = d3ddev->CreateVertexBuffer(8*sizeof(TLVERTEX),
                                   D3DUSAGE_WRITEONLY,
                                   D3DFVF_TLVERTEX,
                                   D3DPOOL_MANAGED,
                                   &v_buffer,
                                   NULL);

        d3ddev->CreateTexture(dwTexHeight, dwTexWidth, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &d3dTexture, NULL);
        r.top    = r.left  = 0;
        r.bottom = r.right = dwTexWidth-1;

        if (FAILED(d3dTexture->LockRect(0, &dr, &r, 0)))
           fatal("LockRect failed\n");

        for (y = 0; y < dwTexHeight; y++)
        {
                uint32_t *p = (uint32_t *)(dr.pBits + (y * dr.Pitch));
                memset(p, 0, dwTexWidth * 4);
        }

        d3dTexture->UnlockRect(0);

        d3ddev->SetTextureStageState(0,D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
        d3ddev->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
        d3ddev->SetTextureStageState(0,D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

        d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
}

/*void d3d_resize(int x, int y)
{
        HRESULT hr;

        d3dpp.BackBufferWidth  = x;
        d3dpp.BackBufferHeight = y;

        d3d_reset();
}*/

void d3d_fs_reset()
{
        pclog("d3d_reset\n");
        HRESULT hr;

        memset(&d3dpp, 0, sizeof(d3dpp));

        d3dpp.Flags                  = 0;
        d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow          = d3d_device_window;
        d3dpp.BackBufferCount        = 1;
        d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;
        d3dpp.MultiSampleQuality     = 0;
        d3dpp.EnableAutoDepthStencil = false;
        d3dpp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
        d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
        d3dpp.Windowed               = false;
        d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
        d3dpp.BackBufferWidth        = d3d_fs_w;
        d3dpp.BackBufferHeight       = d3d_fs_h;

        d3d_fs_close_objects();
        if (psActive)
        {
            ScalingEffectKillThis("d3d_reset");
            SAFE_RELEASE(lpWorkTexture1);
            SAFE_RELEASE(lpWorkTexture2);
            SAFE_RELEASE(lpHq2xLookupTexture);
        }
        hr = d3ddev->Reset(&d3dpp);

        if (hr == D3DERR_DEVICELOST)
                return;
        pclog("d3ddev->Reset %s, %s\n",DXGetErrorString9(hr), DXGetErrorDescription9(hr));
        d3ddev->SetTextureStageState(0,D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
        d3ddev->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
        d3ddev->SetTextureStageState(0,D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

        d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

        d3d_fs_init_objects();
        if (psActive)
        {
            loadScalingEffect_fs();
            CreateEffectTextures_fs();
        }
        device_force_redraw();
}

void d3d_fs_close()
{
        if (psActive)
        {
            ScalingEffectKillThis("d3d_close");
            SAFE_RELEASE(lpWorkTexture1);
            SAFE_RELEASE(lpWorkTexture2);
            SAFE_RELEASE(lpHq2xLookupTexture);
        }

        d3d_fs_close_objects();
        SAFE_RELEASE(d3ddev);
        SAFE_RELEASE(d3d);
        DestroyWindow(d3d_device_window);
}

static void d3d_fs_size(RECT window_rect, double *l, double *t, double *r, double *b, int w, int h)
{
        int ratio_w, ratio_h;
        switch (video_fullscreen_scale)
        {
                case FULLSCR_SCALE_FULL:
                *l = -0.5;
                *t = -0.5;
                *r = (window_rect.right  - window_rect.left) - 0.5;
                *b = (window_rect.bottom - window_rect.top) - 0.5;
                break;
                case FULLSCR_SCALE_43:
                *t = -0.5;
                *b = (window_rect.bottom - window_rect.top) - 0.5;
                *l = ((window_rect.right  - window_rect.left) / 2) - (((window_rect.bottom - window_rect.top) * 4) / (3 * 2)) - 0.5;
                *r = ((window_rect.right  - window_rect.left) / 2) + (((window_rect.bottom - window_rect.top) * 4) / (3 * 2)) - 0.5;
                if (*l < -0.5)
                {
                        *l = -0.5;
                        *r = (window_rect.right  - window_rect.left) - 0.5;
                        *t = ((window_rect.bottom - window_rect.top) / 2) - (((window_rect.right - window_rect.left) * 3) / (4 * 2)) - 0.5;
                        *b = ((window_rect.bottom - window_rect.top) / 2) + (((window_rect.right - window_rect.left) * 3) / (4 * 2)) - 0.5;
                }
                break;
                case FULLSCR_SCALE_SQ:
                *t = -0.5;
                *b = (window_rect.bottom - window_rect.top) - 0.5;
                *l = ((window_rect.right  - window_rect.left) / 2) - (((window_rect.bottom - window_rect.top) * w) / (h * 2)) - 0.5;
                *r = ((window_rect.right  - window_rect.left) / 2) + (((window_rect.bottom - window_rect.top) * w) / (h * 2)) - 0.5;
                if (*l < -0.5)
                {
                        *l = -0.5;
                        *r = (window_rect.right  - window_rect.left) - 0.5;
                        *t = ((window_rect.bottom - window_rect.top) / 2) - (((window_rect.right - window_rect.left) * h) / (w * 2)) - 0.5;
                        *b = ((window_rect.bottom - window_rect.top) / 2) + (((window_rect.right - window_rect.left) * h) / (w * 2)) - 0.5;
                }
                break;
                case FULLSCR_SCALE_INT:
                ratio_w = (window_rect.right  - window_rect.left) / w;
                ratio_h = (window_rect.bottom - window_rect.top)  / h;
                if (ratio_h < ratio_w)
                        ratio_w = ratio_h;
                *l = ((window_rect.right  - window_rect.left) / 2) - ((w * ratio_w) / 2) - 0.5;
                *r = ((window_rect.right  - window_rect.left) / 2) + ((w * ratio_w) / 2) - 0.5;
                *t = ((window_rect.bottom - window_rect.top)  / 2) - ((h * ratio_w) / 2) - 0.5;
                *b = ((window_rect.bottom - window_rect.top)  / 2) + ((h * ratio_w) / 2) - 0.5;
                break;
        }
}

static void d3d_fs_blit_memtoscreen(int x, int y, int y1, int y2, int w, int h)
{
        //pclog("d3d_fs_blit_memtoscreen");
        HRESULT hr = D3D_OK;
        VOID* pVoid;
        D3DLOCKED_RECT dr;
        RECT window_rect;
        uint32_t *p, *src;
        int yy;


        GetClientRect(d3d_device_window, &window_rect);

        float sizex, sizey;
        sizex = (float)w/dwTexWidth;
        sizey = (float)h/dwTexHeight;
        hr = SetVertexData_fs(sizex, sizey);

        window_rect.top    = y1;
        window_rect.left   = 0;
        window_rect.bottom = y2;
        window_rect.right  = 2047;
        if (hr == D3D_OK && !(y1 == 0 && y2 == 0))
        {
                RECT lock_rect;

                lock_rect.top    = y1;
                lock_rect.left   = 0;
                lock_rect.bottom = y2;
                lock_rect.right  = 2047;

                if (FAILED(d3dTexture->LockRect(0, &dr, &lock_rect, 0)))
                   fatal("LockRect failed\n");

                for (yy = y1; yy < y2; yy++)
                        memcpy(dr.pBits + ((yy - y1) * dr.Pitch), &(((uint32_t *)buffer32->line[yy + y])[x]), w * 4);

                d3dTexture->UnlockRect(0);
        }

        if (hr == D3D_OK)
                hr = d3ddev->BeginScene();

        if (hr == D3D_OK)
        {
                if (hr == D3D_OK)
                        d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0, 0);

                SetupSceneScaled_fs(w,h); // calculations of scale; setting up world, projection, m_matView matrixes and setTransformations

                if (hr == D3D_OK)
                        hr = d3ddev->SetTexture(0, d3dTexture);

                if (hr == D3D_OK)
                        hr = d3ddev->SetFVF(D3DFVF_TLVERTEX);

                if (hr == D3D_OK)
                        hr = d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(TLVERTEX));

                if (hr == D3D_OK){
                    if (psActive){
                        SetEffectMatrices_fs();    // copy matrices to effect m_matProj, m_matView, m_matWorld and others
                        D3DSwapBuffers_fs();       // swap buffers and draw primitives
                    }
                    else{
                        hr = d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
                    }
                }
                if (hr == D3D_OK)
                        hr = d3ddev->SetTexture(0, NULL);

                if (hr == D3D_OK)
                        hr = d3ddev->EndScene();
        }

        if (hr == D3D_OK)
                hr = d3ddev->Present(NULL, NULL, d3d_device_window, NULL);

        if (hr == D3DERR_DEVICELOST || hr == D3DERR_INVALIDCALL)
                PostMessage(ghwnd, WM_RESETD3D, 0, 0);
}

static void d3d_fs_blit_memtoscreen_8(int x, int y, int w, int h)
{
        HRESULT hr = D3D_OK;
        VOID* pVoid;
        D3DLOCKED_RECT dr;
        RECT window_rect;
        uint32_t *p, *src;
        int xx, yy;

        GetClientRect(d3d_device_window, &window_rect);

        float sizex, sizey;
        sizex = (float)w/dwTexWidth;
        sizey = (float)h/dwTexHeight;
        hr = SetVertexData_fs(sizex, sizey);

        if (hr == D3D_OK)
        {
                RECT lock_rect;

                lock_rect.top    = 0;
                lock_rect.left   = 0;
                lock_rect.bottom = 2047;
                lock_rect.right  = 2047;

                if (FAILED(d3dTexture->LockRect(0, &dr, &lock_rect, 0)))
                        fatal("LockRect failed\n");

                for (yy = 0; yy < h; yy++)
                {
                        uint32_t *p = (uint32_t *)(dr.pBits + (yy * dr.Pitch));
                        if ((y + yy) >= 0 && (y + yy) < buffer->h)
                        {
                                for (xx = 0; xx < w; xx++)
                                        p[xx] = pal_lookup[buffer->line[y + yy][x + xx]];
                        }
                }

                d3dTexture->UnlockRect(0);
        }

        if (hr == D3D_OK)
                hr = d3ddev->BeginScene();

        if (hr == D3D_OK)
        {
                if (hr == D3D_OK)
                        d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0, 0);
                SetupSceneScaled_fs(w,h); // calculations of scale; setting up world, projection, m_matView matrixes and setTransformations
                if (hr == D3D_OK)
                        hr = d3ddev->SetTexture(0, d3dTexture);

                if (hr == D3D_OK)
                        hr = d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

                if (hr == D3D_OK)
                        hr = d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(TLVERTEX));

                if (hr == D3D_OK){
                    if (psActive){
                        SetEffectMatrices_fs();    // copy matrices to effect m_matProj, m_matView, m_matWorld and others
                        D3DSwapBuffers_fs();       // swap buffers and draw primitives
                    }
                    else{
                        hr = d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
                    }
                }

                if (hr == D3D_OK)
                        hr = d3ddev->SetTexture(0, NULL);

                if (hr == D3D_OK)
                        hr = d3ddev->EndScene();
        }

        if (hr == D3D_OK)
                hr = d3ddev->Present(NULL, NULL, d3d_device_window, NULL);

        if (hr == D3DERR_DEVICELOST || hr == D3DERR_INVALIDCALL)
                PostMessage(ghwnd, WM_RESETD3D, 0, 0);
}


// Draw a textured quad on the back-buffer
bool D3DSwapBuffers_fs(void)
{
    HRESULT hr;
    UINT uPasses;

    /* PS 2.0 path */
    if(psActive) {

        // Set textures

        if(FAILED(ScalingEffectSetTextures(d3dTexture, lpWorkTexture1, lpWorkTexture2, lpHq2xLookupTexture))) {
            pclog("D3D:Failed to set PS textures\n");
            return false;
        }


        // preprocess start
        if(m_hasPreprocess()) {
            //pclog("D3DSwapBuffers::preProcess\n");
            // Set preprocess matrices
            if(FAILED(ScalingEffectSetMatrices(m_matPreProj, m_matPreView, m_matPreWorld))) {
                pclog("D3D:Set matrices failed.\n");
                return false;
            }

            // Save render target
            LPDIRECT3DSURFACE9 lpRenderTarget;

            d3ddev->GetRenderTarget(0, &lpRenderTarget);
            LPDIRECT3DTEXTURE9 lpWorkTexture = lpWorkTexture1;

    pass2:
            // Change the render target
            LPDIRECT3DSURFACE9 lpNewRenderTarget;

            if (lpWorkTexture)
                hr = lpWorkTexture->GetSurfaceLevel(0, &lpNewRenderTarget);
                if (FAILED(hr) || lpNewRenderTarget==NULL)
                    pclog("lpWorkTexture->GetSurfaceLevel %s, %s\n",DXGetErrorString9(hr), DXGetErrorDescription9(hr));

            if(FAILED(hr = d3ddev->SetRenderTarget(0, lpNewRenderTarget))) {
                pclog("D3D:Failed to set render target %s, %s\n",DXGetErrorString9(hr), DXGetErrorDescription9(hr));
                return false;
            }

            SAFE_RELEASE(lpNewRenderTarget);

            uPasses = 0;
            if(FAILED(ScalingEffectBegin((lpWorkTexture==lpWorkTexture1) ?
                (Preprocess1):(Preprocess2), &uPasses))) {
                pclog("D3D:Failed to begin PS\n");
                return false;
            }

            for(UINT uPass=0;uPass<uPasses;uPass++) {
                hr=ScalingEffectBeginPass(uPass);
                if(FAILED(hr)) {
                    pclog("D3D:Failed to begin pass %d\n", uPass);
                    return false;
                }

                // Render the vertex buffer contents
                d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4, 2);
                ScalingEffectEndPass();
            }

            if(FAILED(ScalingEffectEnd())) {
                pclog("D3D:Failed to end effect\n");
                return false;
            }

            if((m_hasPreprocess2()) && (lpWorkTexture == lpWorkTexture1)) {
                lpWorkTexture = lpWorkTexture2;
                goto pass2;
            }

            // Reset the rendertarget
            d3ddev->SetRenderTarget(0, lpRenderTarget);
            SAFE_RELEASE(lpRenderTarget);

            // Set matrices for final pass
            if(FAILED(ScalingEffectSetMatrices(m_matProj, m_matView, m_matWorld))) {
                pclog("D3D:Set matrices failed.\n");
                return false;
            }
        }
// preprocess end
            uPasses = 0;

            if(FAILED(ScalingEffectBegin(Combine, &uPasses))) {
                pclog("D3D:Failed to begin PS\n");
                return false;
            }

            for(UINT uPass=0;uPass<uPasses;uPass++) {
                hr=ScalingEffectBeginPass(uPass);
                if(FAILED(hr)) {
                    pclog("D3D:Failed to begin pass %d\n", uPass);
                    return false;
                }

                d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
                ScalingEffectEndPass();
            }


            if(FAILED(ScalingEffectEnd())) {
                pclog("D3D:Failed to end effect\n");
                return false;
            }

    }
    return true;
}

HRESULT SetEffectMatrices_fs(void)
{
    //pclog("SetEffectMatrices\n");

    if(m_hasPreprocess()) {
    	// Projection is (0,0,0) -> (1,1,1)
	    D3DXMatrixOrthoOffCenterLH(&m_matPreProj, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);

	    // Align texels with pixels
	    D3DXMatrixTranslation(&m_matPreView, -0.5f/dwTexWidth, 0.5f/dwTexHeight, 0.0f);

	    // Identity for world
	    D3DXMatrixIdentity(&m_matPreWorld);
	}
	else if(FAILED(ScalingEffectSetMatrices(m_matProj, m_matView, m_matWorld))) {
	    pclog("D3D:Set matrices failed.\n");
        return E_FAIL;
	}

    return S_OK;
}

HRESULT CreateEffectTextures_fs(void)
{
 //   SAFE_RELEASE(d3dTexture);

    HRESULT hr;
    SAFE_RELEASE(lpWorkTexture1);
    SAFE_RELEASE(lpWorkTexture2);
    SAFE_RELEASE(lpHq2xLookupTexture);

    if(psActive) {
        // Working textures for pixel shader
        if(FAILED(hr=d3ddev->CreateTexture(dwTexWidth, dwTexHeight, 1, D3DUSAGE_RENDERTARGET,
                    D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &lpWorkTexture1, NULL))) {
            pclog("D3D:Failed to create working texture lpWorkTexture1: %s\n", DXGetErrorString9(hr));
            return E_FAIL;
        }

        if(FAILED(hr=d3ddev->CreateTexture(dwTexWidth, dwTexHeight, 1, D3DUSAGE_RENDERTARGET,
                    D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &lpWorkTexture2, NULL))) {
            pclog("D3D:Failed to create working texture lpWorkTexture2: %s\n", DXGetErrorString9(hr));

            return E_FAIL;
        }

        if (m_hasH2qxLookupTexture())
        {
            pclog("Creating hq2x lookup texture\n");
            if(FAILED(hr=d3ddev->CreateVolumeTexture(256, 16, 256, 1, 0, D3DFMT_A8R8G8B8,
                        D3DPOOL_MANAGED, &lpHq2xLookupTexture, NULL))) {
                pclog("D3D:Failed to create volume texture lpHq2xLookupTexture: %s\n", DXGetErrorString9(hr));

                return E_FAIL;
            }

            // build lookup table
            D3DLOCKED_BOX lockedBox;

            if(FAILED(hr = lpHq2xLookupTexture->LockBox(0, &lockedBox, NULL, 0))) {
                pclog("D3D:Failed to lock box of volume texture lpHq2xLookupTexture: %s\n", DXGetErrorString9(hr));

                return E_FAIL;
            }

            BuildHq2xLookupTexture(dwScaledWidth, dwScaledHeight, dwWidth, dwHeight, (Bit8u *)lockedBox.pBits);

            if(FAILED(hr = lpHq2xLookupTexture->UnlockBox(0))) {
                    pclog("D3D:Failed to unlock box of volume texture lpHq2xLookupTexture: %s\n", DXGetErrorString9(hr));

                    return E_FAIL;
            }
        }

    }
    return S_OK;
}

HRESULT SetVertexData_fs(float sizex, float sizey)
{
    TLVERTEX* vertices;
    HRESULT hr;

    // Lock the vertex buffer
    hr = v_buffer->Lock(0, 0, (void**)&vertices, 0);

    if (hr==D3D_OK)
    {
        //Setup vertices
        vertices[0].Position = D3DXVECTOR3(-0.5f, -0.5f, 0.0f);
        vertices[0].Diffuse  = 0xFFFFFFFF;
        vertices[0].TexCoord = D3DXVECTOR2( 0.0f,  sizey);
        vertices[1].Position = D3DXVECTOR3(-0.5f,  0.5f, 0.0f);
        vertices[1].Diffuse  = 0xFFFFFFFF;
        vertices[1].TexCoord = D3DXVECTOR2( 0.0f,  0.0f);
        vertices[2].Position = D3DXVECTOR3( 0.5f, -0.5f, 0.0f);
        vertices[2].Diffuse  = 0xFFFFFFFF;
        vertices[2].TexCoord = D3DXVECTOR2( sizex, sizey);
        vertices[3].Position = D3DXVECTOR3( 0.5f,  0.5f, 0.0f);
        vertices[3].Diffuse  = 0xFFFFFFFF;
        vertices[3].TexCoord = D3DXVECTOR2( sizex, 0.0f);

        // Additional vertices required for some PS effects
        if(m_hasPreprocess()) {
            vertices[4].Position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f);
            vertices[4].Diffuse  = 0xFFFFFF00;
            vertices[4].TexCoord = D3DXVECTOR2( 0.0f, 1.0f);
            vertices[5].Position = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
            vertices[5].Diffuse  = 0xFFFFFF00;
            vertices[5].TexCoord = D3DXVECTOR2( 0.0f, 0.0f);
            vertices[6].Position = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
            vertices[6].Diffuse  = 0xFFFFFF00;
            vertices[6].TexCoord = D3DXVECTOR2( 1.0f, 1.0f);
            vertices[7].Position = D3DXVECTOR3( 1.0f, 1.0f, 0.0f);
            vertices[7].Diffuse  = 0xFFFFFF00;
            vertices[7].TexCoord = D3DXVECTOR2( 1.0f, 0.0f);
        }

    // Unlock the vertex buffer
        hr = v_buffer->Unlock();
    }

    return hr;
}

void SetupSceneScaled_fs(int w, int h)
{
    double l, t, r, b;

    d3ddev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    d3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    d3ddev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
    d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

     // Turn off culling
    d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    // Turn off D3D lighting
    d3ddev->SetRenderState(D3DRS_LIGHTING, false);
    // Turn off the zbuffer
    d3ddev->SetRenderState(D3DRS_ZENABLE, false);

    //D3DVIEWPORT9 Viewport;
    //d3ddev->GetViewport(&Viewport);

    RECT window_rect;
    GetClientRect(d3d_device_window, &window_rect);
    d3d_fs_size(window_rect, &l, &t, &r, &b, w, h);
    float screen_width = window_rect.right-window_rect.left;
    float screen_height = window_rect.bottom - window_rect.top;

    // Projection is screenspace coords
    D3DXMatrixOrthoOffCenterLH(&m_matProj, 0.0f, (float)screen_width, 0.0f, (float)screen_height, 0.0f, 1.0f);

    // View matrix does offset
    // A -0.5f modifier is applied to vertex coordinates to match texture
    // and screen coords. Some drivers may compensate for this
    // automatically, but on others texture alignment errors are introduced
    // More information on this can be found in the Direct3D 9 documentation
    D3DXMatrixTranslation(&m_matView, (float)screen_width/2-0.5f , (float)screen_height/2+0.5f , 0.0f);

    // World View does scaling
    D3DXMatrixScaling(&m_matWorld, (float)(r-l), (float)(b-t), 1.0f);


    d3ddev->SetTransform(D3DTS_PROJECTION, &m_matProj);
	d3ddev->SetTransform(D3DTS_VIEW, &m_matView);
	d3ddev->SetTransform(D3DTS_WORLD, &m_matWorld);
}

void loadScalingEffect_fs()
{
    //HRESULT hr = ScalingEffectLoadEffect(d3ddev, "CRTD3D.txt");
    if (video_shaders_index==0)
    {
        psActive = false;
        return;
    }
    psActive = true;
    char buf[255];
    strcpy(buf,"shaders\\");
    strcat(buf,video_shaders_name);
    strcat(buf,".fx");

    HRESULT hr = ScalingEffectLoadEffect(d3ddev, buf);
    //HRESULT hr = ScalingEffectLoadEffect(d3ddev, "GS4x.fx");
    //HRESULT hr = ScalingEffectLoadEffect(d3ddev, "SuperEagle.fx");
    //HRESULT hr = ScalingEffectLoadEffect(d3ddev, "hq2x.fx");

    if(FAILED(hr) || FAILED(ScalingEffectValidate()))
    {
        pclog("D3D:Pixel shader error:%s %s\n",DXGetErrorString9(hr), DXGetErrorDescription9(hr));
        pclog("D3D:Pixel shader output disabled\n");
        ScalingEffectKillThis("loadScalingEffect(failed loading or validate)");
        psActive = false;
    }
    else
    {
        pclog("D3D:Pixel shader loaded and validated \n");
        pclog("D3D:Pixel shader m_hasPreprocess=%i\n",m_hasPreprocess());
        if(m_hasPreprocess())
        {
            pclog("D3D:Pixel shader preprocess active\n");
//          vertexbuffersize = sizeof(TLVERTEX) * 8;
        }
    }

}
