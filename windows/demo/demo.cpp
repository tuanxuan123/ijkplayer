// demo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "demo.h"
#include "h5_player.h"
#include <iostream>
#include <d3d9.h>                  
#include <d3dx9.h>


#define MAX_LOADSTRING 100


HINSTANCE hInst;                                
WCHAR szTitle[MAX_LOADSTRING];                 
WCHAR szWindowClass[MAX_LOADSTRING];            

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

static LPDIRECT3DDEVICE9       s_device = NULL;  
static LPDIRECT3DVERTEXBUFFER9 s_vertexBuf = NULL;
static LPDIRECT3DINDEXBUFFER9  s_indexBuf = NULL;
static LPDIRECT3DTEXTURE9      s_videoTexture = NULL;

struct VideoVertex
{
	float m_x, m_y, m_z, m_r, m_u, m_v;

	VideoVertex(float x, float y, float z, float r, float u, float v)
		: m_x(x), m_y(y), m_z(z), m_r(r), m_u(u), m_v(v) {}
};

#define D3DFVF_VideoVertex (D3DFVF_XYZRHW | D3DFVF_TEX1)

void InitDirectX9(HWND hWnd);    
void RenderVideo();
             

void UpdateTextureData(int index, int w, int h, unsigned char *data)
{
	if (!s_videoTexture)
	{
		D3DXCreateTexture(s_device, w, h, 0, 0, D3DFMT_A8B8G8R8, D3DPOOL_MANAGED, &s_videoTexture);
	}

	D3DLOCKED_RECT lockedRect = { 0 };

	s_videoTexture->LockRect(0, &lockedRect, NULL, 0);
	int pitch = lockedRect.Pitch;

	DWORD* pRGBA = (DWORD*)data;
	byte* buffer = (byte*)lockedRect.pBits;
	for (int row = 0; row < h; ++row)
	{
		DWORD* wBuffer = (DWORD*)buffer;
		for (int col = 0; col < w; ++col)
		{
			DWORD pix = *pRGBA;
			wBuffer[col] = pix;
			pRGBA++;
		}
		buffer += lockedRect.Pitch;
	}
	s_videoTexture->UnlockRect(0);

}


void MessageCallback(int index, int event, int arg1, int arg2, const char* msg)
{
	printf("MessageCallback event : %d, message: %s\n", event, msg);
}




static void CALLBACK TimeProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	h5_video_seek(4);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	AllocConsole();
	freopen("conin$", "r+t", stdin);
	freopen("conout$", "w+t", stdout);


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DEMO));

	
	h5_video_init(UpdateTextureData, MessageCallback, FMT_RGBA);
	h5_video_set_cache_path("cache");

	//h5_video_play("https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4", false, 0);
	/*for (int i = 0; i < 300; i++) {
		h5_video_play("F://video/LobbyBg.mp4", true, 0);
		h5_video_stop();
	}*/
	
	h5_video_play("F://video/LobbyBg.mp4", true, 0);
	MSG msg;
	memset(&msg, 0, sizeof(msg));

	// Main message loop:
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			h5_video_update();
			RenderVideo();
		}
	} 
	//add 
	h5_video_stop();
	//h5_video_destory_cache(NULL,0);
	fclose(stdin);
	fclose(stdout);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 1280, 720, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   InitDirectX9(hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



void InitDirectX9(HWND hWnd)
{
	LPDIRECT3D9 pD3D = NULL;
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);


	D3DCAPS9 caps; int vp = 0;
	pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = 1280;
	d3dpp.BackBufferHeight = 720;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		hWnd, vp, &d3dpp, &s_device);
	pD3D->Release();



	s_device->CreateVertexBuffer(4 * sizeof(VideoVertex), 0,
		D3DFVF_VideoVertex, D3DPOOL_DEFAULT, &s_vertexBuf, NULL);


	VideoVertex *pVertices = NULL;
	s_vertexBuf->Lock(0, 0, (void**)&pVertices, 0);

	pVertices[0] = VideoVertex(40.0f, 10.0f, 0.5f, 1.0f, 0.0f, 0.0f);
	pVertices[1] = VideoVertex(1240.0f, 10.0f, 0.5f, 1.0f, 1.0f, 0.0f);
	pVertices[2] = VideoVertex(1240.0f, 710.0f, 0.5f, 1.0f, 1.0f, 1.0f);
	pVertices[3] = VideoVertex(40.0f, 710.0f, 0.5f, 1.0f, 0.0f, 1.0f);


	s_vertexBuf->Unlock();

	s_device->CreateIndexBuffer(6 * sizeof(WORD), 0,
		D3DFMT_INDEX16, D3DPOOL_DEFAULT, &s_indexBuf, NULL);


	WORD *pIndices = NULL;
	s_indexBuf->Lock(0, 0, (void**)&pIndices, 0);
	
	pIndices[0] = 0; pIndices[1] = 1; pIndices[2] = 2;
	pIndices[3] = 0; pIndices[4] = 2; pIndices[5] = 3;


	s_indexBuf->Unlock();

}



void RenderVideo()
{
	s_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	s_device->BeginScene();

												

	s_device->SetStreamSource(0, s_vertexBuf, 0, sizeof(VideoVertex));
	s_device->SetFVF(D3DFVF_VideoVertex);
	s_device->SetIndices(s_indexBuf);
	
	if (s_videoTexture)
	{
		s_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		s_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		s_device->SetTexture(0, s_videoTexture);
	}
		
	
	s_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);

	s_device->EndScene();
	s_device->Present(NULL, NULL, NULL, NULL);
}