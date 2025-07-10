// demo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "demo.h"
#include "h5_player.h"
#include "pandora_player.h"
#include "cJson.h"
#include "player_utils.h"
#include <iostream>
#include <d3d9.h>                  
#include <d3dx9.h>
#include <stdlib.h>


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
static LPDIRECT3DVERTEXBUFFER9 s_vertexBuf[2] = { NULL };
static LPDIRECT3DINDEXBUFFER9  s_indexBuf = NULL ;
static LPDIRECT3DTEXTURE9      s_videoTexture[2] = { NULL };
static Pixel_Format			   s_videoDataFormat = FMT_RGBA;

struct VideoVertex
{
	float m_x, m_y, m_z, m_r, m_u, m_v;

	VideoVertex(float x, float y, float z, float r, float u, float v)
		: m_x(x), m_y(y), m_z(z), m_r(r), m_u(u), m_v(v) {}
};

#define D3DFVF_VideoVertex (D3DFVF_XYZRHW | D3DFVF_TEX1)

void InitDirectX9(HWND hWnd);    
void RenderVideo();

static void* H5FileOpen(const char *path, int mode)
{
	return (void *) fopen(path, "rb");
}

static int H5FileRead(void *file_handle, unsigned char *buf, int size)
{
	return fread(buf, sizeof(char), size, (FILE *) file_handle);
}


static int64_t H5FileSeek(void *file_handle, int64_t offset, int whence)
{
	FILE *file = (FILE *) file_handle;
	if (whence == 65536)
	{
		int64_t cur_pos = ftell(file);
		fseek(file, 0, SEEK_END);
		int64_t file_len = ftell(file);

		fseek(file, cur_pos, SEEK_SET);
		cur_pos = ftell(file);
		return file_len;
	}

	return fseek(file, offset, whence);
}


static int H5FileClose(void *file_handle)
{
	return fclose((FILE*)file_handle);
}

static void PandoraUpdateTextureData(uint8_t* data[], int w, int h, int format, int tag)
{
	printf("hPandoraUpdateTextureData tag: %d\n", tag);
}

static void UpdateTextureData(int w, int h, unsigned char *data, int tag)
{
	if (tag >= 2)
		return;

	if (!s_videoTexture[tag])
	{
		D3DXCreateTexture(s_device, w, h, 0, 0, (s_videoDataFormat == FMT_RGB ? D3DFMT_R8G8B8 : D3DFMT_A8R8G8B8), D3DPOOL_MANAGED, &s_videoTexture[tag]);
	}

	D3DLOCKED_RECT lockedRect = { 0 };

	s_videoTexture[tag]->LockRect(0, &lockedRect, NULL, 0);


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

		buffer += pitch;
	}


	s_videoTexture[tag]->UnlockRect(0);

}


void MessageCallback(int event, int arg1, int arg2, const char* msg, int tag)
{
	if (event == 200)
	{
		cJSON* root = cJSON_Parse(msg);

		cJSON* duration = cJSON_GetObjectItem(root, "duration/ms");
		cJSON* width = cJSON_GetObjectItem(root, "width/p");
		cJSON* bit_rate = cJSON_GetObjectItem(root, "bit_rate/bps");
		cJSON* codec_id = cJSON_GetObjectItem(root, "codec_id");
		cJSON* cformat = cJSON_GetObjectItem(root, "cformat");
		cJSON* hardware_decode = cJSON_GetObjectItem(root, "hardware_decode");

		printf("MessageCallback json_message: %s\n", msg);
		printf("MessageCallback json_item duration:%d, width:%d, bit_rate:%d, code_id:%s, nformat:%s, hardware_decode:%d\n", 
			duration->valueint, width->valueint, bit_rate->valueint, codec_id->valuestring, cformat->valuestring, hardware_decode->valueint);
	}
}


void AudioDataCallback(const unsigned char* data, unsigned int size, int samples, int channels, int sample_rate, double clock, int tag)
{
	printf("AudioDataCallback samples: %d, channels: %d, sample_rate: %d, clock: %f, size : %d\n", samples, channels, sample_rate, clock, size);
}


void NetworkDataCallback(const unsigned char *data, unsigned int size, int tag)
{
	printf("NetworkDataCallback size: %d\n",  size);
}
static int tag1 = 0;
static int tag2 = 0;
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

	h5_video_set_log_level(LOG_INFO);
	h5_video_init(UpdateTextureData, MessageCallback, s_videoDataFormat);
	//h5_video_set_audio_data_callback(AudioDataCallback, 1);
	//h5_video_set_network_data_callback(NetworkDataCallback);

	//h5_video_set_file_delegate(H5FileOpen, H5FileRead, H5FileSeek, H5FileClose);

	//h5_video_set_cache_path("C:/Users/xanderdeng/Desktop/PVideoPlayerCache");
	//h5_video_set_cache_path("C:/Users/kiddpeng/Desktop/PVideoPlayerCache");

	//int tag1 = h5_video_play("C:/Users/xanderdeng/Desktop/videosrc/too_short.mp4", 0, 0);
	//int tag1 = h5_video_play("ijkio:cache:ffio:https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4", 1, 0);
	tag1 = h5_video_play("https://1318168317.vod-qcloud.com/b16262f1vodtranscq1318168317/dfe79e571397757886876166210/v.f100040.mp4", 1, 0);
	tag2 = pandora_video_play("https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4", 1);
	//int tag1 = h5_video_play("ijkio:cache:ffio:E:/1/warn.ogg", 0, 0);

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
			h5_video_update(tag1);
			pandora_video_update(tag2);
			RenderVideo();

			Sleep(5);
		}
	} 



  
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
	static int volume = 100;
	static int seek_time = 0;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
				//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				h5_video_stop(0);
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
	case WM_KEYDOWN:
		
		switch (wParam) {
		case VK_SPACE:
			seek_time = rand() % 120;
			h5_video_seek(seek_time, 0);//or resume
			break;
		case VK_LEFT: {
			volume -= 10;
			volume = volume < 0 ? 0 : volume;
			h5_video_set_volume(volume, 0);
			break;
		}
		case VK_RIGHT:
			volume += 10;
			volume = volume > 100 ? 100 : volume;
			h5_video_set_volume(volume, 0);
			break;
		case VK_UP:
			h5_video_pause(0);
			break;
		case VK_DOWN:
			h5_video_resume(0);
			break;
		case VK_ADD:
			
			break;
		case VK_SUBTRACT:
			
			break;
		}
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


	for(int i = 0; i < 2; ++i)
		s_device->CreateVertexBuffer(4 * sizeof(VideoVertex), 0,
			D3DFVF_VideoVertex, D3DPOOL_DEFAULT, &s_vertexBuf[i], NULL);


	VideoVertex *pVertices = NULL;
	s_vertexBuf[0]->Lock(0, 0, (void**)&pVertices, 0);
	pVertices[0] = VideoVertex(40.0f, 10.0f, 0.5f, 1.0f, 0.0f, 0.0f);
	pVertices[1] = VideoVertex(640.0f, 10.0f, 0.5f, 1.0f, 1.0f, 0.0f);
	pVertices[2] = VideoVertex(640.0f, 410.0f, 0.5f, 1.0f, 1.0f, 1.0f);
	pVertices[3] = VideoVertex(40.0f, 410.0f, 0.5f, 1.0f, 0.0f, 1.0f);
	s_vertexBuf[0]->Unlock();

	s_vertexBuf[1]->Lock(0, 0, (void**)&pVertices, 0);
	pVertices[0] = VideoVertex(650.0f, 10.0f, 0.5f, 1.0f, 0.0f, 0.0f);
	pVertices[1] = VideoVertex(1250.0f, 10.0f, 0.5f, 1.0f, 1.0f, 0.0f);
	pVertices[2] = VideoVertex(1250.0f, 410.0f, 0.5f, 1.0f, 1.0f, 1.0f);
	pVertices[3] = VideoVertex(650.0f, 410.0f, 0.5f, 1.0f, 0.0f, 1.0f);
	s_vertexBuf[1]->Unlock();



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
		D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
	s_device->BeginScene();

												
	for (int i = 0; i < 2; ++i)
	{
		s_device->SetStreamSource(0, s_vertexBuf[i], 0, sizeof(VideoVertex));
		s_device->SetFVF(D3DFVF_VideoVertex);
		s_device->SetIndices(s_indexBuf);

		if (s_videoTexture)
		{
			s_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			s_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			s_device->SetTexture(0, s_videoTexture[i]);
		}


		s_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	}

	s_device->EndScene();
	s_device->Present(NULL, NULL, NULL, NULL);
}