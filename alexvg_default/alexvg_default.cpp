/*
게임설명
enermy = 플레이어 따라옴, 충돌시 게임오버, 속도 더 빠름
별 아이템 = 스피드업
컵 아이템 = 다먹어야 스테이지 클리어
컵을 다 먹으면 -> stage2로 넘어가는 문이 생기고, 그 문에 접촉해야 클리어

stage1 = cup 6개

*/
#include "stdafx.h"
#include "alexvg_default.h"

#include "LoadImage_vcl.h"
#include <egl/egl.h>
#include <vg/openvg.h>
#include <vg/vgu.h>

#include <time.h>
#include <fstream>

// 사운드 setup include
#include "DXUTsound.h" // 빌드 해야되서 포함

// 사운드 라이브러리 추가
#pragma comment (lib, "winmm")
#pragma comment (lib, "dsound")
#pragma comment (lib, "dxguid")

#define M_PI       3.14159265358979323846

#define MAX_LOADSTRING 100

#define MY_WIDTH 19 * 34       // window 초기가로크기
#define MY_HEIGHT 14 * 34       // window 초기세로크기

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

// window size와 관련된 변수
int cw, ch;
int cx, cy;

float ws = 1.0f;
float hs = 1.0f;

// 이미지 관련 변수
VGImage img[2];
VGfloat x, y;

// Update 관련 함수
void timeProc();
void draw();
void move_tank();

// 사운드 초기화
CSoundManager* sm = NULL; 
CSound *sound_bgm = NULL; // It’s a just example
CSound *sound_ing_bgm = NULL;
CSound *sound_item = NULL;
CSound *sound_shot = NULL;
CSound *sound_move = NULL;
CSound *sound_broken = NULL;
CSound *sound_hit = NULL;

// 배경음악 타이머
int bgm = 0;

// ready, cup, over이미지
VGImage ready;
VGImage over;
VGImage cup;
VGImage stage1;
VGImage win_image;

// 게임오버
int gameover = 0;

// 무적
int god = 0;

// 승리조건
int cup_count = 0;
int win = 0;

//붐
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


// 맵 정보
#define MAP_W		23 // 782
#define MAP_H		16 // 544
#define TILE_SIZE	34

// 맵 배열
char map[MAP_W*MAP_H];

// 맵 이미지
VGImage block;
VGImage item;

// 움직임 상수
#define LEFT  0
#define RIGHT 1
#define UP    2
#define DOWN  3

// 플레이어 탱크 구조체
struct Tank {
	int x; // VGfloat
	int y;
	unsigned isActive; // 010101
	int activeTime; // 아이템 먹었을때
	unsigned speed;
	unsigned direction;
	unsigned frame;
	VGImage up[2], down[2], left[2], right[2], effect[2];
} player;

// 적 탱크 구조체
struct Tank2 {
	int x; // VGfloat
	int y;
	unsigned isActive; // 010101
	int activeTime; // 아이템 먹었을때
	unsigned speed;
	unsigned direction;
	unsigned frame;
	VGImage up[2], down[2], left[2], right[2], effect[2];
} enemy;

// 폭탄 구조체
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

//발사 구조체
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
총알(Bullet) 구조체 {
	정수형 좌표;
	정수형 발사여부;
	정수형 스피드; // 모든 총알이 따로 가질 필요는 없으나 탱크 구조체를 바탕으로 해서 존재.
	정수형 방향;
} 총알들[총알 최대갯수만큼]
총알 이미지 위, 아래, 좌, 우 // 각 방향에 대해서 이미지 선언
*/
/////////////////////////////////////////////////////////////////////////////////////
struct Bull{
	VGfloat x, y;
	unsigned shot;
	unsigned direction;
	int speed;
} bullet [10];
VGImage bull_left, bull_right, bull_up, bull_down;



HINSTANCE hInst;					// 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];				// 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];			// 기본 창 클래스 이름입니다.

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
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

	// TODO: 여기에 코드를 입력합니다.
	MSG msg;
	HACCEL hAccelTable;

	// 전역 문자열을 초기화합니다.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ALEXVG_DEFAULT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	
	// 응용 프로그램 초기화를 수행합니다.
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
		
		if (GetTickCount() - lastTime > 32) { // 30프레임
			lastTime += 33;
			timeProc();
		}
		

	}

	// 기본 메시지 루프입니다.
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
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
//  설명:
//
//    Windows 95에서 추가된 'RegisterClassEx' 함수보다 먼저
//    해당 코드가 Win32 시스템과 호환되도록
//    하려는 경우에만 이 함수를 사용합니다. 이 함수를 호출해야
//    해당 응용 프로그램에 연결된
//    '올바른 형식의' 작은 아이콘을 가져올 수 있습니다.
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
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	  CW_USEDEFAULT, 0, MY_WIDTH, MY_HEIGHT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
	  return FALSE;
   }
   
   // 사운드 초기화 & 재생
   sm = new CSoundManager();
   sm->Initialize( hWnd, DSSCL_PRIORITY );
   sm->SetPrimaryBufferFormat(2,22050,16); // It’s a just example. sm->Create(&sound_bgm,TEXT("bgm.wav"),0,GUID_NULL);
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
//   함수: AppInit( NativeWindowType native_window, NativeDisplayType native_display, int cw, int ch )
//				   
//   목적: EGL surface를 초기화 한다. 이미지를 로드한다.
//
//   설명: 이 함수를 통해 그리기 위한 EGL surface를 초기화 합니다.

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

	

	// 이미지 로드

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
	// ready,cup,over 이미지
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
	// 폭탄 이미지
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

	// Initialize tank properties 사용자 탱크 초기화
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

	// Initialize enemy properties 적 탱크 초기화
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

	
	//각 방향에 대한 총알 이미지 로드
	for( int i=0; i<10; i++ ){
	bullet[i].shot = 0;
	bullet[i].x = 0;
	bullet[i].y = 0;
	}

	//폭탄 초기화
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
	반복문 총알 갯수만큼 {
	   각 총알 스피드는 4로 한다.
	}
	*/

	for (int i=0; i<10; i++)
	{
		bullet[i].speed = 4;
	}
	
	return 1;
}

//
//   함수: AppRelease( void )
//
//   목적: EGL surface를 release한다. 이미지를 삭제 한다.
//
//   설명:  이 함수를 통해 EGL surface를 release합니다.   
//
int AppRelease( void )
{
	/////////////////////////////////////////////////////////////////////////////////
	// 릴리즈
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
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND	- 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT	- 주 창을 그립니다.
//  WM_DESTROY	- 종료 메시지를 게시하고 반환합니다.
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
		// 메뉴의 선택 영역을 구문 분석합니다.
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
		// TODO: 여기에 그리기 코드를 추가합니다.
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:				//윈도우의 크기가 변하였을 경우
		{
			GetClientRect(hWnd, &rect);
			ww = rect.right - rect.left;	// 클라이언트 윈도우의 틀를 제외한 가로 길이
			hh = rect.bottom - rect.top;	// 클라이언트 윈도우의 틀를 제외한 세로 길이

			// 가로 크기가 800보다 클경우 800으로 고정합니다(alexVG의 image클래스의 
			// 버그때문에 fix시킴)
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

			// 사이즈가 변했을 경우 EGL surface를 release 하고 다시 현재 크기로 재생성
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
	// 키보드 부분 코드 설명
	/////////////////////////////////////////////////////////////////////////////////
	/*
	키보드를 눌렀다 땠을 때의 이벤트 처리:
	{
		만일 스페이스바를 눌렀을 경우 {
			반복문 모든 총알에 대해서 검사 {
				만일 총알이 발사되지 않은 경우 {
					총알이 발사됐다고 변경;
					총알 초기위치를 탱크의 정가운데로 설정;
					총알의 방향을 탱크의 방향으로 저장;
					반복문 종료;
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

							case UP: // y축 진행
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

// 정보 대화 상자의 메시지 처리기입니다.
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
	// 탱크 물리 움직임 코드 설명
	/////////////////////////////////////////////////////////////////////////////////
	// Tank Movement
	if (player.isActive)
	{
		player.speed = 5;
	}

	/// 배경음악 + 못움직이게 함
	bgm ++;

	// 탱크충돌 
	if(!crush){
		if ( player.x < (enemy.x+30) && (player.x+30) > enemy.x && player.y < (enemy.
		y+30) && (player.y+30) > enemy.y)
		{
			gameover = 1;
			sound_broken->Play(0);
		}
	}

	// 총알충돌
	if(!crush){
		for (int i=0; i<10; i++)
		if(bullet[i].x < (enemy.x+30) && ((bullet[i].x+30) > enemy.x && bullet[i].y < (enemy.y+30) && (bullet[i].y+30) > enemy.y))
		bullet[i].shot = FALSE;
	}
		if (win==1)
		{
			gameover = 1;
		}
	
	// 폭탄충돌
		if(!crush){
		for (int i=0; i<10; i++)
		if(bombs[i].x < (enemy.x+30) && ((bombs[i].x+30) > enemy.x && bombs[i].y < (enemy.y+30) && (bombs[i].y+30) > enemy.y)){
		crush = TRUE;
		bombs[i].state = FALSE;
		
		}
		}

		
	// 플레이어 움직임
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
	// 아이템 코드 설명
	만일 맵상에서 탱크가 아이템위에 있다면 {
		탱크의 특수상태 = 1;	// 초기화가 0으로 되어있으니 1로 간단히 구분
		맵정보[탱크의 맵상 현재 위치] = 빈칸으로 변경;
	}
	만일 탱크가 특수상태이면 {
		탱크의 특수상태타이머 ++;
	}
	만일 탱크의 특수상태로 변한지 10초가 지났다면 {
		탱크의 특수상태 = 0;
		탱크의 특수상태타이머 = 0;
	}
	*/
	// 스피드 아이템
	if( map[((int)player.y/34) * MAP_W + ((int)player.x/32) ] == '.')
	{
		 player.isActive = TRUE;
		 map[((int)player.y/34) * MAP_W + ((int)player.x/32)] = ' ';
		 sound_item->Play(0);
		
	}
	// 컵 아이템
	if( map[((int)player.y/34) * MAP_W + ((int)player.x/32) ] == 'c')
	{
		 cup_count++; // 컵 갯수 증가
		 map[((int)player.y/34) * MAP_W + ((int)player.x/32)] = ' ';
		 sound_item->Play(0);
		
	}
	// 승리조건
	if(cup_count == 6){
	if( map[((int)player.y/34) * MAP_W + ((int)player.x/32) ] == 'x')
	{
		 win = 1; // 컵 6개 먹으먼 승리조건 세움
		 map[((int)player.y/34) * MAP_W + ((int)player.x/32)] = ' ';
		 sound_item->Play(0);
		
	}
	}

	// 특수 아이템 시간
	if (player.isActive)
	{
		player.activeTime ++;
		
	}

	// 300프레임 = 10초
	if ( player.activeTime > 300)
	{
		player.isActive = FALSE;
		player.speed = 3;
		player.activeTime = 0;
	
	}

	/*
	반복문 모든 총알에 대해서 검사 {
		만일 총알의 위치가 클라이언트화면(윈도우창)을 벗어난 경우 {
			// 전역변수로 선언했을시 초기화가 0으로 됐을터이니, 0을 발사하지
			// 않은 상태로 하고 1을 발사한 상태로 구분한다.
			총알 발사여부 = 0;
		}
		// 충돌 검사 코드
		if (map[bullet[i].x/TILE_SIZE + (bullet[i].y/TILE_SIZE)*MAP_W] == 'b') {
			bullet[i].isShoot = 0;
		}
		만일 총알이 발사했을 경우 {
			스위치 총알의 방향 {
				각 방향에 맞게 총알의 위치를 총알의 스피드만큼 증가시키거나 감소시킨다;
			}
		}
	}
	*/

		 for (int i=0; i<10; i++) {
		  // 스크롤
		  if ((bullet[i].x < 0 || bullet[i].x > MY_WIDTH) || (bullet[i].y < 0 || bullet[i].y > MY_HEIGHT))
		  {
			  bullet[i].shot = FALSE;
		  }
		  // 타일
		  if (map[((int)bullet[i].x)/TILE_SIZE + ((int)bullet[i].y/TILE_SIZE)*MAP_W] == 'b') {
		  bullet[i].shot = FALSE;
		  }

		    // 스크롤
		  if ((bombs[i].x < 0 || bombs[i].x > MY_WIDTH) || (bombs[i].y < 0 || bombs[i].y > MY_HEIGHT))
		  {
			  bombs[i].state = FALSE;
		  }

		  // 총알 스피드 방향 속성
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
		// 특수이펙트 애니메이션
		if (e == 0) e = e + 1; 
		else e = 0;

		// 횡 스크롤링
		// 카메라(cx) 의 위치를 화면 중앙으로 설정
		cx = player.x - MY_WIDTH/2;
		//카메라 이동 범위 제한
		if (cx < 0) cx = 0;
		if (cx > (MAP_W)*TILE_SIZE - MY_WIDTH+15)
		cx = (MAP_W)*TILE_SIZE - MY_WIDTH+15;
		// 종 스크롤링
		// 카메라(cy)의 위치 설정
		// 카메라 이동 범위 제한

		cy = player.y - MY_HEIGHT/2;
		//카메라 이동 범위 제한
		if (cy < 0) cy = 0;
		if (cy > (MAP_H)*TILE_SIZE - MY_HEIGHT+TILE_SIZE+13)
		cy = (MAP_H)*TILE_SIZE - MY_HEIGHT+TILE_SIZE+13;

	if(!crush){
	//적 인공지능 코드
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
	// 벨로시티 초기화
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
					bomb_counter++; // 엔터 누르는 동안 거리누적
					}
			}
		}
	}
	*/

	// 총알 발사 부분
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

	// 총알 발사랑 유사
	
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
		// 속도에 따른 위치 설정
		bombs[i].x += bombs[i].vx;
		bombs[i].y += bombs[i].vy;
		bombs[i].z += bombs[i].vz;
		// 중력가속도(-1)에 대한 z축으로의 속도 조절
		bombs[i].vz -= 1; // 떨어지게 -> z가 0이 될때 종료
		}
	}

	draw();
}

void draw()
{
	// openvg surface의 초기 색을 지정합니다.
	VGfloat clear[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	// OpenVG Setup...
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	vgSetfv(VG_CLEAR_COLOR, 4, clear);
	vgClear(0, 0, cw, ch);
	// TODO: Add your drawing code heare			
	/////////////////////////////////////////////////////////////////////////////////
	// 드로윙 부분 코드
	/////////////////////////////////////////////////////////////////////////////////
	// 배경음악 재생
		if ( bgm > 120)  
		sound_ing_bgm->Play(0);
		
		// 맵정보 읽기
		for (int i=0; i<MAP_H; i++) {
			for (int j=0; j<MAP_W; j++) {
			vgLoadIdentity();
			// 카메라 변환에 따른 타일 위치 변환
			vgTranslate(j*TILE_SIZE - cx, i*TILE_SIZE - cy);
				switch (map[j + i*MAP_W]) {
				case 'b': vgDrawImage(block); break;
				case '.': vgDrawImage(item); break;
				case 'c': vgDrawImage(cup); break;
				default: break;
				}
			}
		}

		//스테이지 클리어 문 열리는 조건
		if(cup_count == 6){
			for (int i=0; i<MAP_H; i++) {
			for (int j=0; j<MAP_W; j++) {
			vgLoadIdentity();
			// 카메라 변환에 따른 타일 위치 변환
			vgTranslate(j*TILE_SIZE - cx, i*TILE_SIZE - cy);
				switch (map[j + i*MAP_W]) {
				case 'x': vgDrawImage(stage1); break;
				default: break;
				}
			}
		}
		}

		// 카메라 위치 변환 따른 캐릭터 이동
		vgLoadIdentity();
		vgTranslate(player.x - cx, player.y - cy);
		switch (player.direction) {
		case UP: vgDrawImage(player.up[player.frame%2]); break;
		case DOWN: vgDrawImage(player.down[player.frame%2]); break;
		case LEFT: vgDrawImage(player.left[player.frame%2]); break;
		case RIGHT: vgDrawImage(player.right[player.frame%2]); break;
		}

		if (player.isActive) vgDrawImage(player.effect[player.activeTime%2]);

		// 적 그리기
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
	만일 탱크가 특수상태라면 { 
		특수상태 이펙트를 덧씌운다
		이미지가 두개이니 이동시 애니메이션을 응용
	}
	*/
	if (player.isActive)
	{
		vgLoadIdentity();
		vgTranslate(player.x - cx, player.y -cy);
		vgDrawImage(img[e]);
	
	}

	// 게임 오버 그림 띄우기
	if (gameover)
	{
		vgLoadIdentity();
		vgTranslate(player.x - cx, player.y -cy);
		vgDrawImage(over);
	
	}
	/*
	반복문 모든 총알 대해서 검사 {
		만일 총알이 발사된 경우 {
			vgLoadIdentity();
			vgTranslate(총알의 위치);
			스위치 총알의 방향에 따라 {
				각 경우에 맞는 이미지를 출력
			}
		}
	}
	*/
	// 총알 발사 그리기
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
	//폭탄 발사 그리기
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
			vgDrawImage(fire.img[e]); // 적 탱크 파괴 그림
			crush_timer++;
			}
		}
	
	if (win)  {
		vgTranslate(0, 0);
		vgDrawImage(win_image); // 승리 이미지 올리기
	}

	if ( bgm < 125)  {
		vgLoadIdentity();
		vgTranslate(200, 200);
		vgDrawImage(ready); // 오프닝 이미지 올리기
	}
	
	eglSwapBuffers(display, surface);
}