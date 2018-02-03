#ifndef EWHEADER
#define EWHEADER

#include <windows.h>

int EWinit(HINSTANCE hInstance, int nCmdShow, int w, int h, const char* titlename, WNDPROC WndProc, int showlogo = 1); //�ʱ�ȭ
/*
hInstance, nCmdShow	: WinMain�� �Ķ���� ����
w, h				: �ʺ�, ����
titlename			: â ����
WndProc				: WM_PAINT, WM_EXIT �� �޼����� ó���ϴ� WndProc �Լ�
*/
int EWcheckmsg(); //�޼��� üũ. �޼����� ������ ó��. WM_PAINT�� WM_EXIT�� �ƴϸ� WndProc ȣ��.
int EWfin(); //EWIN ���̺귯�� ����
int EWwait(unsigned int msdur); //��ٸ���
BOOL EWclear(); //â�� ������� �����.
BOOL EWfillB(); //â�� �귯�������� ĥ�ϱ�.
BOOL EWtext(int x, int y, char* szText); //������ ���ڸ� ����.
/*
x,y		: ���� ǥ�� ��ǥ
szText	: ����� �ؽ�Ʈ
dwLen	: szText�� ����
*/
BOOL EWshow(); // ����� ȭ�� ǥ��.
int EWsetBRGB(int r, int g, int b); // r,g,b�� �귯�� ���� ����
int EWsetPRGB(int penstyle, int border, int r, int g, int b); //�� ��Ÿ���� ��Ÿ��, �β�(border), rgb�� ����.
int EWsetTransB(); // ������ �����ϰ� �ҷ��� ����. EWsetBRGB�� ���󺹱�
/*
EWsetPRGB
penstyle		: ��Ÿ��
PS_SOLID		: �Ǽ�
PS_DASH			: ��� ������ ����. ���� 1�� �����ϴ�.
PS_DOT			: ����. ���� 1�� �����ϴ�.
PS_DASHDOT		: ���� �⼱. ���� 1�� �����ϴ�.
PS_DASHDOTDOT	: ���� �⼱. ���� 1�� �����ϴ�.
PS_NULL			: ���� ��
PS_INSIDEFRAME	: �Ǽ��̵� �簢�� �������θ� �׷�����.
*/
int EWsetTCRGB(int r, int g, int b); //���ڻ�
int EWsetTBRGB(int r, int g, int b); //������
int EWsetTM(int mode);
/*
mode		: ȿ��
OPAQUE		: ��������
TRANSPARENT	: �������
*/
BOOL EWrect(int x1, int y1, int x2, int y2); //���簢���� �׸�
BOOL EWellipse(int x1, int y1, int x2, int y2); //Ÿ���� �׸�.
BOOL EWline(int x1, int y1, int x2, int y2); //���� �׸�
BOOL EWpoint(int x, int y); //�� ����
BOOL EWpoly(int n, ...);

BOOL EWsetframe(int fps);
BOOL EWstartframe();
BOOL EWendframe();

//coord system
int EWsetcoord(int x, int y, double ratio);
/*
(x,y)�� ratio �������� �׸��� �׸��� ��ǥ�踦 �����Ѵ�.
���� ��ǥ�迡 ���� ���ȴ�.
EWtext�� ���� ũ�� �� ��ǥ�� ������� �ʴ´�.
*/
int EWendcoord(); //��ǥ�� ����

//Bitmap system
int EWloadBMP(int no, char *bmpsrc);
/*
no ���Կ� bmpsrc ��θ� ���� �׸� ������ �ε��Ѵ�.
������ 0, EWIN�� �ʱ�ȭ���� �ʾ����� -2, ������ �������� -3,
BMP ������ ���� �������� -4�� �����Ѵ�.
*/

int EWshowBMP(int no, int x, int y, int mode = SRCCOPY);
/*
no ������ ��Ʈ���� ȭ���� (x,y) ��ǥ�� ����Ѵ�.
mode�� BitBlt�� ���̴� ����� �����ϹǷ� �� ����Ʈ�� �����Ѵ�.
http://www.winapi.co.kr/reference/Function/BitBlt.htm
*/
int EWkillBMP(int no);
/*
no ���Կ� ������ �׸� ������ ������Ų��.
������ 0, EWIN�� �ʱ�ȭ���� ������ -2, �� ������ ��������� -3�� �����Ѵ�.
*/
#endif
