#include <windows.h>
#include <stdarg.h>
#include "ewin.h"

#define calxpos(x) (int)(xcoord[coordpos] + rcoord[coordpos] * (x))
#define calypos(y) (int)(ycoord[coordpos] + rcoord[coordpos] * (y))

static HINSTANCE hInst;
HWND hWnd;
static WNDCLASSA WndClass;
static HDC hdc;
static HDC DrawDC;
static HBITMAP hOldMemBitMap,hDIBMemBitMap;
static HBRUSH MyBrush,OldBrush;
static HPEN MyPen, OldPen;
static WNDPROC ExtWndProc;
static int width,height;
static int isinited=0;

static HBRUSH trans_brush;

static unsigned framems, deltams;

static int xcoord[256] = {0}, ycoord[256]= {0};
static double rcoord[256] = {1};
int coordpos = 0;

//스프라이트, 이미지 관련
static HDC bitmapdc[1024];
static HBITMAP bitmap[1024];
static HBITMAP oldbitmap[1024];
static int bitmapflag[1024];
static int bmw[1024], bmh[1024];


//VC2008수정 끝

LRESULT CALLBACK InWndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam) {
	PAINTSTRUCT ps;
	switch(iMessage) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		BitBlt(hdc,0,0,width,height,DrawDC,0,0,SRCCOPY);
		EndPaint(hWnd,&ps);
		return 0;
	default:
		return ExtWndProc(hWnd,iMessage,wParam,lParam);
	}
}
int EWcheckmsg() {
	MSG Message;
	if(isinited==0) return 0; //에러
	if(PeekMessage(&Message,0,0,0,PM_REMOVE)) {
		if (Message.message==WM_QUIT) {
			if(isinited==0) return -2;

			int i;
			for(i = 0 ; i < 1024 ; i++) {
				if(bitmapflag[i] == 1) EWkillBMP(i);
			}

			SelectObject(DrawDC,hOldMemBitMap);
			DeleteObject(DrawDC);
			DeleteObject(hDIBMemBitMap);

			SelectObject(DrawDC,OldBrush);
			DeleteObject(MyBrush);

			SelectObject(DrawDC,OldPen);
			DeleteObject(MyPen);

			isinited=0;
			return 0;
		}
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return 1;
}
BOOL EWclear() {
	if(isinited==0) return -2; //에러
	return PatBlt(DrawDC,0,0,width,height,WHITENESS);
}
BOOL EWfillB() {
	if(isinited==0) return -2; //에러
	return PatBlt(DrawDC,0,0,width,height,PATCOPY);
}
BOOL EWtext(int x, int y, char* szText) {
	if(isinited==0) return -2; //에러

	int dwLen = strlen(szText);
	return TextOutA(DrawDC,x,y, szText, dwLen);
}
BOOL EWshow() {
	if(isinited==0) return -2; //에러
	//return InvalidateRect(hWnd,NULL,FALSE);
	return BitBlt(hdc,0,0,width,height,DrawDC,0,0,SRCCOPY);
}

int EWsetBRGB(int r, int g, int b) {
	if(isinited==0) return -2; //에러
	SelectObject(DrawDC,OldBrush);
	DeleteObject(MyBrush);
	MyBrush=CreateSolidBrush(RGB(r,g,b));
	OldBrush=(HBRUSH)SelectObject(DrawDC,MyBrush);
	return 1;
}

int EWsetPRGB(int penstyle, int border, int r, int g, int b) {
	if(isinited==0) return -2; //에러
	SelectObject(DrawDC,OldPen);
	DeleteObject(MyPen);
	MyPen=CreatePen(penstyle,(int)(rcoord[coordpos] * (border-1) + 0.5), RGB(r,g,b));
	OldPen=(HPEN)SelectObject(DrawDC,MyPen);
	return 1;
}

int EWsetTCRGB(int r, int g, int b) {
	if(isinited==0) return -2; //에러
	SetTextColor(DrawDC,RGB(r,g,b));
	return 1;
}
int EWsetTBRGB(int r, int g, int b) {
	if(isinited==0) return -2; //에러
	SetBkColor(DrawDC,RGB(r,g,b));
	return 1;
}
int EWsetTM(int mode) {
	if(isinited==0) return -1;
	return SetBkMode(DrawDC,mode);
}
BOOL EWrect(int x1, int y1, int x2, int y2) {
	if(isinited==0) return -2;
	return Rectangle(DrawDC,calxpos(x1),calypos(y1),calxpos(x2),calypos(y2));
}
BOOL EWellipse(int x1, int y1, int x2, int y2) {
	if(isinited==0) return -1;
	return Ellipse(DrawDC,calxpos(x1),calypos(y1),calxpos(x2),calypos(y2));
}
BOOL EWline(int x1, int y1, int x2, int y2) {
	if(isinited==0) return -1;
	MoveToEx(DrawDC,calxpos(x1),calypos(y1),NULL);
	return LineTo(DrawDC,calxpos(x2),calypos(y2));
}
BOOL EWpoint(int x, int y) {
	if(isinited==0) return -1;
	return Rectangle(DrawDC,calxpos(x),calypos(y),calxpos(x+1),calypos(y+1));
}
BOOL EWwait(unsigned int msdur) {
	unsigned int end=msdur+GetTickCount();
	while(GetTickCount()<end) {
		if(!EWcheckmsg()) return FALSE;
		Sleep(2);
	}
	return TRUE;
}

int EWsetTransB() {
    trans_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(DrawDC, trans_brush);
    return 1;
}

#define MAX_POINT 100

BOOL EWpoly(int n, ...) {
    va_list ap;
    int i, ptr;
    POINT pt[MAX_POINT];

    va_start(ap, n);

    for(i = 0 ; i < n ; i++) {
        ptr = va_arg(ap, int);
        pt[i].x = calxpos(ptr);

        ptr = va_arg(ap, int);
        pt[i].y = calypos(ptr);
    }

    Polygon(DrawDC, pt, n);
    return TRUE;
}
int EWinit(HINSTANCE hInstance, int nCmdShow, int w, int h, const char* szTitlename, WNDPROC WndProc, int showlogo) {
	if(isinited==1) return -2; //에러

	const char* szClassName = "epTraceApp";
	RECT wSize;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &wSize, 0);
	w = wSize.right - wSize.left;
	h = wSize.bottom - wSize.top;

	WndClass.cbClsExtra=0;
	WndClass.cbWndExtra=0;
	WndClass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	WndClass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	WndClass.hInstance=hInstance;
	WndClass.lpfnWndProc=InWndProc;
	ExtWndProc=DefWindowProc;

	WndClass.lpszClassName = szClassName;
	WndClass.lpszMenuName=NULL;
	WndClass.style=CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	RegisterClassA(&WndClass);

	hWnd = CreateWindowA(szClassName, nullptr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
		| WS_MINIMIZEBOX, 0, 0,
		w, h, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	isinited=1;
	width=w;
	height=h;
	hdc=GetDC(hWnd);

	DrawDC=CreateCompatibleDC(hdc);
	hDIBMemBitMap=CreateCompatibleBitmap(hdc,w,h);
	hOldMemBitMap=(HBITMAP)SelectObject(DrawDC,hDIBMemBitMap);

	MyBrush=CreateSolidBrush(RGB(0,0,0));
	OldBrush=(HBRUSH)SelectObject(hdc,MyBrush);

	MyPen=CreatePen(PS_SOLID,0,RGB(0,0,0));
	OldPen=(HPEN)SelectObject(DrawDC,MyPen);

	for(int i = 0 ; i < 1024 ; i++) {
		bitmapflag[i] = 0;
	}

	hInst = hInstance;

	//로고
	if(showlogo) {
		EWclear();
		EWrect(w/2-80,h/2-20,w/2+80,h/2+20);
		EWtext(w/2-72,h/2-17,"Powered by EasyWin");
	 	EWsetBRGB(0,0,0);
	 	EWrect(w/2-80,h/2,w/2+80,h/2+20);
		EWsetTBRGB(0,0,0);
		EWsetTCRGB(255,255,255);
		EWtext(w/2-72,h/2+1,"phu54321@naver.com");
		EWshow();
		EWwait(700);
	}
	EWsetBRGB(255,255,255);
	EWsetTBRGB(255,255,255);
	EWsetTCRGB(0,0,0);
	EWclear();
	ExtWndProc=WndProc;
	return 0;
}
int EWfin() {
	if(isinited==0) return -2;
	PostQuitMessage(0);
	return 0;
}

BOOL EWsetframe(int fps) {
	if(isinited==0) return -2;
    deltams = 1000 / fps;
    framems = GetTickCount() + deltams;
    return TRUE;
}

BOOL EWstartframe() {
	if(isinited==0) return -2;
    if(GetTickCount() > framems) {
        framems += deltams;
        return FALSE;
    }

    return EWwait(framems - GetTickCount());
}
BOOL EWendframe() {
	if(isinited==0) return -2;
    framems += deltams;
    return TRUE;
}

int EWsetcoord(int x, int y, double ratio) {
	if(isinited==0) return -2;
	xcoord[coordpos+1] = (int)(xcoord[coordpos] + x * rcoord[coordpos]);
	ycoord[coordpos+1] = (int)(ycoord[coordpos] + y * rcoord[coordpos]);
	rcoord[coordpos+1] = ratio * rcoord[coordpos];
	coordpos++;
	return 0;
}

int EWendcoord() {
	if(isinited==0) return -2;
	if(coordpos == 0) return -1;
	coordpos--;
	return 0;
}

//이미지 및 스프라이트 처리

int EWloadBMP(int no, char *bmpsrc) {
	if(isinited==0) return -2; //ewin not inited
	if(bitmapflag[no] == 1) return -3; //already full;

	bitmap[no]=(HBITMAP)LoadImageA(NULL,bmpsrc,IMAGE_BITMAP,
			0,0,LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if(bitmap[no] == NULL) return -4; //Bitmap not readable

	BITMAP bm;
	GetObject(bitmap[no], sizeof(bm), &bm);

	bmw[no] = bm.bmWidth;
	bmh[no] = bm.bmHeight;

	bitmapdc[no] = CreateCompatibleDC(DrawDC);
	oldbitmap[no] = (HBITMAP)SelectObject(bitmapdc[no], bitmap[no]);

	bitmapflag[no] = 1;
	return 0; //No error;
}

int EWshowBMP(int no, int x, int y, int mode) {
	if(isinited == 0) return -2;
	if(bitmapflag[no] != 1) return -3;
	return BitBlt(DrawDC, x, y, bmw[no], bmh[no], bitmapdc[no], 0, 0, mode);
}

int EWkillBMP(int no) {
	if(isinited==0) return -2;
	if(bitmapflag[no] != 1) return -3; //empty slot

	SelectObject(bitmapdc[no], oldbitmap[no]);
	DeleteObject(bitmapdc[no]);
	DeleteObject(bitmap[no]);
	bitmapflag[no] = 0;

	return 0; //No error;
}

//마우스 및 키보드 처리
