/*
���Ӽ���
enermy = �÷��̾� �����, �浹�� ���ӿ���, �ӵ� �� ����
�� ������ = ���ǵ��
�� ������ = �ٸԾ�� �������� Ŭ����
���� �� ������ -> stage2�� �Ѿ�� ���� �����, �� ���� �����ؾ� Ŭ����

stage1 = cup 6��

*/
#include "stdafx.h"
#include "alexvg_default.h"

#include "LoadImage_vcl.h"
#include <egl/egl.h>
#include <vg/openvg.h>
#include <vg/vgu.h>

#include <time.h>
#include <fstream>

// ���� setup include
#include "DXUTsound.h" // ���� �ؾߵǼ� ����

// ���� ���̺귯�� �߰�
#pragma comment (lib, "winmm")
#pragma comment (lib, "dsound")
#pragma comment (lib, "dxguid")

#define M_PI       3.14159265358979323846

#define MAX_LOADSTRING 100

#define MY_WIDTH 19 * 34       // window �ʱⰡ��ũ��
#define MY_HEIGHT 14 * 34       // window �ʱ⼼��ũ��

#define READY_THROWING 0
#define CHARGE_POWER 1
#define THROW_BOMB 2
#define THROW_ANGLE 45
#define MAX_NUM_BOMBS 10
/*
#define MAP_W  23 782
#define MAP_H  16 544
#define TILE_SIZE 34
*/
EGLDisplay display;
EGLSurface surface;
EGLContext context;
		 int e = 0;

// window size�� ���õ� ����
int cw, ch;
int cx, cy;

float ws = 1.0f;
float hs = 1.0f;

// �̹��� ���� ����
VGImage img[2];
VGfloat x, y;

// Update ���� �Լ�
void timeProc();
void draw();
void move_tank();

// ���� �ʱ�ȭ
CSoundManager* sm = NULL; 
CSound *sound_bgm = NULL; // It��s a just example
CSound *sound_ing_bgm = NULL;
CSound *sound_item = NULL;
CSound *sound_shot = NULL;
CSound *sound_move = NULL;
CSound *sound_broken = NULL;
CSound *sound_hit = NULL;

// ������� Ÿ�̸�
int bgm = 0;

// ready, cup, over�̹���
VGImage ready;
VGImage over;
VGImage cup;
VGImage stage1;
VGImage win_image;

// ���ӿ���
int gameover = 0;

// ����
int god = 0;

// �¸�����
int cup_count = 0;
int win = 0;

//��
int bomb_counter = 0;
int crush = 0;
int crush_timer = 0;

void ReadMap(const char* filename, char* map, int W, int H)
{
	char temp[100];
	// Open the file
	std::ifstream ifs(filename);
	// For each line
	for(int i = 0; i < H; i++)
	{
		// Read a line
		ifs.read(&(map[(H-i-1)*W]), W);
		// Skip to end of line
		ifs.getline(temp, 100);
	}
	ifs.close();
}


// �� ����
#define MAP_W		23 // 782
#define MAP_H		16 // 544
#define TILE_SIZE	34

// �� �迭
char map[MAP_W*MAP_H];

// �� �̹���
VGImage block;
VGImage item;

// ������ ���
#define LEFT  0
#define RIGHT 1
#define UP    2
#define DOWN  3

// �÷��̾� ��ũ ����ü
struct Tank {
	int x; // VGfloat
	int y;
	unsigned isActive; // 010101
	int activeTime; // ������ �Ծ�����
	unsigned speed;
	unsigned direction;
	unsigned frame;
	VGImage up[2], down[2], left[2], right[2], effect[2];
} player;

// �� ��ũ ����ü
struct Tank2 {
	int x; // VGfloat
	int y;
	unsigned isActive; // 010101
	int activeTime; // ������ �Ծ�����
	unsigned speed;
	unsigned direction;
	unsigned frame;
	VGImage up[2], down[2], left[2], right[2], effect[2];
} enemy;

// ��ź ����ü
struct Bomb
{
int vx, vy, vz, x, y, z;
int state;
static VGImage img;
static int phase;
} bombs[MAX_NUM_BOMBS];
int Bomb::phase = READY_THROWING;
VGImage bomb;
//VGImage Bomb::img = NULL;

//�߻� ����ü
struct FIRE
{
int x, y;
VGImage img[2];
int frame;
int counter;
} fire;

// Asynchronous keyboard input setup
#define KEY_DOWN(code) ((GetAsyncKeyState(code)&0x8000)? 1 : 0)

/*
�Ѿ�(Bullet) ����ü {
	������ ��ǥ;
	������ �߻翩��;
	������ ���ǵ�; // ��� �Ѿ��� ���� ���� �ʿ�� ������ ��ũ ����ü�� �������� �ؼ� ����.
	������ ����;
} �Ѿ˵�[�Ѿ� �ִ밹����ŭ]
�Ѿ� �̹��� ��, �Ʒ�, ��, �� // �� ���⿡ ���ؼ� �̹��� ����
*/
/////////////////////////////////////////////////////////////////////////////////////
struct Bull{
	VGfloat x, y;
	unsigned shot;
	unsigned direction;
	int speed;
} bullet [10];
VGImage bull_left, bull_right, bull_up, bull_down;



HINSTANCE hInst;					// ���� �ν��Ͻ��Դϴ�.
TCHAR szTitle[MAX_LOADSTRING];				// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR szWindowClass[MAX_LOADSTRING];			// �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ���⿡ �ڵ带 �Է��մϴ�.
	MSG msg;
	HACCEL hAccelTable;

	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ALEXVG_DEFAULT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	
	// ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ALEXVG_DEFAULT));

	DWORD lastTime = GetTickCount();
	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, 1)) {
			if (!TranslateAccelerator(msg.hwnd, NULL, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (msg.message == WM_QUIT) break;
		}
		
		if (GetTickCount() - lastTime > 32) { // 30������
			lastTime += 33;
			timeProc();
		}
		

	}

	// �⺻ �޽��� �����Դϴ�.
	/*while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}*/

	return (int) msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
//  ����:
//
//    Windows 95���� �߰��� 'RegisterClassEx' �Լ����� ����
//    �ش� �ڵ尡 Win32 �ý��۰� ȣȯ�ǵ���
//    �Ϸ��� ��쿡�� �� �Լ��� ����մϴ�. �� �Լ��� ȣ���ؾ�
//    �ش� ���� ���α׷��� �����
//    '�ùٸ� ������' ���� �������� ������ �� �ֽ��ϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ALEXVG_DEFAULT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ALEXVG_DEFAULT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	  CW_USEDEFAULT, 0, MY_WIDTH, MY_HEIGHT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
	  return FALSE;
   }
   
   // ���� �ʱ�ȭ & ���
   sm = new CSoundManager();
   sm->Initialize( hWnd, DSSCL_PRIORITY );
   sm->SetPrimaryBufferFormat(2,22050,16); // It��s a just example. sm->Create(&sound_bgm,TEXT("bgm.wav"),0,GUID_NULL);
   sm->Create(&sound_bgm,TEXT("sounds/start.wav"),0,GUID_NULL);
   sm->Create(&sound_ing_bgm,TEXT("sounds/STARSNG.wav"),0,GUID_NULL);
   sm->Create(&sound_shot,TEXT("sounds/dot.wav"),0,GUID_NULL);
   sm->Create(&sound_item,TEXT("sounds/bite.wav"),0,GUID_NULL);
   sm->Create(&sound_broken,TEXT("sounds/broken.wav"),0,GUID_NULL);
   sm->Create(&sound_hit,TEXT("sounds/hit.wav"),0,GUID_NULL);

   sound_bgm->Play(0);
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   
   return TRUE;
}

//
//   �Լ�: AppInit( NativeWindowType native_window, NativeDisplayType native_display, int cw, int ch )
//				   
//   ����: EGL surface�� �ʱ�ȭ �Ѵ�. �̹����� �ε��Ѵ�.
//
//   ����: �� �Լ��� ���� �׸��� ���� EGL surface�� �ʱ�ȭ �մϴ�.

int AppInit( NativeWindowType native_window, NativeDisplayType native_display, 
			 int cw, int ch )
{
	EGLint config_list[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	EGLint attrib_list[] = {
		EGL_WIDTH, cw,
		EGL_HEIGHT, ch,
		EGL_NONE
	};

	EGLConfig config[10];
	EGLint num_config;

	display = eglGetDisplay( native_display );

	if (display == EGL_NO_DISPLAY)
		return 0;

	if (eglInitialize(display, NULL, NULL) == EGL_FALSE || eglGetError() != EGL_SUCCESS)
		return 0;

	eglBindAPI(EGL_OPENVG_API);
	eglChooseConfig(display, config_list, config, 10, &num_config);
	surface = eglCreateWindowSurface( display, config[0], native_window, attrib_list );

	if ( surface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS )
		return 0;

	context = eglCreateContext( display, 0, NULL, NULL );
	if ( context == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS )
		return 0;

	if ( eglMakeCurrent( display, surface, surface, context ) == EGL_FALSE || eglGetError() != EGL_SUCCESS )
		return 0;

	

	// �̹��� �ε�

	if ((img[0] = vgextLoadImage(TEXT("images/effect0.png"))) == NULL) {
		VGEXTPRINTERROR();
	}

	if ((img[1] = vgextLoadImage(TEXT("images/effect1.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((block = vgextLoadImage(TEXT("images/block.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((item = vgextLoadImage(TEXT("images/item.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	// Tank image load...
	if ((player.down[0] = vgextLoadImage(TEXT("images/down0.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((player.down[1] = vgextLoadImage(TEXT("images/down1.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((player.up[0] = vgextLoadImage(TEXT("images/up0.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((player.up[1] = vgextLoadImage(TEXT("images/up1.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((player.left[0] = vgextLoadImage(TEXT("images/left0.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((player.left[1] = vgextLoadImage(TEXT("images/left1.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((player.right[0] = vgextLoadImage(TEXT("images/right0.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((player.right[1] = vgextLoadImage(TEXT("images/right1.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	// enemy 
	if ((enemy.down[0] = vgextLoadImage(TEXT("images/tank_down0_r.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((enemy.down[1] = vgextLoadImage(TEXT("images/tank_down1_r.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((enemy.up[0] = vgextLoadImage(TEXT("images/tank_up0_r.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((enemy.up[1] = vgextLoadImage(TEXT("images/tank_up1_r.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((enemy.left[0] = vgextLoadImage(TEXT("images/tank_left0_r.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((enemy.left[1] = vgextLoadImage(TEXT("images/tank_left1_r.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((enemy.right[0] = vgextLoadImage(TEXT("images/tank_right0_r.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((enemy.right[1] = vgextLoadImage(TEXT("images/tank_right1_r.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	// ready,cup,over �̹���
	if ((ready = vgextLoadImage(TEXT("images/ready.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((over = vgextLoadImage(TEXT("images/gameover.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((cup = vgextLoadImage(TEXT("images/cup.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((stage1 = vgextLoadImage(TEXT("images/exit.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((win_image = vgextLoadImage(TEXT("images/win.jpg"))) == NULL) {
		VGEXTPRINTERROR();
	}
	// ��ź �̹���
	if ((fire.img[0] = vgextLoadImage(TEXT("images/tankBomb0.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((fire.img[1] = vgextLoadImage(TEXT("images/tankBomb1.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((bomb = vgextLoadImage(TEXT("images/bomb.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	// Map load...
	ReadMap("map/map_w23h16.txt", map, MAP_W, MAP_H);

	// Initialize tank properties ����� ��ũ �ʱ�ȭ
	for (int i=0; i<MAP_H; i++) {
		for (int j=0; j<MAP_W; j++) {
			if (map[j + i*MAP_W] == 'p') {
				player.x = (VGfloat)j*TILE_SIZE;
				player.y = (VGfloat)i*TILE_SIZE;
				break;
			}
		}
	}
	player.direction = UP;
	player.speed = 3;
	player.isActive = 0;

	// Initialize enemy properties �� ��ũ �ʱ�ȭ
	for (int i=0; i<MAP_H; i++) {
		for (int j=0; j<MAP_W; j++) {
			if (map[j + i*MAP_W] == 'e') {
				enemy.x = (VGfloat)j*TILE_SIZE;
				enemy.y = (VGfloat)i*TILE_SIZE;
				break;
			}
		}
	}
	enemy.direction = UP;
	enemy.speed = 3;
	enemy.isActive = 0;

	
	//�� ���⿡ ���� �Ѿ� �̹��� �ε�
	for( int i=0; i<10; i++ ){
	bullet[i].shot = 0;
	bullet[i].x = 0;
	bullet[i].y = 0;
	}

	//��ź �ʱ�ȭ
	for( int i=0; i<MAX_NUM_BOMBS; i++ ){
		bombs[i].state = FALSE;
	}
	

	if ((bull_left = vgextLoadImage(TEXT("images/bullet_left.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((bull_right = vgextLoadImage(TEXT("images/bullet_right.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((bull_down = vgextLoadImage(TEXT("images/bullet_down.png"))) == NULL) {
		VGEXTPRINTERROR();
	}
	if ((bull_up = vgextLoadImage(TEXT("images/bullet_up.png"))) == NULL) {
		VGEXTPRINTERROR();
	}

	// Initialize bullet properties
	/*
	�ݺ��� �Ѿ� ������ŭ {
	   �� �Ѿ� ���ǵ�� 4�� �Ѵ�.
	}
	*/

	for (int i=0; i<10; i++)
	{
		bullet[i].speed = 4;
	}
	
	return 1;
}

//
//   �Լ�: AppRelease( void )
//
//   ����: EGL surface�� release�Ѵ�. �̹����� ���� �Ѵ�.
//
//   ����:  �� �Լ��� ���� EGL surface�� release�մϴ�.   
//
int AppRelease( void )
{
	/////////////////////////////////////////////////////////////////////////////////
	// ������
	/////////////////////////////////////////////////////////////////////////////////
	// Destroy map image
	vgDestroyImage( block );
	vgDestroyImage( item );
	// Destroy character images
	for (int i=0; i<2; i++) {
		vgDestroyImage( player.down[i] );
		vgDestroyImage( player.up[i] );
		vgDestroyImage( player.left[i] );
		vgDestroyImage( player.right[i] );
		vgDestroyImage( player.effect[i] );
		vgDestroyImage( img[i]);
		vgDestroyImage( enemy.down[i] );
		vgDestroyImage( enemy.up[i] );
		vgDestroyImage( enemy.left[i] );
		vgDestroyImage( enemy.right[i] );
	}
	vgDestroyImage(ready);
	vgDestroyImage(bull_left);
	vgDestroyImage(bull_right);
	vgDestroyImage(bull_up);
	vgDestroyImage(bull_down);
	vgDestroyImage(over);
	vgDestroyImage(cup);
	vgDestroyImage(stage1);
	vgDestroyImage(win_image);
	/////////////////////////////////////////////////////////////////////////////////

	eglMakeCurrent(display, EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT );

	eglDestroyContext( display, context );
	context = NULL;

	eglDestroySurface( display, surface );
	surface = NULL;

	eglTerminate( display );
	display = NULL;

	return 1;
}

//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����: �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND	- ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT	- �� â�� �׸��ϴ�.
//  WM_DESTROY	- ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	RECT rect;
	int ww;
	int hh;


	switch (message)
	{
	case WM_CREATE:
		{
			hdc = GetDC( hWnd );
			GetClientRect(hWnd, &rect);
			ww = rect.right - rect.left;
			hh = rect.bottom - rect.top;

			cw = ww;
			ch = hh;
			if(cw%4)
				cw = cw - (cw%4) + 4;
			if ( AppInit( hWnd, hdc ,cw,ch) == 0 )
				return 0;
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �޴��� ���� ������ ���� �м��մϴ�.
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
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:				//�������� ũ�Ⱑ ���Ͽ��� ���
		{
			GetClientRect(hWnd, &rect);
			ww = rect.right - rect.left;	// Ŭ���̾�Ʈ �������� Ʋ�� ������ ���� ����
			hh = rect.bottom - rect.top;	// Ŭ���̾�Ʈ �������� Ʋ�� ������ ���� ����

			// ���� ũ�Ⱑ 800���� Ŭ��� 800���� �����մϴ�(alexVG�� imageŬ������ 
			// ���׶����� fix��Ŵ)
			if(ww > 800)	
			{				
				POINT curpos;
				GetWindowRect(hWnd,&rect);
				curpos.x = rect.left;
				curpos.y = rect.top;
				int curheight = rect.bottom-rect.top;
				int curwidth = rect.right-rect.left;
				MoveWindow(hWnd,curpos.x,curpos.y,curwidth-ww+800,curheight,TRUE);
				ww = 800;
			}

			int w = ww;
			int h = hh;
			if(w%4)
				w = w - (w%4) + 4;

			if( w < 4)
				w = 4;
			if( h < 4)
				h = 4;

			// ����� ������ ��� EGL surface�� release �ϰ� �ٽ� ���� ũ��� �����
			if((w != cw) || (h != ch))	
			{
				AppRelease();		
				hdc = GetDC( hWnd );

				if ( AppInit( hWnd, hdc, w,h ) == 0 )
					return 0;

				cw = w;
				ch = h;

				ws = (float)cw /(float)MY_WIDTH;
				hs = (float)ch /(float)MY_HEIGHT;
			}
			eglSwapBuffers(display, surface);
		}
		break;
	/////////////////////////////////////////////////////////////////////////////////
	// Ű���� �κ� �ڵ� ����
	/////////////////////////////////////////////////////////////////////////////////
	/*
	Ű���带 ������ ���� ���� �̺�Ʈ ó��:
	{
		���� �����̽��ٸ� ������ ��� {
			�ݺ��� ��� �Ѿ˿� ���ؼ� �˻� {
				���� �Ѿ��� �߻���� ���� ��� {
					�Ѿ��� �߻�ƴٰ� ����;
					�Ѿ� �ʱ���ġ�� ��ũ�� ������� ����;
					�Ѿ��� ������ ��ũ�� �������� ����;
					�ݺ��� ����;
				}
			}
		}
	}
	*/
		/////////////////////////////////////////////////////////////////////////////
	case WM_KEYUP:
		{
			if ( bgm > 125 && gameover == 0) {
			if (wParam == VK_SPACE)
			{
				for(int i = 0; i<10; i++)
				{
					if(! bullet[i].shot)
					{
						bullet[i].shot = TRUE;
						bullet[i].x = player.x+7;
						bullet[i].y = player.y+7; // State Definition int fighterY, missle[i].y = fighterY+20;
						bullet[i].direction = player.direction;
						sound_shot->Play(0);
						break;
					}
				}
			}
			}

		if ( wParam == VK_RETURN){
		for( int i=0; i<MAX_NUM_BOMBS; i++ ) 
			{
				if(!bombs[i].state){
				bombs[i].state=1; bombs[i].x= player.x; bombs[i].y=player.y;

						switch(player.direction)
							{

							case UP: // y�� ����
							bombs[i].vx = 0;
							bombs[i].vy = bomb_counter*cos(THROW_ANGLE*3.141592/180);
							bombs[i].vz = bomb_counter*sin(THROW_ANGLE*3.141592/180);
							break;

							case LEFT:
							bombs[i].vy = 0;
							bombs[i].vx = -bomb_counter*cos(THROW_ANGLE*3.141592/180);
							bombs[i].vz = bomb_counter*sin(THROW_ANGLE*3.141592/180);
							break;

							case DOWN:
							bombs[i].vy = 0;
							bombs[i].vx = -bomb_counter*cos(THROW_ANGLE*3.141592/180);
							bombs[i].vz = bomb_counter*sin(THROW_ANGLE*3.141592/180);
							break;

							case RIGHT:
							bombs[i].vy = 0;
							bombs[i].vx = bomb_counter*cos(THROW_ANGLE*3.141592/180);
							bombs[i].vz = bomb_counter*sin(THROW_ANGLE*3.141592/180);
							break;
		
							}
					}
				}
			}
		}
		break;

	case WM_DESTROY:
		{
		PostQuitMessage(0);		
		AppRelease();
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
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

void timeProc() 
{
	/////////////////////////////////////////////////////////////////////////////////
	// ��ũ ���� ������ �ڵ� ����
	/////////////////////////////////////////////////////////////////////////////////
	// Tank Movement
	if (player.isActive)
	{
		player.speed = 5;
	}

	/// ������� + �������̰� ��
	bgm ++;

	// ��ũ�浹 
	if(!crush){
		if ( player.x < (enemy.x+30) && (player.x+30) > enemy.x && player.y < (enemy.
		y+30) && (player.y+30) > enemy.y)
		{
			gameover = 1;
			sound_broken->Play(0);
		}
	}

	// �Ѿ��浹
	if(!crush){
		for (int i=0; i<10; i++)
		if(bullet[i].x < (enemy.x+30) && ((bullet[i].x+30) > enemy.x && bullet[i].y < (enemy.y+30) && (bullet[i].y+30) > enemy.y))
		bullet[i].shot = FALSE;
	}
		if (win==1)
		{
			gameover = 1;
		}
	
	// ��ź�浹
		if(!crush){
		for (int i=0; i<10; i++)
		if(bombs[i].x < (enemy.x+30) && ((bombs[i].x+30) > enemy.x && bombs[i].y < (enemy.y+30) && (bombs[i].y+30) > enemy.y)){
		crush = TRUE;
		bombs[i].state = FALSE;
		
		}
		}

		
	// �÷��̾� ������
	if ( bgm > 125 && gameover == 0) 
	{
		
	if (KEY_DOWN(VK_UP)) {
	player.y += player.speed;
	player.direction = UP;
	player.frame++;
	sound_shot->Play(0);
	if ((map[(player.x+30)/TILE_SIZE + ((player.y+30)/TILE_SIZE)*MAP_W] == 'b') || 
	(map[(player.x)/TILE_SIZE + ((player.y+30)/TILE_SIZE)*MAP_W] == 'b') ) 
	{
	player.y -= player.speed;
	}	
	} 
	
	else if (KEY_DOWN(VK_DOWN)) {
	player.y -= player.speed;
	player.direction = DOWN;
	player.frame++;
	sound_shot->Play(0);
	if ((map[(player.x+30)/TILE_SIZE + ((player.y)/TILE_SIZE)*MAP_W] == 'b') || 
	(map[(player.x)/TILE_SIZE + ((player.y)/TILE_SIZE)*MAP_W] == 'b') ) 
	{
	player.y += player.speed;
	}
	} 
	
	else if (KEY_DOWN(VK_LEFT)) {
	player.x -= player.speed;
	player.direction = LEFT;
	player.frame++;
	sound_shot->Play(0);
	if ((map[(player.x)/TILE_SIZE + ((player.y+30)/TILE_SIZE)*MAP_W] == 'b') || 
	(map[(player.x)/TILE_SIZE + ((player.y)/TILE_SIZE)*MAP_W] == 'b') ) 
	{
	player.x += player.speed;
	}
	} 
	
	else if (KEY_DOWN(VK_RIGHT)) {
	player.x += player.speed;
	player.direction = RIGHT;
	player.frame++;
	sound_shot->Play(0);
	if ((map[(player.x+30)/TILE_SIZE + ((player.y+30)/TILE_SIZE)*MAP_W] == 'b') || 
	(map[(player.x+30)/TILE_SIZE + ((player.y)/TILE_SIZE)*MAP_W] == 'b') ) 
	{
	player.x -= player.speed;
	}
	}
	

	}
	/*
	// ������ �ڵ� ����
	���� �ʻ󿡼� ��ũ�� ���������� �ִٸ� {
		��ũ�� Ư������ = 1;	// �ʱ�ȭ�� 0���� �Ǿ������� 1�� ������ ����
		������[��ũ�� �ʻ� ���� ��ġ] = ��ĭ���� ����;
	}
	���� ��ũ�� Ư�������̸� {
		��ũ�� Ư������Ÿ�̸� ++;
	}
	���� ��ũ�� Ư�����·� ������ 10�ʰ� �����ٸ� {
		��ũ�� Ư������ = 0;
		��ũ�� Ư������Ÿ�̸� = 0;
	}
	*/
	// ���ǵ� ������
	if( map[((int)player.y/34) * MAP_W + ((int)player.x/32) ] == '.')
	{
		 player.isActive = TRUE;
		 map[((int)player.y/34) * MAP_W + ((int)player.x/32)] = ' ';
		 sound_item->Play(0);
		
	}
	// �� ������
	if( map[((int)player.y/34) * MAP_W + ((int)player.x/32) ] == 'c')
	{
		 cup_count++; // �� ���� ����
		 map[((int)player.y/34) * MAP_W + ((int)player.x/32)] = ' ';
		 sound_item->Play(0);
		
	}
	// �¸�����
	if(cup_count == 6){
	if( map[((int)player.y/34) * MAP_W + ((int)player.x/32) ] == 'x')
	{
		 win = 1; // �� 6�� ������ �¸����� ����
		 map[((int)player.y/34) * MAP_W + ((int)player.x/32)] = ' ';
		 sound_item->Play(0);
		
	}
	}

	// Ư�� ������ �ð�
	if (player.isActive)
	{
		player.activeTime ++;
		
	}

	// 300������ = 10��
	if ( player.activeTime > 300)
	{
		player.isActive = FALSE;
		player.speed = 3;
		player.activeTime = 0;
	
	}

	/*
	�ݺ��� ��� �Ѿ˿� ���ؼ� �˻� {
		���� �Ѿ��� ��ġ�� Ŭ���̾�Ʈȭ��(������â)�� ��� ��� {
			// ���������� ���������� �ʱ�ȭ�� 0���� �������̴�, 0�� �߻�����
			// ���� ���·� �ϰ� 1�� �߻��� ���·� �����Ѵ�.
			�Ѿ� �߻翩�� = 0;
		}
		// �浹 �˻� �ڵ�
		if (map[bullet[i].x/TILE_SIZE + (bullet[i].y/TILE_SIZE)*MAP_W] == 'b') {
			bullet[i].isShoot = 0;
		}
		���� �Ѿ��� �߻����� ��� {
			����ġ �Ѿ��� ���� {
				�� ���⿡ �°� �Ѿ��� ��ġ�� �Ѿ��� ���ǵ常ŭ ������Ű�ų� ���ҽ�Ų��;
			}
		}
	}
	*/

		 for (int i=0; i<10; i++) {
		  // ��ũ��
		  if ((bullet[i].x < 0 || bullet[i].x > MY_WIDTH) || (bullet[i].y < 0 || bullet[i].y > MY_HEIGHT))
		  {
			  bullet[i].shot = FALSE;
		  }
		  // Ÿ��
		  if (map[((int)bullet[i].x)/TILE_SIZE + ((int)bullet[i].y/TILE_SIZE)*MAP_W] == 'b') {
		  bullet[i].shot = FALSE;
		  }

		    // ��ũ��
		  if ((bombs[i].x < 0 || bombs[i].x > MY_WIDTH) || (bombs[i].y < 0 || bombs[i].y > MY_HEIGHT))
		  {
			  bombs[i].state = FALSE;
		  }

		  // �Ѿ� ���ǵ� ���� �Ӽ�
		  if( bullet[i].shot)
				{
					switch(bullet[i].direction){
								case UP : bullet[i].y += 4; break;
								case DOWN : bullet[i].y -= 4; break;
								case LEFT :bullet[i].x -= 4; break;
								case RIGHT : bullet[i].x += 4; break;
								break;
								}
			
				}
	}
	/////////////////////////////////////////////////////////////////////////////////
	/*
	#define MAP_W		23 // 782
	#define MAP_H		16 // 544
	#define TILE_SIZE	34
	*/
		// Ư������Ʈ �ִϸ��̼�
		if (e == 0) e = e + 1; 
		else e = 0;

		// Ⱦ ��ũ�Ѹ�
		// ī�޶�(cx) �� ��ġ�� ȭ�� �߾����� ����
		cx = player.x - MY_WIDTH/2;
		//ī�޶� �̵� ���� ����
		if (cx < 0) cx = 0;
		if (cx > (MAP_W)*TILE_SIZE - MY_WIDTH+15)
		cx = (MAP_W)*TILE_SIZE - MY_WIDTH+15;
		// �� ��ũ�Ѹ�
		// ī�޶�(cy)�� ��ġ ����
		// ī�޶� �̵� ���� ����

		cy = player.y - MY_HEIGHT/2;
		//ī�޶� �̵� ���� ����
		if (cy < 0) cy = 0;
		if (cy > (MAP_H)*TILE_SIZE - MY_HEIGHT+TILE_SIZE+13)
		cy = (MAP_H)*TILE_SIZE - MY_HEIGHT+TILE_SIZE+13;

	if(!crush){
	//�� �ΰ����� �ڵ�
	if ( bgm > 125 && gameover ==0) 
	{
		int distance_x = player.x - enemy.x;
		int distance_y = player.y - enemy.y;
		
		if( abs(distance_x)-abs(distance_y) >= 0)
		{

			if(distance_x >=0)
			{
				//move_tank(RIGHT, enemy);
				enemy.x += enemy.speed;
				enemy.direction = RIGHT;
				enemy.frame++;
			
				if ((map[(enemy.x+30)/TILE_SIZE + ((enemy.y+30)/TILE_SIZE)*MAP_W] == 'b') || 
				(map[(enemy.x+30)/TILE_SIZE + ((enemy.y)/TILE_SIZE)*MAP_W] == 'b') ) 
				{
				enemy.x -= enemy.speed;
				}
			}

			else
				{
					enemy.x -= enemy.speed;
					enemy.direction = LEFT;
					enemy.frame++;
					
					if ((map[(enemy.x)/TILE_SIZE + ((enemy.y+30)/TILE_SIZE)*MAP_W] == 'b') || 
					(map[(enemy.x)/TILE_SIZE + ((enemy.y)/TILE_SIZE)*MAP_W] == 'b') ) 
					{
					enemy.x += player.speed;
					}
				} 
		
		}

		else
		{
			if(distance_y >=0)//move_tank(UP, enemy);
			{
					enemy.y += enemy.speed;
					enemy.direction = UP;
					enemy.frame++;
					
					if ((map[(enemy.x+30)/TILE_SIZE + ((enemy.y+30)/TILE_SIZE)*MAP_W] == 'b') || 
					(map[(enemy.x)/TILE_SIZE + ((enemy.y+30)/TILE_SIZE)*MAP_W] == 'b') ) 
					{
					enemy.y -= enemy.speed;
					}	
			}
			

			else//move_tank(DOWN, enemy);
			{
			enemy.y -= enemy.speed;
			enemy.direction = DOWN;
			enemy.frame++;
			if ((map[(enemy.x+30)/TILE_SIZE + ((enemy.y)/TILE_SIZE)*MAP_W] == 'b') || 
			(map[(enemy.x)/TILE_SIZE + ((enemy.y)/TILE_SIZE)*MAP_W] == 'b') ) 
			{
			enemy.y += enemy.speed;
			}
			}
			
		}
	}
	}
	// ���ν�Ƽ �ʱ�ȭ
	/*
	if (KEY_DOWN(VK_RETURN))
	{
		for(int i = 0; i<MAX_NUM_BOMBS; i++)
		{
			if(!bombs[i].state){
			bombs[i].state = TRUE;
			Bomb::phase == READY_THROWING;
			
				if(Bomb::phase == READY_THROWING)
				{
				Bomb::phase = CHARGE_POWER;
				}
					if(Bomb::phase == CHARGE_POWER)
					{
					bomb_counter++; // ���� ������ ���� �Ÿ�����
					}
			}
		}
	}
	*/

	// �Ѿ� �߻� �κ�
	/*
		if (wParam == VK_SPACE)
			{
				for(int i = 0; i<10; i++)
				{
					if(! bullet[i].shot)
					{
						bullet[i].shot = TRUE;
						bullet[i].x = player.x+7;
						bullet[i].y = player.y+7; // State Definition int fighterY, missle[i].y = fighterY+20;
						bullet[i].direction = player.direction;
						sound_shot->Play(0);
						break;
					}
				}
			}
	*/

	// �Ѿ� �߻�� ����
	
	if (KEY_DOWN(VK_RETURN))
	{
		Bomb::phase == READY_THROWING;
			if(Bomb::phase == READY_THROWING)
			{
			Bomb::phase = CHARGE_POWER;
			}

			if(Bomb::phase == CHARGE_POWER)
			{
			bomb_counter++;
			}
	}

	// move bombs
	for( int i=0; i < MAX_NUM_BOMBS; i++)
	{
		if(bombs[i].state)
		{
		// �ӵ��� ���� ��ġ ����
		bombs[i].x += bombs[i].vx;
		bombs[i].y += bombs[i].vy;
		bombs[i].z += bombs[i].vz;
		// �߷°��ӵ�(-1)�� ���� z�������� �ӵ� ����
		bombs[i].vz -= 1; // �������� -> z�� 0�� �ɶ� ����
		}
	}

	draw();
}

void draw()
{
	// openvg surface�� �ʱ� ���� �����մϴ�.
	VGfloat clear[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	// OpenVG Setup...
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	vgSetfv(VG_CLEAR_COLOR, 4, clear);
	vgClear(0, 0, cw, ch);
	// TODO: Add your drawing code heare			
	/////////////////////////////////////////////////////////////////////////////////
	// ����� �κ� �ڵ�
	/////////////////////////////////////////////////////////////////////////////////
	// ������� ���
		if ( bgm > 120)  
		sound_ing_bgm->Play(0);
		
		// ������ �б�
		for (int i=0; i<MAP_H; i++) {
			for (int j=0; j<MAP_W; j++) {
			vgLoadIdentity();
			// ī�޶� ��ȯ�� ���� Ÿ�� ��ġ ��ȯ
			vgTranslate(j*TILE_SIZE - cx, i*TILE_SIZE - cy);
				switch (map[j + i*MAP_W]) {
				case 'b': vgDrawImage(block); break;
				case '.': vgDrawImage(item); break;
				case 'c': vgDrawImage(cup); break;
				default: break;
				}
			}
		}

		//�������� Ŭ���� �� ������ ����
		if(cup_count == 6){
			for (int i=0; i<MAP_H; i++) {
			for (int j=0; j<MAP_W; j++) {
			vgLoadIdentity();
			// ī�޶� ��ȯ�� ���� Ÿ�� ��ġ ��ȯ
			vgTranslate(j*TILE_SIZE - cx, i*TILE_SIZE - cy);
				switch (map[j + i*MAP_W]) {
				case 'x': vgDrawImage(stage1); break;
				default: break;
				}
			}
		}
		}

		// ī�޶� ��ġ ��ȯ ���� ĳ���� �̵�
		vgLoadIdentity();
		vgTranslate(player.x - cx, player.y - cy);
		switch (player.direction) {
		case UP: vgDrawImage(player.up[player.frame%2]); break;
		case DOWN: vgDrawImage(player.down[player.frame%2]); break;
		case LEFT: vgDrawImage(player.left[player.frame%2]); break;
		case RIGHT: vgDrawImage(player.right[player.frame%2]); break;
		}

		if (player.isActive) vgDrawImage(player.effect[player.activeTime%2]);

		// �� �׸���
		if(!crush){
		vgLoadIdentity();
		vgTranslate(enemy.x-cx, enemy.y-cy);
		switch (enemy.direction) {
		case UP: vgDrawImage(enemy.up[enemy.frame%2]); break;
		case DOWN: vgDrawImage(enemy.down[enemy.frame%2]); break;
		case LEFT: vgDrawImage(enemy.left[enemy.frame%2]); break;
		case RIGHT: vgDrawImage(enemy.right[enemy.frame%2]); break;
		}
		}

	
	/*
	���� ��ũ�� Ư�����¶�� { 
		Ư������ ����Ʈ�� �������
		�̹����� �ΰ��̴� �̵��� �ִϸ��̼��� ����
	}
	*/
	if (player.isActive)
	{
		vgLoadIdentity();
		vgTranslate(player.x - cx, player.y -cy);
		vgDrawImage(img[e]);
	
	}

	// ���� ���� �׸� ����
	if (gameover)
	{
		vgLoadIdentity();
		vgTranslate(player.x - cx, player.y -cy);
		vgDrawImage(over);
	
	}
	/*
	�ݺ��� ��� �Ѿ� ���ؼ� �˻� {
		���� �Ѿ��� �߻�� ��� {
			vgLoadIdentity();
			vgTranslate(�Ѿ��� ��ġ);
			����ġ �Ѿ��� ���⿡ ���� {
				�� ��쿡 �´� �̹����� ���
			}
		}
	}
	*/
	// �Ѿ� �߻� �׸���
	if ( bgm > 125 && gameover == 0) {
	for (int i=0; i<10; i++){
		if(bullet[i].shot){
			vgLoadIdentity();
			vgTranslate(bullet[i].x -cx, bullet[i].y - cy);
			switch (bullet[i].direction){
				case UP : vgDrawImage(bull_up); break;
				case DOWN : vgDrawImage(bull_down); break;
				case LEFT : vgDrawImage(bull_left); break;
				case RIGHT : vgDrawImage(bull_right); break;
			}
		}
	}
	}
	//��ź �߻� �׸���
	if ( bgm > 125 && gameover == 0) {
	for (int i=0; i<MAX_NUM_BOMBS; i++){
		if(bombs[i].state){
			vgLoadIdentity();
			vgScale(1,1);
			vgTranslate(bombs[i].x -cx, bombs[i].y -cy);
			switch (player.direction){
			case UP : vgDrawImage(bomb); break;
			case DOWN : vgDrawImage(bomb); break;
			case LEFT : vgDrawImage(bomb); break;
			case RIGHT : vgDrawImage(bomb); break;
			
			}
		}
	}
	}
	/////////////////////////////////////////////////////////////////////////////////
	
		if (crush){
			if(crush_timer < 20){
			vgLoadIdentity();
			vgTranslate(enemy.x-cx, enemy.y-cy);
			vgDrawImage(fire.img[e]); // �� ��ũ �ı� �׸�
			crush_timer++;
			}
		}
	
	if (win)  {
		vgTranslate(0, 0);
		vgDrawImage(win_image); // �¸� �̹��� �ø���
	}

	if ( bgm < 125)  {
		vgLoadIdentity();
		vgTranslate(200, 200);
		vgDrawImage(ready); // ������ �̹��� �ø���
	}
	
	eglSwapBuffers(display, surface);
}