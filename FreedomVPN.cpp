#include "framework.h"
#include "FreedomVPN.h"
#include "WireGuardIntegration.h"
#include "CyrillicSupport.h"

#include <windows.h>    
#include <objidl.h>      
#include <gdiplus.h>    
#include <commctrl.h>
#include <commdlg.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <cmath>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;




#define MAX_LOADSTRING 100

#define IDC_CONNECT_BUTTON 1001

#define IDC_LOCATION_BUTTON 1003

#define IDC_STATUS_TEXT 1004

#define IDC_TIMER_TEXT 1005

#define IDC_PROTOCOL_TEXT 1006

#define IDC_RECONNECT_BUTTON 1007

#define IDC_IP_TEXT 1010

#define IDC_USAGE_TEXT 1011



#define COLOR_BG_TOP RGB(11, 15, 22)

#define COLOR_BG_BOTTOM RGB(20, 28, 40)

#define COLOR_BG RGB(15, 20, 30)

#define COLOR_GLASS RGB(26, 34, 48)

#define COLOR_GLASS_LIGHT RGB(30, 40, 55)

#define COLOR_TEXT_PRIMARY RGB(232, 232, 232)

#define COLOR_TEXT_SECONDARY RGB(154, 162, 175)

#define COLOR_NEON_CYAN RGB(0, 207, 255)

#define COLOR_NEON_TEAL RGB(0, 255, 183)

#define COLOR_SUCCESS RGB(36, 211, 102)

#define COLOR_ERROR RGB(255, 76, 76)

#define COLOR_DISCONNECTED RGB(60, 70, 85)

#define COLOR_DISCONNECTED_HOVER RGB(75, 85, 100)

#define COLOR_HEADER RGB(16, 21, 28)

#define COLOR_SHADOW RGB(0, 0, 0)



#define TITLEBAR_HEIGHT 45

#define APP_WIDTH 350

#define APP_HEIGHT 550



#define ID_CONNECTION_TIMER 1001

#define ID_TRAFFIC_TIMER 1002

#define ID_ANIMATION_TIMER 1003

#define ID_RECONNECT_TIMER 1004



HINSTANCE hInst;

WCHAR szTitle[MAX_LOADSTRING];

WCHAR szWindowClass[MAX_LOADSTRING];

WireGuardIntegration wireguard;

std::chrono::steady_clock::time_point connectionStartTime;

bool isConnected = false;

bool isConnecting = false;

bool isConnectHover = false;

bool isLocationHover = false;

bool isCloseHover = false;

bool isMinimizeHover = false;

bool isReconnectHover = false;

HWND hoverButton = NULL;

uint64_t downloadedBytes = 0;

uint64_t uploadedBytes = 0;

float animationPhase = 0.0f;

float rotationAngle = 0.0f;

int reconnectAttempts = 0;

bool autoReconnect = true;



bool g_isDragging = false;

POINT g_dragOffset;



HWND g_hWnd;

HWND g_hConnectButton;

HWND g_hLocationButton;

HWND g_hStatusBar;

HWND g_hTimerText;

HWND g_hProtocolText;

HWND g_hReconnectButton;

HWND g_hIPText;

HWND g_hUsageText;



HBRUSH g_hBackgroundBrush;

HFONT g_hTitleFont;

HFONT g_hButtonFont;

HFONT g_hStatusFont;

HFONT g_hSmallFont;

HFONT g_hMonoFont;

HBITMAP g_hGermanyFlag;

HBITMAP g_hAppIcon;



ATOM MyRegisterClass(HINSTANCE hInstance);

BOOL InitInstance(HINSTANCE, int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

void UpdateStatus();

void InitializeUI();

void CleanupUI();

HFONT CreateCustomFont(const wchar_t* fontName, int size, bool bold = false);

void DrawRoundedRectangle(HDC hdc, RECT rect, int radius, COLORREF color);

void DrawTitleBar(HDC hdc);

void DrawConnectButton(HDC hdc);

void DrawStatusBar(HDC hdc);

void DrawGradientRect(HDC hdc, RECT rect, COLORREF startColor, COLORREF middleColor, COLORREF endColor);

void UpdateConnectionTimer();

void UpdateTrafficStats();

void ToggleConnection();

HBITMAP CreateFlagBitmap();

HBITMAP CreateAppIcon();

void DrawVectorShield(Graphics* graphics, int centerX, int centerY, bool isActive);

void DrawBitmap(HDC hdc, HBITMAP hBitmap, int x, int y, int width, int height);

std::wstring FormatBytes(uint64_t bytes);

void MakeWindowRounded();

bool IsPointInRect(POINT pt, RECT rect);

HWND GetButtonAtPoint(int x, int y);

void UpdateButtonHoverState(int x, int y);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,

    _In_opt_ HINSTANCE hPrevInstance,

    _In_ LPWSTR lpCmdLine,

    _In_ int nCmdShow)

{

    UNREFERENCED_PARAMETER(hPrevInstance);

    UNREFERENCED_PARAMETER(lpCmdLine);

    GdiplusStartupInput gdiplusStartupInput;

    ULONG_PTR gdiplusToken;

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    INITCOMMONCONTROLSEX icc;

    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);

    icc.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;

    InitCommonControlsEx(&icc);



    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

    LoadStringW(hInstance, IDC_FREEDOMVPN, szWindowClass, MAX_LOADSTRING);

    MyRegisterClass(hInstance);



    if (!InitInstance(hInstance, nCmdShow))

    {

        return FALSE;

    }



    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FREEDOMVPN));



    MSG msg;



    while (GetMessage(&msg, nullptr, 0, 0))

    {

        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))

        {

            TranslateMessage(&msg);

            DispatchMessage(&msg);

        }

    }



    CleanupUI();

    GdiplusShutdown(gdiplusToken);

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

    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FREEDOMVPN));

    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);

    wcex.hbrBackground = CreateSolidBrush(COLOR_BG);

    wcex.lpszMenuName = NULL;

    wcex.lpszClassName = szWindowClass;

    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));



    return RegisterClassExW(&wcex);

}



BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)

{

    hInst = hInstance;



    g_hWnd = CreateWindowW(szWindowClass, szTitle,

        WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU | WS_MINIMIZEBOX,

        CW_USEDEFAULT, 0, APP_WIDTH, APP_HEIGHT, nullptr, nullptr, hInstance, nullptr);



    if (!g_hWnd)

    {

        return FALSE;

    }



    MakeWindowRounded();



    g_hBackgroundBrush = CreateSolidBrush(COLOR_BG);



    g_hTitleFont = CreateCustomFont(L"Segoe UI", 16);

    g_hButtonFont = CreateCustomFont(L"Segoe UI", 22);

    g_hStatusFont = CreateCustomFont(L"Segoe UI", 16);

    g_hSmallFont = CreateCustomFont(L"Segoe UI", 12);

    g_hMonoFont = CreateCustomFont(L"Consolas", 11);



    g_hGermanyFlag = CreateFlagBitmap();

    g_hAppIcon = CreateAppIcon();



    InitializeUI();



    const char* defaultConfig =

        "[Interface]\n"

        "PrivateKey = 8H/QQV752yFaUcbFbwopabG1arkG3Thh/VmJe3DQiV0=\n"

        "Address = 10.7.0.2/24, fddd:2c4:2c4:2c4::2/64\n"

        "DNS = 1.1.1.1, 1.0.0.1\n"

        "MTU = 1420\n"

        "\n"

        "[Peer]\n"

        "PublicKey = lvCMCOhewd4s3LA0jLk6iHSlsEBJzm6VYyU29qYUfw4=\n"

        "PresharedKey = kmCrmIk2iiX2FVwcPvUqvIVqXhnJFXFPH3VHRnVShJs=\n"

        "Endpoint = 72.61.154.191:51820\n"

        "AllowedIPs = 0.0.0.0/0, ::/0\n"

        "PersistentKeepalive = 25\n";



    wireguard.LoadConfigFromString(defaultConfig);

    UpdateStatus();



    SetTimer(g_hWnd, ID_ANIMATION_TIMER, 16, NULL);

    SetTimer(g_hWnd, ID_RECONNECT_TIMER, 5000, NULL);



    ShowWindow(g_hWnd, nCmdShow);

    UpdateWindow(g_hWnd);



    return TRUE;

}



void MakeWindowRounded()

{

    typedef enum _DWM_WINDOW_CORNER_PREFERENCE {

        DWMWCP_DEFAULT = 0,

        DWMWCP_DONOTROUND = 1,

        DWMWCP_ROUND = 2,

        DWMWCP_ROUNDSMALL = 3

    } DWM_WINDOW_CORNER_PREFERENCE;



    DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;

    DwmSetWindowAttribute(g_hWnd, 33, &preference, sizeof(preference));

}



void InitializeUI() {

    g_hLocationButton = CreateWindowW(L"BUTTON", L"Frankfurt 🇩🇪",

        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,

        50, 80, 250, 35, g_hWnd, (HMENU)IDC_LOCATION_BUTTON, hInst, NULL);



    g_hConnectButton = CreateWindowW(L"BUTTON", L"Connect",

        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,

        (APP_WIDTH - 180) / 2, 150, 180, 180, g_hWnd, (HMENU)IDC_CONNECT_BUTTON, hInst, NULL);



    g_hStatusBar = CreateWindowW(L"STATIC", L"",

        WS_VISIBLE | WS_CHILD | SS_OWNERDRAW,

        20, 360, APP_WIDTH - 40, 140, g_hWnd, NULL, hInst, NULL);



    g_hIPText = CreateWindowW(L"STATIC", L"IP      ---.---.---.---",

        WS_VISIBLE | WS_CHILD | SS_LEFT,

        40, 380, 270, 18, g_hWnd, (HMENU)IDC_IP_TEXT, hInst, NULL);

    SendMessage(g_hIPText, WM_SETFONT, (WPARAM)g_hMonoFont, TRUE);



    g_hProtocolText = CreateWindowW(L"STATIC", L"Protocol WireGuard",

        WS_VISIBLE | WS_CHILD | SS_LEFT,

        40, 405, 270, 18, g_hWnd, (HMENU)IDC_PROTOCOL_TEXT, hInst, NULL);

    SendMessage(g_hProtocolText, WM_SETFONT, (WPARAM)g_hMonoFont, TRUE);



    g_hTimerText = CreateWindowW(L"STATIC", L"Time     00:00:00",

        WS_VISIBLE | WS_CHILD | SS_LEFT,

        40, 430, 270, 18, g_hWnd, (HMENU)IDC_TIMER_TEXT, hInst, NULL);

    SendMessage(g_hTimerText, WM_SETFONT, (WPARAM)g_hMonoFont, TRUE);



    g_hUsageText = CreateWindowW(L"STATIC", L"Traffic  0 B ↓ | 0 B ↑",

        WS_VISIBLE | WS_CHILD | SS_LEFT,

        40, 455, 270, 18, g_hWnd, (HMENU)IDC_USAGE_TEXT, hInst, NULL);

    SendMessage(g_hUsageText, WM_SETFONT, (WPARAM)g_hMonoFont, TRUE);



    g_hReconnectButton = CreateWindowW(L"BUTTON", L"↻",

        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,

        APP_WIDTH - 50, 510, 30, 30, g_hWnd, (HMENU)IDC_RECONNECT_BUTTON, hInst, NULL);



    ShowWindow(g_hStatusBar, SW_HIDE);

    ShowWindow(g_hReconnectButton, SW_HIDE);

    ShowWindow(g_hProtocolText, SW_HIDE);

    ShowWindow(g_hTimerText, SW_HIDE);

    ShowWindow(g_hIPText, SW_HIDE);

    ShowWindow(g_hUsageText, SW_HIDE);

}



void CleanupUI() {

    DeleteObject(g_hBackgroundBrush);



    DeleteObject(g_hTitleFont);

    DeleteObject(g_hButtonFont);

    DeleteObject(g_hStatusFont);

    DeleteObject(g_hSmallFont);

    DeleteObject(g_hMonoFont);



    DeleteObject(g_hGermanyFlag);

    DeleteObject(g_hAppIcon);



    KillTimer(g_hWnd, ID_CONNECTION_TIMER);

    KillTimer(g_hWnd, ID_TRAFFIC_TIMER);

    KillTimer(g_hWnd, ID_ANIMATION_TIMER);

    KillTimer(g_hWnd, ID_RECONNECT_TIMER);

}



HFONT CreateCustomFont(const wchar_t* fontName, int size, bool bold) {

    return CreateFontW(

        -MulDiv(size, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72),

        0, 0, 0,

        bold ? FW_BOLD : FW_NORMAL,

        FALSE, FALSE, FALSE,

        DEFAULT_CHARSET,

        OUT_TT_PRECIS,

        CLIP_DEFAULT_PRECIS,

        CLEARTYPE_QUALITY,

        DEFAULT_PITCH | FF_DONTCARE,

        fontName

    );

}



void DrawRoundedRectangle(HDC hdc, RECT rect, int radius, COLORREF color) {

    HBRUSH brush = CreateSolidBrush(color);

    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

    HPEN pen = CreatePen(PS_SOLID, 1, color);

    HPEN oldPen = (HPEN)SelectObject(hdc, pen);



    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);



    SelectObject(hdc, oldBrush);

    SelectObject(hdc, oldPen);

    DeleteObject(brush);

    DeleteObject(pen);

}



void DrawGradientRect(HDC hdc, RECT rect, COLORREF startColor, COLORREF middleColor, COLORREF endColor) {

    int width = rect.right - rect.left;

    int height = rect.bottom - rect.top;



    for (int y = 0; y < height; y++) {

        float ratio = (float)y / height;

        int r, g, b;



        if (ratio < 0.5f) {

            float localRatio = ratio * 2.0f;

            r = GetRValue(startColor) + (int)((GetRValue(middleColor) - GetRValue(startColor)) * localRatio);

            g = GetGValue(startColor) + (int)((GetGValue(middleColor) - GetGValue(startColor)) * localRatio);

            b = GetBValue(startColor) + (int)((GetBValue(middleColor) - GetBValue(startColor)) * localRatio);

        }

        else {

            float localRatio = (ratio - 0.5f) * 2.0f;

            r = GetRValue(middleColor) + (int)((GetRValue(endColor) - GetRValue(middleColor)) * localRatio);

            g = GetGValue(middleColor) + (int)((GetGValue(endColor) - GetGValue(middleColor)) * localRatio);

            b = GetBValue(middleColor) + (int)((GetBValue(endColor) - GetBValue(middleColor)) * localRatio);

        }



        HPEN pen = CreatePen(PS_SOLID, 1, RGB(r, g, b));

        HPEN oldPen = (HPEN)SelectObject(hdc, pen);



        MoveToEx(hdc, rect.left, rect.top + y, NULL);

        LineTo(hdc, rect.right, rect.top + y);



        SelectObject(hdc, oldPen);

        DeleteObject(pen);

    }

}



void DrawTitleBar(HDC hdc) {

    RECT clientRect;

    GetClientRect(g_hWnd, &clientRect);



    RECT titleBarRect = { 0, 0, clientRect.right, TITLEBAR_HEIGHT };



    HBRUSH glassБrush = CreateSolidBrush(COLOR_HEADER);

    FillRect(hdc, &titleBarRect, glassБrush);

    DeleteObject(glassБrush);



    DrawBitmap(hdc, g_hAppIcon, 15, 14, 18, 18);



    SetBkMode(hdc, TRANSPARENT);

    SetTextColor(hdc, COLOR_TEXT_PRIMARY);

    HFONT oldFont = (HFONT)SelectObject(hdc, g_hTitleFont);



    RECT textRect = { 40, 0, clientRect.right - 80, TITLEBAR_HEIGHT };

    DrawTextW(hdc, L"FreedomVPN", -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



    SelectObject(hdc, oldFont);



    RECT minimizeRect = { APP_WIDTH - 70, 8, APP_WIDTH - 40, 37 };

    RECT closeRect = { APP_WIDTH - 35, 8, APP_WIDTH - 5, 37 };



    if (isMinimizeHover) {

        HBRUSH hoverBrush = CreateSolidBrush(RGB(40, 50, 65));

        FillRect(hdc, &minimizeRect, hoverBrush);

        DeleteObject(hoverBrush);

    }



    if (isCloseHover) {

        HBRUSH hoverBrush = CreateSolidBrush(COLOR_ERROR);

        FillRect(hdc, &closeRect, hoverBrush);

        DeleteObject(hoverBrush);

    }



    HFONT iconFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,

        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,

        DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");

    oldFont = (HFONT)SelectObject(hdc, iconFont);



    SetTextColor(hdc, COLOR_TEXT_PRIMARY);

    DrawTextW(hdc, L"\uE921", -1, &minimizeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DrawTextW(hdc, L"\uE106", -1, &closeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



    SelectObject(hdc, oldFont);

    DeleteObject(iconFont);

}



void DrawVectorShield(Graphics* graphics, int centerX, int centerY, bool isActive) {

    int shieldSize = 48;

    int halfSize = shieldSize / 2;



    PointF shieldPoints[6] = {

        PointF((REAL)centerX, (REAL)(centerY - halfSize + 4)),

        PointF((REAL)(centerX + halfSize - 4), (REAL)(centerY - halfSize / 2 + 6)),

        PointF((REAL)(centerX + halfSize - 4), (REAL)(centerY + halfSize / 2 - 6)),

        PointF((REAL)centerX, (REAL)(centerY + halfSize - 4)),

        PointF((REAL)(centerX - halfSize + 4), (REAL)(centerY + halfSize / 2 - 6)),

        PointF((REAL)(centerX - halfSize + 4), (REAL)(centerY - halfSize / 2 + 6))

    };



    GraphicsPath shieldPath;

    shieldPath.AddPolygon(shieldPoints, 6);



    PathGradientBrush shieldBrush(&shieldPath);

    Color centerColor(255, 198, 207, 218);

    Color edgeColor(255, 170, 181, 194);

    shieldBrush.SetCenterColor(centerColor);

    int colorCount = 1;

    shieldBrush.SetSurroundColors(&edgeColor, &colorCount);



    graphics->FillPath(&shieldBrush, &shieldPath);



    if (isActive) {

        Pen outlinePen(Color(255, 0, 255, 183), 2.0f);

        graphics->DrawPath(&outlinePen, &shieldPath);

    }
    else {

        Pen outlinePen(Color(120, 198, 207, 218), 1.5f);

        graphics->DrawPath(&outlinePen, &shieldPath);

    }

}



void DrawConnectButton(HDC hdc, RECT* pRect) {

    Graphics graphics(hdc);

    graphics.SetSmoothingMode(SmoothingModeHighQuality);

    graphics.SetCompositingMode(CompositingModeSourceOver);

    graphics.SetCompositingQuality(CompositingQualityHighQuality);



    int centerX = (pRect->left + pRect->right) / 2;

    int centerY = (pRect->top + pRect->bottom) / 2;

    int baseRadius = 75;



    COLORREF mainColor, glowColor;

    bool showGlow = false;



    if (isConnected) {

        mainColor = COLOR_SUCCESS;

        glowColor = COLOR_NEON_TEAL;

        showGlow = true;

    }

    else if (isConnecting) {

        int pulse = (int)(15 * sin(animationPhase * 3.14159 * 2));

        mainColor = RGB(0 + pulse, 207 + pulse, 255);

        glowColor = COLOR_NEON_CYAN;

        showGlow = true;

    }

    else {

        mainColor = isConnectHover ? COLOR_DISCONNECTED_HOVER : RGB(60, 70, 88);

        glowColor = isConnectHover ? COLOR_NEON_CYAN : RGB(40, 50, 65);

        showGlow = isConnectHover;

    }



    if (showGlow) {

        int pulseOffset = (int)(8 * sin(animationPhase * 3.14159 * 2));



        for (int i = 6; i >= 0; i--) {

            int alpha = 45 - (i * 6);

            if (alpha < 8) alpha = 8;



            int glowRadius = baseRadius + 18 + (i * 5) + pulseOffset;

            RectF glowRect(

                (REAL)(centerX - glowRadius),

                (REAL)(centerY - glowRadius),

                (REAL)(glowRadius * 2),

                (REAL)(glowRadius * 2)

            );



            GraphicsPath glowPath;

            glowPath.AddEllipse(glowRect);



            PathGradientBrush glowBrush(&glowPath);

            Color glowCenter(alpha, GetRValue(glowColor), GetGValue(glowColor), GetBValue(glowColor));

            Color glowEdge(0, 0, 0, 0);

            glowBrush.SetCenterColor(glowCenter);

            int colorCount = 1;

            glowBrush.SetSurroundColors(&glowEdge, &colorCount);



            graphics.FillEllipse(&glowBrush, glowRect);

        }

    }



    RectF buttonRect(

        (REAL)(centerX - baseRadius),

        (REAL)(centerY - baseRadius),

        (REAL)(baseRadius * 2),

        (REAL)(baseRadius * 2)

    );



    GraphicsPath buttonPath;

    buttonPath.AddEllipse(buttonRect);



    PathGradientBrush buttonBrush(&buttonPath);

    Color outerColor(255, GetRValue(mainColor), GetGValue(mainColor), GetBValue(mainColor));

    Color innerColor(255, GetRValue(COLOR_GLASS), GetGValue(COLOR_GLASS), GetBValue(COLOR_GLASS));

    buttonBrush.SetCenterColor(innerColor);

    int colorCount = 1;

    buttonBrush.SetSurroundColors(&outerColor, &colorCount);



    graphics.FillEllipse(&buttonBrush, buttonRect);



    RectF innerShadowRect(

        (REAL)(centerX - baseRadius + 5),

        (REAL)(centerY - baseRadius + 5),

        (REAL)(baseRadius * 2 - 10),

        (REAL)(baseRadius * 2 - 10)

    );



    GraphicsPath innerShadowPath;

    innerShadowPath.AddEllipse(innerShadowRect);



    PathGradientBrush shadowBrush(&innerShadowPath);

    Color shadowCenter(0, 0, 0, 0);

    Color shadowEdge(50, 0, 0, 0);

    shadowBrush.SetCenterColor(shadowCenter);

    shadowBrush.SetSurroundColors(&shadowEdge, &colorCount);



    graphics.FillEllipse(&shadowBrush, innerShadowRect);



    if (isConnecting) {

        for (int i = 0; i < 4; i++) {

            float startAngle = rotationAngle + (i * 90.0f);

            float sweepAngle = 50.0f;



            int arcRadius = baseRadius + 8;

            RectF arcRect(

                (REAL)(centerX - arcRadius),

                (REAL)(centerY - arcRadius),

                (REAL)(arcRadius * 2),

                (REAL)(arcRadius * 2)

            );



            Pen arcPen(Color(255, 0, 255, 183), 4.0f);

            arcPen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);

            graphics.DrawArc(&arcPen, arcRect, startAngle, sweepAngle);

        }

    }



    DrawVectorShield(&graphics, centerX, centerY, isConnected || isConnecting);



    SetBkMode(hdc, TRANSPARENT);

    HFONT oldFont = (HFONT)SelectObject(hdc, g_hButtonFont);



    std::wstring buttonText;

    COLORREF textColor = COLOR_TEXT_PRIMARY;



    if (isConnecting) {

        buttonText = L"Connecting";

        textColor = COLOR_NEON_CYAN;

    }

    else if (isConnected) {

        buttonText = L"Disconnect";

        textColor = COLOR_SUCCESS;

    }

    else {

        buttonText = L"Connect";

        textColor = isConnectHover ? COLOR_NEON_CYAN : COLOR_TEXT_PRIMARY;

    }



    RECT shadowRect = { pRect->left, centerY + 91, pRect->right, pRect->bottom };

    SetTextColor(hdc, RGB(0, 0, 0));

    DrawTextW(hdc, buttonText.c_str(), -1, &shadowRect, DT_CENTER);



    RECT textRect = { pRect->left, centerY + 90, pRect->right, pRect->bottom };

    SetTextColor(hdc, textColor);



    SelectObject(hdc, oldFont);

}



void DrawStatusBar(HDC hdc) {

    RECT rect;

    GetWindowRect(g_hStatusBar, &rect);

    POINT topLeft = { rect.left, rect.top };

    POINT bottomRight = { rect.right, rect.bottom };

    ScreenToClient(g_hWnd, &topLeft);

    ScreenToClient(g_hWnd, &bottomRight);



    rect.left = topLeft.x;

    rect.top = topLeft.y;

    rect.right = bottomRight.x;

    rect.bottom = bottomRight.y;



    if (isConnected) {

        HBRUSH glassBrush = CreateSolidBrush(COLOR_GLASS);

        HPEN glassPen = CreatePen(PS_SOLID, 1, COLOR_GLASS_LIGHT);

        SelectObject(hdc, glassBrush);

        SelectObject(hdc, glassPen);



        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 12, 12);



        DeleteObject(glassBrush);

        DeleteObject(glassPen);

    }

}



HBITMAP CreateFlagBitmap() {

    HDC hdcScreen = GetDC(NULL);

    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 30, 30);

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);



    RECT topRect = { 0, 0, 30, 10 };

    RECT middleRect = { 0, 10, 30, 20 };

    RECT bottomRect = { 0, 20, 30, 30 };



    HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));

    HBRUSH hRedBrush = CreateSolidBrush(RGB(221, 0, 0));

    HBRUSH hGoldBrush = CreateSolidBrush(RGB(255, 206, 0));



    FillRect(hdcMem, &topRect, hBlackBrush);

    FillRect(hdcMem, &middleRect, hRedBrush);

    FillRect(hdcMem, &bottomRect, hGoldBrush);



    SelectObject(hdcMem, hOldBitmap);

    DeleteDC(hdcMem);

    ReleaseDC(NULL, hdcScreen);

    DeleteObject(hBlackBrush);

    DeleteObject(hRedBrush);

    DeleteObject(hGoldBrush);



    return hBitmap;

}
HBITMAP CreateAppIcon() {

    HDC hdcScreen = GetDC(NULL);

    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 18, 18);

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);



    RECT bmpRect = { 0, 0, 18, 18 };

    HBRUSH hHeaderBrush = CreateSolidBrush(COLOR_HEADER);
    FillRect(hdcMem, &bmpRect, hHeaderBrush);
    DeleteObject(hHeaderBrush);



    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 207, 255));

    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);

    HBRUSH hBrush = CreateSolidBrush(RGB(0, 207, 255));

    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);



    POINT shield[7] = {

        {9, 2}, {16, 5}, {16, 11}, {9, 16}, {2, 11}, {2, 5}, {9, 2}

    };

    Polyline(hdcMem, shield, 7);



    HRGN hRgn = CreatePolygonRgn(shield, 6, ALTERNATE);

    FillRgn(hdcMem, hRgn, hBrush);



    SetTextColor(hdcMem, RGB(10, 15, 22));

    SetBkMode(hdcMem, TRANSPARENT);

    HFONT hFont = CreateFontW(11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,

        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,

        DEFAULT_PITCH | FF_DONTCARE, L"Arial");

    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);



    TextOutW(hdcMem, 6, 5, L"F", 1);



    SelectObject(hdcMem, hOldPen);

    SelectObject(hdcMem, hOldBrush);

    SelectObject(hdcMem, hOldFont);

    SelectObject(hdcMem, hOldBitmap);



    DeleteObject(hPen);

    DeleteObject(hBrush);

    DeleteObject(hFont);

    DeleteObject(hRgn);

    DeleteDC(hdcMem);

    ReleaseDC(NULL, hdcScreen);



    return hBitmap;

}



void DrawBitmap(HDC hdc, HBITMAP hBitmap, int x, int y, int width, int height) {

    HDC hdcMem = CreateCompatibleDC(hdc);

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);



    BITMAP bm;

    GetObject(hBitmap, sizeof(BITMAP), &bm);



    SetStretchBltMode(hdc, HALFTONE);

    StretchBlt(hdc, x, y, width, height, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);



    SelectObject(hdcMem, hOldBitmap);

    DeleteDC(hdcMem);

}



void UpdateStatus() {

    bool wasConnected = isConnected;

    isConnected = wireguard.IsConnected();



    if (wasConnected != isConnected) {

        if (isConnected) {

            connectionStartTime = std::chrono::steady_clock::now();

            SetTimer(g_hWnd, ID_CONNECTION_TIMER, 1000, NULL);

            SetTimer(g_hWnd, ID_TRAFFIC_TIMER, 2000, NULL);



            ShowWindow(g_hStatusBar, SW_SHOW);

            ShowWindow(g_hReconnectButton, SW_SHOW);

            ShowWindow(g_hProtocolText, SW_SHOW);

            ShowWindow(g_hTimerText, SW_SHOW);

            ShowWindow(g_hIPText, SW_SHOW);

            ShowWindow(g_hUsageText, SW_SHOW);



            SetWindowTextW(g_hIPText, L"IP       72.61.154.191");



            downloadedBytes = 0;

            uploadedBytes = 0;

            UpdateTrafficStats();

        }

        else {

            KillTimer(g_hWnd, ID_CONNECTION_TIMER);

            KillTimer(g_hWnd, ID_TRAFFIC_TIMER);



            ShowWindow(g_hStatusBar, SW_HIDE);

            ShowWindow(g_hReconnectButton, SW_HIDE);

            ShowWindow(g_hProtocolText, SW_HIDE);

            ShowWindow(g_hTimerText, SW_HIDE);

            ShowWindow(g_hIPText, SW_HIDE);

            ShowWindow(g_hUsageText, SW_HIDE);

        }



        InvalidateRect(g_hWnd, NULL, TRUE);

    }



    if (isConnected) {

        UpdateConnectionTimer();

    }

}



void UpdateConnectionTimer() {

    if (!isConnected) return;



    auto now = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - connectionStartTime);



    int hours = static_cast<int>(elapsed.count() / 3600);

    int minutes = static_cast<int>((elapsed.count() % 3600) / 60);

    int seconds = static_cast<int>(elapsed.count() % 60);



    wchar_t timeText[50];

    swprintf_s(timeText, L"Time     %02d:%02d:%02d", hours, minutes, seconds);



    SetWindowTextW(g_hTimerText, timeText);

}



void UpdateTrafficStats() {

    if (!isConnected) return;



    downloadedBytes += (rand() % 100000) + 50000;

    uploadedBytes += (rand() % 50000) + 5000;



    std::wstring downStr = FormatBytes(downloadedBytes);

    std::wstring upStr = FormatBytes(uploadedBytes);



    wchar_t usageText[100];

    swprintf_s(usageText, L"Traffic  %ls ↓ | %ls ↑", downStr.c_str(), upStr.c_str());



    SetWindowTextW(g_hUsageText, usageText);

}



std::wstring FormatBytes(uint64_t bytes) {

    const wchar_t* suffixes[] = { L"B", L"KB", L"MB", L"GB", L"TB" };

    int suffixIndex = 0;

    double size = static_cast<double>(bytes);



    while (size >= 1024 && suffixIndex < 4) {

        size /= 1024;

        suffixIndex++;

    }



    wchar_t buffer[30];

    if (suffixIndex == 0) {

        swprintf_s(buffer, L"%llu %ls", static_cast<unsigned long long>(size), suffixes[suffixIndex]);

    }

    else {

        swprintf_s(buffer, L"%.2f %ls", size, suffixes[suffixIndex]);

    }



    return std::wstring(buffer);

}



void ToggleConnection() {

    if (isConnecting) {

        return;

    }



    if (isConnected) {

        if (wireguard.Disconnect()) {

            MessageBoxW(g_hWnd, L"Disconnected successfully", L"Success", MB_OK | MB_ICONINFORMATION);

            reconnectAttempts = 0;

        }

        else {

            MessageBoxW(g_hWnd, L"Disconnection error", L"Error", MB_OK | MB_ICONERROR);

        }

    }

    else {

        isConnecting = true;

        InvalidateRect(g_hConnectButton, NULL, TRUE);



        if (wireguard.Connect()) {

            MessageBoxW(g_hWnd, L"Connected successfully", L"Success", MB_OK | MB_ICONINFORMATION);

            reconnectAttempts = 0;

        }

        else {

            MessageBoxW(g_hWnd,

                L"Connection error. Check settings and ensure WireGuard is installed.",

                L"Error", MB_OK | MB_ICONERROR);

        }



        isConnecting = false;

    }



    UpdateStatus();

}



bool IsPointInRect(POINT pt, RECT rect) {

    return (pt.x >= rect.left && pt.x <= rect.right &&

        pt.y >= rect.top && pt.y <= rect.bottom);

}



HWND GetButtonAtPoint(int x, int y) {

    POINT pt = { x, y };



    HWND buttons[] = {

        g_hConnectButton,

        g_hLocationButton,

        g_hReconnectButton

    };



    for (HWND button : buttons) {

        if (!IsWindow(button) || !IsWindowVisible(button))

            continue;



        RECT rect;

        GetWindowRect(button, &rect);

        ScreenToClient(g_hWnd, (POINT*)&rect);

        ScreenToClient(g_hWnd, (POINT*)&rect.right);



        if (IsPointInRect(pt, rect)) {

            return button;

        }

    }



    return NULL;

}



void UpdateButtonHoverState(int x, int y) {

    RECT minimizeRect = { APP_WIDTH - 70, 8, APP_WIDTH - 40, 37 };

    RECT closeRect = { APP_WIDTH - 35, 8, APP_WIDTH - 5, 37 };

    POINT pt = { x, y };



    bool wasMinimizeHover = isMinimizeHover;

    bool wasCloseHover = isCloseHover;



    isMinimizeHover = IsPointInRect(pt, minimizeRect);

    isCloseHover = IsPointInRect(pt, closeRect);



    if (wasMinimizeHover != isMinimizeHover || wasCloseHover != isCloseHover) {

        RECT titleBarRect = { 0, 0, APP_WIDTH, TITLEBAR_HEIGHT };

        InvalidateRect(g_hWnd, &titleBarRect, FALSE);

    }



    HWND newHoverButton = GetButtonAtPoint(x, y);



    if (newHoverButton != hoverButton) {

        if (hoverButton) {

            InvalidateRect(hoverButton, NULL, TRUE);

        }



        hoverButton = newHoverButton;



        if (hoverButton) {

            InvalidateRect(hoverButton, NULL, TRUE);

        }



        isConnectHover = (hoverButton == g_hConnectButton);

        isLocationHover = (hoverButton == g_hLocationButton);

        isReconnectHover = (hoverButton == g_hReconnectButton);

    }

}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)

{

    static bool isDragging = false;

    static POINT dragStart;



    switch (message)

    {

    case WM_TIMER:

        if (wParam == ID_CONNECTION_TIMER) {

            UpdateConnectionTimer();

        }

        else if (wParam == ID_TRAFFIC_TIMER) {

            UpdateTrafficStats();

        }

        else if (wParam == ID_ANIMATION_TIMER) {

            animationPhase += 0.05f;

            if (animationPhase >= 1.0f) animationPhase = 0.0f;



            if (isConnecting) {

                rotationAngle += 2.5f;

                if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;

            }



            InvalidateRect(g_hConnectButton, NULL, FALSE);

        }

        else if (wParam == ID_RECONNECT_TIMER) {

            if (isConnected && !wireguard.IsConnected() && autoReconnect) {

                reconnectAttempts++;

                if (reconnectAttempts <= 5) {

                    isConnecting = true;

                    if (wireguard.Connect()) {

                        reconnectAttempts = 0;

                        isConnecting = false;

                    }

                    UpdateStatus();

                }

            }

        }

        break;



    case WM_MOUSEMOVE:

    {

        int x = GET_X_LPARAM(lParam);

        int y = GET_Y_LPARAM(lParam);



        UpdateButtonHoverState(x, y);



        if (isDragging) {

            POINT currentPos = { x, y };

            ClientToScreen(hWnd, &currentPos);



            int deltaX = currentPos.x - dragStart.x;

            int deltaY = currentPos.y - dragStart.y;



            RECT windowRect;

            GetWindowRect(hWnd, &windowRect);



            SetWindowPos(hWnd, NULL,

                windowRect.left + deltaX,

                windowRect.top + deltaY,

                0, 0, SWP_NOSIZE | SWP_NOZORDER);



            dragStart = currentPos;

        }

        else if (y <= TITLEBAR_HEIGHT && x > 40 && x < APP_WIDTH - 80) {

            SetCursor(LoadCursor(NULL, IDC_SIZEALL));

        }

        else {

            SetCursor(LoadCursor(NULL, IDC_ARROW));

        }

    }

    break;



    case WM_LBUTTONDOWN:

    {

        int x = GET_X_LPARAM(lParam);

        int y = GET_Y_LPARAM(lParam);



        RECT minimizeRect = { APP_WIDTH - 70, 8, APP_WIDTH - 40, 37 };

        RECT closeRect = { APP_WIDTH - 35, 8, APP_WIDTH - 5, 37 };

        POINT pt = { x, y };



        if (IsPointInRect(pt, closeRect)) {

            DestroyWindow(hWnd);

            return 0;

        }



        if (IsPointInRect(pt, minimizeRect)) {

            ShowWindow(hWnd, SW_MINIMIZE);

            return 0;

        }



        if (y <= TITLEBAR_HEIGHT && x > 40 && x < APP_WIDTH - 80) {

            isDragging = true;

            SetCapture(hWnd);



            POINT pt = { x, y };

            ClientToScreen(hWnd, &pt);

            dragStart = pt;

        }

    }

    break;



    case WM_LBUTTONUP:

    {

        if (isDragging) {

            isDragging = false;

            ReleaseCapture();

        }

    }

    break;



    case WM_DRAWITEM:

    {

        DRAWITEMSTRUCT* pDIS = (DRAWITEMSTRUCT*)lParam;



        if (pDIS->CtlID == IDC_CONNECT_BUTTON) {

            DrawConnectButton(pDIS->hDC, &pDIS->rcItem);

            return TRUE;

        }

        else if (pDIS->hwndItem == g_hStatusBar) {

            DrawStatusBar(pDIS->hDC);

            return TRUE;

        }

        else if (pDIS->CtlID == IDC_LOCATION_BUTTON) {

            RECT rect = pDIS->rcItem;

            COLORREF bgColor = isLocationHover ? COLOR_GLASS_LIGHT : COLOR_GLASS;



            HBRUSH brush = CreateSolidBrush(bgColor);

            HPEN pen = CreatePen(PS_SOLID, 1, isLocationHover ? COLOR_NEON_CYAN : COLOR_GLASS_LIGHT);

            SelectObject(pDIS->hDC, brush);

            SelectObject(pDIS->hDC, pen);



            RoundRect(pDIS->hDC, rect.left, rect.top, rect.right, rect.bottom, 10, 10);



            DeleteObject(brush);

            DeleteObject(pen);



            SetBkMode(pDIS->hDC, TRANSPARENT);

            SetTextColor(pDIS->hDC, COLOR_TEXT_SECONDARY);

            HFONT oldFont = (HFONT)SelectObject(pDIS->hDC, g_hSmallFont);

            DrawTextW(pDIS->hDC, L"Frankfurt 🇩🇪", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(pDIS->hDC, oldFont);

            return TRUE;

        }

        else if (pDIS->CtlID == IDC_RECONNECT_BUTTON) {

            RECT rect = pDIS->rcItem;

            COLORREF bgColor = isReconnectHover ? COLOR_GLASS_LIGHT : COLOR_GLASS;



            HBRUSH brush = CreateSolidBrush(bgColor);

            HPEN pen = CreatePen(PS_SOLID, 1, COLOR_GLASS_LIGHT);

            SelectObject(pDIS->hDC, brush);

            SelectObject(pDIS->hDC, pen);



            Ellipse(pDIS->hDC, rect.left, rect.top, rect.right, rect.bottom);



            DeleteObject(brush);

            DeleteObject(pen);



            SetBkMode(pDIS->hDC, TRANSPARENT);

            SetTextColor(pDIS->hDC, COLOR_TEXT_SECONDARY);

            HFONT oldFont = (HFONT)SelectObject(pDIS->hDC, g_hStatusFont);

            DrawTextW(pDIS->hDC, L"↻", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(pDIS->hDC, oldFont);

            return TRUE;

        }

    }

    break;



    case WM_CTLCOLORSTATIC:

    {

        HDC hdcStatic = (HDC)wParam;



        SetTextColor(hdcStatic, COLOR_TEXT_SECONDARY);

        SetBkColor(hdcStatic, COLOR_BG);

        return (LRESULT)g_hBackgroundBrush;

    }



    case WM_CTLCOLORBTN:

    {

        HDC hdcBtn = (HDC)wParam;



        SetBkMode(hdcBtn, TRANSPARENT);

        return (LRESULT)GetStockObject(NULL_BRUSH);

    }



    case WM_COMMAND:

    {

        int wmId = LOWORD(wParam);



        switch (wmId)

        {

        case IDC_CONNECT_BUTTON:

            ToggleConnection();

            break;



        case IDC_RECONNECT_BUTTON:

            if (isConnected) {

                if (wireguard.Disconnect()) {

                    if (wireguard.Connect()) {

                        MessageBoxW(g_hWnd, L"Reconnected successfully", L"Success", MB_OK | MB_ICONINFORMATION);

                    }

                }

                UpdateStatus();

            }

            break;



        case IDC_LOCATION_BUTTON:

            MessageBoxW(hWnd, L"Current location: Frankfurt, Germany\n\nMore locations coming soon!", L"Location", MB_OK | MB_ICONINFORMATION);

            break;



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



        RECT clientRect;

        GetClientRect(hWnd, &clientRect);



        DrawGradientRect(hdc, clientRect, COLOR_BG_TOP, COLOR_BG, COLOR_BG_BOTTOM);



        DrawTitleBar(hdc);



        EndPaint(hWnd, &ps);

        return 0;

    }

    break;



    case WM_DESTROY:

        if (wireguard.IsConnected()) {

            wireguard.Disconnect();

        }

        PostQuitMessage(0);

        break;



    default:

        return DefWindowProc(hWnd, message, wParam, lParam);

    }

    return 0;

}



INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

    UNREFERENCED_PARAMETER(lParam);

    switch (message)

    {

    case WM_INITDIALOG:

    {

        HFONT hFont = CreateCustomFont(L"Segoe UI", 12);

        EnumChildWindows(hDlg,

            [](HWND hwnd, LPARAM lParam) -> BOOL {

                SendMessage(hwnd, WM_SETFONT, lParam, TRUE);

                return TRUE;

            },

            (LPARAM)hFont);

        DeleteObject(hFont);

    }

    return (INT_PTR)TRUE;



    case WM_CTLCOLORDLG:

    case WM_CTLCOLORSTATIC:

    {

        HDC hdcStatic = (HDC)wParam;

        SetTextColor(hdcStatic, COLOR_TEXT_SECONDARY);

        SetBkColor(hdcStatic, COLOR_GLASS);

        return (INT_PTR)CreateSolidBrush(COLOR_GLASS);

    }



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