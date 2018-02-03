#ifndef EWHEADER
#define EWHEADER

#include <windows.h>

int EWinit(HINSTANCE hInstance, int nCmdShow, int w, int h, const char* titlename, WNDPROC WndProc, int showlogo = 1); //초기화
/*
hInstance, nCmdShow	: WinMain의 파라미터 참조
w, h				: 너비, 높이
titlename			: 창 제목
WndProc				: WM_PAINT, WM_EXIT 외 메세지를 처리하는 WndProc 함수
*/
int EWcheckmsg(); //메세지 체크. 메세지가 있으면 처리. WM_PAINT나 WM_EXIT가 아니면 WndProc 호출.
int EWfin(); //EWIN 라이브러리 종료
int EWwait(unsigned int msdur); //기다리기
BOOL EWclear(); //창을 흰색으로 만들기.
BOOL EWfillB(); //창을 브러쉬색으로 칠하기.
BOOL EWtext(int x, int y, char* szText); //펜으로 글자를 쓰기.
/*
x,y		: 글자 표시 좌표
szText	: 출력할 텍스트
dwLen	: szText의 길이
*/
BOOL EWshow(); // 출력한 화면 표시.
int EWsetBRGB(int r, int g, int b); // r,g,b로 브러쉬 색상 설정
int EWsetPRGB(int penstyle, int border, int r, int g, int b); //펜 스타일을 스타일, 두께(border), rgb로 설정.
int EWsetTransB(); // 배경색을 투명하게 할려면 쓴다. EWsetBRGB로 원상복귀
/*
EWsetPRGB
penstyle		: 스타일
PS_SOLID		: 실선
PS_DASH			: 길게 끊어진 점선. 굵기 1만 가능하다.
PS_DOT			: 점선. 굵기 1만 가능하다.
PS_DASHDOT		: 일점 쇄선. 굵기 1만 가능하다.
PS_DASHDOTDOT	: 이점 쇄선. 굵기 1만 가능하다.
PS_NULL			: 투명 선
PS_INSIDEFRAME	: 실선이되 사각형 안쪽으로만 그려진다.
*/
int EWsetTCRGB(int r, int g, int b); //글자색
int EWsetTBRGB(int r, int g, int b); //형광색
int EWsetTM(int mode);
/*
mode		: 효과
OPAQUE		: 배경불투명
TRANSPARENT	: 배경투명
*/
BOOL EWrect(int x1, int y1, int x2, int y2); //직사각형을 그림
BOOL EWellipse(int x1, int y1, int x2, int y2); //타원을 그림.
BOOL EWline(int x1, int y1, int x2, int y2); //직선 그림
BOOL EWpoint(int x, int y); //점 찍음
BOOL EWpoly(int n, ...);

BOOL EWsetframe(int fps);
BOOL EWstartframe();
BOOL EWendframe();

//coord system
int EWsetcoord(int x, int y, double ratio);
/*
(x,y)에 ratio 비율으로 그림을 그리는 좌표계를 설정한다.
이전 좌표계에 의해 계산된다.
EWtext의 글자 크기 및 좌표는 영향받지 않는다.
*/
int EWendcoord(); //좌표계 해제

//Bitmap system
int EWloadBMP(int no, char *bmpsrc);
/*
no 슬롯에 bmpsrc 경로를 가진 그림 파일을 로드한다.
성공시 0, EWIN이 초기화되지 않았으면 -2, 슬롯이 차있으면 -3,
BMP 파일을 읽지 못했으면 -4을 리턴한다.
*/

int EWshowBMP(int no, int x, int y, int mode = SRCCOPY);
/*
no 슬롯의 비트맵을 화면의 (x,y) 좌표에 출력한다.
mode는 BitBlt에 쓰이는 값들과 동일하므로 이 사이트를 참조한다.
http://www.winapi.co.kr/reference/Function/BitBlt.htm
*/
int EWkillBMP(int no);
/*
no 슬롯에 설정된 그림 파일을 해제시킨다.
성공시 0, EWIN이 초기화되지 않으면 -2, 그 슬롯이 비어있으면 -3을 리턴한다.
*/
#endif
