#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "d2d1")
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <dwrite.h>
#pragma comment(lib, "dwrite")

ID2D1Factory* pD2DFactory = NULL;
ID2D1HwndRenderTarget* pRT = NULL;
IDWriteFactory* m_pDWriteFactory;
IDWriteTextFormat* m_pTextFormat;

/*global variables*/
#define HIBA_00 TEXT("Error:Program initialisation process.")
#define IDC_STATIC -1
#define WINSTYLE_DIALOG (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU)

HINSTANCE hInstGlob;
int SajatiCmdShow;
POINT MousePos;

void ShowMessage(LPCTSTR uzenet, LPCTSTR cim, HWND kuldo);

HWND Form1;
LRESULT CALLBACK WndProc0(HWND, UINT, WPARAM, LPARAM);
char szClassName[] = "WindowsApp";

//*******************
//a játék változói
//*******************
#define ID_MYTIMER1 256

int jatekter[6][8];
D2D1_COLOR_F szinek[10000];

int gyozelem;//0-jatek folyamatban,1-vege
int gomboc_tipus;
int gomboc_tipuslista[10000];//sororles ellenorzesehez
int kov_gomboc_tipus;
int gomboc_id;
int pontszam;

void D2D_text_init(ID2D1HwndRenderTarget* pRT);
void init();
void jatekter_kirajzol();
void gravitacio();
int leeshet_e(int azonosito);
void leeses(int azonosito);
void kov_gomboc();
void uj_gomboc();
int jobbra_mehete(int azonosito);
int balra_mehete(int azonosito);
void jobbra_mozgat(int azonosito);
void balra_mozgat(int azonosito);
int sorellenorzes(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("StdWinClassName");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass0;
	SajatiCmdShow = iCmdShow;
	hInstGlob = hInstance;

	wndclass0.style = CS_HREDRAW | CS_VREDRAW;
	wndclass0.lpfnWndProc = WndProc0;
	wndclass0.cbClsExtra = 0;
	wndclass0.cbWndExtra = 0;
	wndclass0.hInstance = hInstance;
	wndclass0.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass0.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass0.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndclass0.lpszMenuName = NULL;
	wndclass0.lpszClassName = TEXT("WIN0");

	if (!RegisterClass(&wndclass0))
	{
		MessageBox(NULL, HIBA_00, TEXT("Program Start"), MB_ICONERROR); return 0;
	}

	Form1 = CreateWindow(TEXT("WIN0"),
		TEXT("Jatekprogram_2 by Krisztián Fehér"),
		WINSTYLE_DIALOG,
		0,
		0,
		700,
		670,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(Form1, SajatiCmdShow);
	UpdateWindow(Form1);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc0(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int joyxPos,joyGomb;

	switch (message)
	{
	case WM_CREATE:
		if (joyGetNumDevs() > 0) joySetCapture(hwnd,JOYSTICKID1, NULL, FALSE);
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
		pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				hwnd, D2D1::SizeU(700, 670)),
			&pRT);
		SetTimer(hwnd, ID_MYTIMER1, 400, (TIMERPROC)NULL);
		D2D_text_init(pRT);
		init();
		jatekter_kirajzol();		
		return 0;
		//*********************************
		//Képernyõvillogás elkerülése
		//*********************************
	case WM_ERASEBKGND:
		return (LRESULT)1;
	case WM_NOTIFY: {
		return 0; }
	case WM_LBUTTONDOWN:
		//*******************
		//felhasználói kattintás
		//*******************
		MousePos.x = LOWORD(lParam);
		MousePos.y = HIWORD(lParam);
		return 0;
	case MM_JOY1MOVE:
		joyxPos = LOWORD(lParam);
		if (joyxPos == 0)
		{
			if (balra_mehete(gomboc_id) == 0)
			{
				balra_mozgat(gomboc_id);
				Sleep(150);
			}			
		}
		else if (joyxPos == 65535)
		{
			if (jobbra_mehete(gomboc_id) == 0)
			{
				jobbra_mozgat(gomboc_id);
				Sleep(150);
			}
		}

		joyGomb = wParam;
		
		if(joyGomb == 4)
		{
			if (leeshet_e(gomboc_id) == 0) leeses(gomboc_id);
			Sleep(10);
		}
		return 0;
	case WM_SIZE:
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_TIMER:
		switch (wParam)
		{
		case ID_MYTIMER1:
			if (gyozelem == 0)
			{
				gravitacio();
				jatekter_kirajzol();
			}
			else init();
			break;
		}
		return 0;
	case WM_CHAR:
		//*******************
		//játék újraindítása
		//*******************
		switch (wParam)
		{
		case 27:// 'Esc' billentyû
			init();
			return 0;
		}
	case WM_CLOSE:
		KillTimer(hwnd, ID_MYTIMER1);
		pRT->Release();
		pD2DFactory->Release();
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void ShowMessage(LPCTSTR uzenet, LPCTSTR cim, HWND kuldo)
{
	MessageBox(kuldo, uzenet, cim, MB_OK);
}

//*******************
//a játék inicializálása
//*******************
void init()
{
	int i, j;
	for (i = 0; i < 6; ++i)
		for (j = 0; j < 8; ++j)
			jatekter[i][j] = 0;

	pontszam = 0;
	gyozelem = 0;
	gomboc_id = 0;
	kov_gomboc();
	uj_gomboc();	
	kov_gomboc();
	srand((int)GetCurrentTime());
}

//*******************
//a bejelölt cellák kirajzolása
//*******************
void jatekter_kirajzol()
{
	int i, j;
	D2D1_ELLIPSE gomboc = D2D1::Ellipse(
		D2D1::Point2F(0.0f, 0.0f),
		10.0f,
		10.0f
	);
	
	ID2D1SolidColorBrush *hBrush, *hBrush2, * hBrush3;
	pRT->BeginDraw();
	pRT->CreateSolidColorBrush(D2D1::ColorF(1.0F, 1.0F, 1.0F, 3.0F), &hBrush);
	pRT->CreateSolidColorBrush(D2D1::ColorF(0.6F, 0.9F, 0.6F, 1.0F), &hBrush2);
	pRT->CreateSolidColorBrush(D2D1::ColorF(0.2, 0.5, 1.0, 1.0F), &hBrush3);

	pRT->Clear(D2D1::ColorF(D2D1::ColorF(0.0F, 0.0F, 0.0F, 1.0F)));
	
	//jatekter kerete
	if (hBrush != NULL)
	{
		pRT->DrawRectangle(D2D1::RectF(2, 2, 485, 655), hBrush, 2.0);
	}
	
	//a jatekter tartalma
	for (i = 0; i < 6; ++i)
		for (j = 0; j < 8; ++j)
		{
			if (jatekter[i][j] > 0)
			{
				hBrush2->SetColor(szinek[jatekter[i][j]]);
				pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(i * 80 + 45, j * 80 + 45), 40.0f, 40.0f), hBrush2);
				pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(i * 80 + 45, j * 80 + 45), 40.0f, 40.0f), hBrush3,5.0f);
			}
		}

	//pontszám kiírása
	char sc_helloWorld[256];
	wchar_t Wszoveg[256];
	int hossz;
	_itoa_s(pontszam, sc_helloWorld, sizeof(sc_helloWorld), 10);
	hossz = strlen(sc_helloWorld);
	MultiByteToWideChar(CP_ACP, 0, sc_helloWorld, -1, Wszoveg, hossz * 2 + 1);
	hBrush2->SetColor(D2D1::ColorF(D2D1::ColorF(0.8, 0.8, 0.8, 1)));
	pRT->DrawText(
		Wszoveg,
		lstrlenW(Wszoveg),
		m_pTextFormat,
		D2D1::RectF(510, 10, 700, 100),
		hBrush2);

	pRT->EndDraw();
}

//*******************
//blokkok mozgatasa lefele
//*******************
void gravitacio()
{
	int i, j, vanemozgas=0;
	for (i = 1; i < gomboc_id + 1; ++i)
	{
		if (leeshet_e(i) == 0)
		{
			leeses(i);
			vanemozgas = 1;
		}
	}
		
	if (vanemozgas == 0)
	{
		if (sorellenorzes() > 0) return;
		uj_gomboc();
		kov_gomboc();
	}
}

//*******************
//lefele mozgathato-e meg egy adott gomboc: 0-igen, 1-nem
//*******************
int leeshet_e(int azonosito)
{
	int i, j, eredmeny=0,vaneertelme=0;

	//legalso sorban van-e
	for (i = 0, j = 7; i < 6; ++i)
		if (jatekter[i][j] == azonosito)
		{
			return 1;
		}

	for (i = 0; i < 6; ++i)
		for (j = 6; j > -1; --j)
			if (jatekter[i][j] == azonosito)
			{
				vaneertelme = 1; break;
			}
	
	//nincs ertelme tovabb ellenorizni
	if (vaneertelme == 0) return 1;

	//letezik es nem legalul van
	for (i = 0; i < 6; ++i)
		for (j = 6; j > -1; --j)
		{
			if (jatekter[i][j] == azonosito)
			{   //nincs hova leesni
				if (jatekter[i][j + 1] > 0) return 1;
			}
		}

	//igen, leeshet
	return 0;
}

//*******************
//leeses elvegzese
//*******************
void leeses(int azonosito)
{
	int i, j;
	for (i = 0; i < 6; ++i)
	{
		for (j = 6; j > -1; --j)
		{
			if (jatekter[i][j] == azonosito)
			{
				jatekter[i][j + 1] = azonosito;
				jatekter[i][j] = 0;
				return;
			}
		}		
	}		
}

//*******************
//kovetkezo gomboc meghatarozasa
//*******************
void kov_gomboc()
{
	kov_gomboc_tipus = rand() % (1 + 1);	
}

//*******************
//uj gomboc bedobasa, a 4. oszloptól kezdve
//*******************
void uj_gomboc()
{	
	gomboc_tipus = kov_gomboc_tipus;
	++gomboc_id;

	if (gomboc_tipus == 1) szinek[gomboc_id] = D2D1::ColorF(D2D1::ColorF(1.0, 1.0, 1.0, 1.0));
	else if (gomboc_tipus == 0) szinek[gomboc_id] = D2D1::ColorF(D2D1::ColorF(0.0, 0.0, 0.0, 1.0));
	gomboc_tipuslista[gomboc_id] = gomboc_tipus;

	if (jatekter[2][0] > 0)
	{//nincs több hely
		gyozelem = 1;
		return;
	}
	else
	{//van hely
		jatekter[2][0] = gomboc_id;
	}
}

int jobbra_mehete(int azonosito)
{
	int i, j;

	//legalul vane
	for (i = 0; i < 5; ++i)
		if (jatekter[i][7] == azonosito) return 1;

	//az gomboc teljesen szelen vane
	for (i=5,j = 6; j > -1; --j)
		if (jatekter[i][j] == azonosito)
		{
			return 1;
		}

	//szabad-e jobbra
	for (i = 4; i > -1; --i)//****
		for (j = 6; j > -1; --j)
		{
			if (jatekter[i][j] == azonosito)
			{
				if (jatekter[i + 1][j] > 0) return 1;
			}
		}
	return 0;
}

int balra_mehete(int azonosito)
{
	int i, j;

	//legalul vane
	for (i = 0; i < 5; ++i)
		if (jatekter[i][7] == azonosito) return 1;
	
	//az gomboc teljesen szelen vane
	for (i=0,j = 6; j > -1; --j)
		if (jatekter[i][j] == azonosito)
		{
			return 1;
		}
	
	//szabad-e balra
	for (i = 1; i < 6; ++i)
		for (j = 6; j > -1; --j)
		{
			if (jatekter[i][j] == azonosito)
			{
				if (jatekter[i-1][j] > 0) return 1;
			}
		}
	return 0;
}

void jobbra_mozgat(int azonosito)
{
	int i, j;

	for (i = 4; i > -1; --i)
		for (j = 6; j > -1; --j)
		{
			if (jatekter[i][j] == azonosito)
			{
				jatekter[i + 1][j] = azonosito;
				jatekter[i][j] = 0;
				return;
			}
		}
}

void balra_mozgat(int azonosito)
{
	int i, j;

	for (i = 1; i < 6; ++i)
		for (j = 6; j > -1; --j)
		{
			if (jatekter[i][j] == azonosito)
			{
				jatekter[i-1][j] = azonosito;
				jatekter[i][j] = 0;
				return;
			}
		}
}

//*******************
//kesz sorok ellenorzese
//*******************
int sorellenorzes(void)
{
	int i, j,eredmeny=0,sorszamlalo,teljes_sor;

	for (j = 7; j > -1; --j)	
	{
		//teljes sor keresese
		for (i = 0, sorszamlalo = 0; i < 6; ++i)
		{
			if (jatekter[i][j] > 0)
			{
				++sorszamlalo;
			}
		}
		//teljes sor ellenorzese
		if (sorszamlalo == 6)
		{
			teljes_sor = 0;
			for (i = 0; i < 6; ++i) teljes_sor += gomboc_tipuslista[jatekter[i][j]];
			if ((teljes_sor == 0) || (teljes_sor == 6))
			{
				++eredmeny;//**ennyi sor lesz torolve
				for (i = 0; i < 6; ++i) jatekter[i][j] = 0;
				pontszam += 100;
			}			
		}
	}		
	return eredmeny;
}

void D2D_text_init(ID2D1HwndRenderTarget* pRT)
{
	static const WCHAR msc_fontName[] = L"Verdana";
	static const FLOAT msc_fontSize = 52;

	DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

	m_pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		msc_fontSize,
		L"", //locale
		&m_pTextFormat);

	m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}