#include "stdafx.h"
#include "monitoring_RFB.h"
#include "ChildMonitor.h"

#define MAX_LOADSTRING 100
#define INPUT_TIMER 1
#define ALERT_TIMER 2

// 전역 변수:
HINSTANCE g_hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

CMonitorGraphUnit *p1 = nullptr;
CMonitorGraphUnit *p2 = nullptr;
CMonitorGraphUnit *p3 = nullptr;
CMonitorGraphUnit *p4 = nullptr;

// 전역 hdc
HDC g_hdc;

// 알람용
HBRUSH g_redBrush = CreateSolidBrush(RGB(255, 0, 0));
bool g_bAlert = false;

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.
	// 콘솔 창 띄우기
	AllocConsole();
	FILE *acStreamOut;
	freopen_s(&acStreamOut, "CONOUT$", "wt", stdout);

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MONITORINGRFB, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// 기본 메시지 루프입니다.
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MONITORINGRFB));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MONITORINGRFB);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	g_hdc = GetDC(hWnd);

	// 랜덤 시드값
	srand((unsigned int)time(NULL));

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		p1 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::Color::PINK,
			CMonitorGraphUnit::LINE_SINGLE, 10, 10, 200, 200);
		p1->SetInformation(L"p1의 타이틀", 400, 390);

		p2 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::Color::YELLOW,
			CMonitorGraphUnit::LINE_SINGLE, 220, 10, 200, 200);
		p2->SetInformation(L"p2의 타이틀", 100, 97);

		p3 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::Color::GREEN,
			CMonitorGraphUnit::LINE_SINGLE, 430, 10, 400, 200);
		p3->SetInformation(L"p3의 타이틀", 100, 97);

		p4 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::Color::BLUE,
			CMonitorGraphUnit::LINE_MULTI, 10, 220, 300, 250, 4);
		p4->SetInformation(L"p4의 타이틀", 100, 97);

		const ULONG64 ServerID_1 = 0x1122334455667788;
		const ULONG64 ServerID_2 = 0x9988776655443322;

		// 컬럼, X, 타입(id), 컬럼제목
		p1->SetDataColumnInfo(0, ServerID_1, 1);
		p2->SetDataColumnInfo(0, ServerID_1, 2);
		p3->SetDataColumnInfo(0, ServerID_2, 1);

		p4->SetDataColumnInfo(0, ServerID_1, 4, L"릴리즈서버");
		p4->SetDataColumnInfo(1, ServerID_1, 5, L"개판서버");
		p4->SetDataColumnInfo(2, ServerID_1, 6, L"먹튀서버");
		p4->SetDataColumnInfo(3, ServerID_1, 7, L"다운스멜서버");

		SetTimer(hWnd, INPUT_TIMER, 100, NULL);

		// -부모 왼도우 WM_CREATE 에서 자식 윈도우 클래스들 생성.
		// - 타이머도 생성.
	}
	break;
	case UM_ALERT:
	{
		g_bAlert = true;
		SetTimer(hWnd, ALERT_TIMER, 500, NULL);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case WM_TIMER:
	{
		switch (wParam)
		{
		case INPUT_TIMER: // 1
		{
			const ULONG64 ServerID_1 = 0x1122334455667788;
			const ULONG64 ServerID_2 = 0x9988776655443322;

			p1->InsertData(ServerID_1, 1, rand() % 100);
			p2->InsertData(ServerID_1, 2, rand() % 100);
			p3->InsertData(ServerID_2, 1, 1);

			p4->InsertData(ServerID_1, 4, rand() % 100);
			p4->InsertData(ServerID_1, 5, rand() % 100);
			p4->InsertData(ServerID_1, 6, rand() % 100);
			p4->InsertData(ServerID_1, 7, rand() % 100);

			// TOOD : 정확한 의미가? 모든 윈도우로 전송?
			//-타이머는 랜덤하게 데이터를 생성하여 모든 윈도우로 전송.
		}
		break;
		case ALERT_TIMER: // 2
		{
			// 지운다
			InvalidateRect(hWnd, NULL, TRUE);

			// 타이머도 지운다
			KillTimer(hWnd, ALERT_TIMER);

			g_bAlert = false;
		}
		break;
		}
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택을 구문 분석합니다.
		switch (wmId)
		{
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
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.

		if (g_bAlert)
		{
			// 알람
			RECT tempRect;
			GetClientRect(hWnd, &tempRect);
			FillRect(g_hdc, &tempRect, g_redBrush);
		}

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