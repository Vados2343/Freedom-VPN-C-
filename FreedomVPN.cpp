#include "framework.h"
#include "FreedomVPN.h"
#include "WireGuardIntegration.h"
#include "CyrillicSupport.h"
#include <commctrl.h>
#include <commdlg.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <cmath>

// Определение идентификаторов элементов управления
#define MAX_LOADSTRING 100
#define IDC_CONNECT_BUTTON 1001
#define IDC_SETTINGS_BUTTON 1002
#define IDC_LOCATION_BUTTON 1003
#define IDC_STATUS_TEXT 1004
#define IDC_TIMER_TEXT 1005
#define IDC_PROTOCOL_TEXT 1006
#define IDC_RECONNECT_BUTTON 1007
#define IDC_CLOSE_BUTTON 1008
#define IDC_MINIMIZE_BUTTON 1009
#define IDC_IP_TEXT 1010
#define IDC_USAGE_TEXT 1011

#define COLOR_BACKGROUND_TOP RGB(10,14,20)
#define COLOR_BACKGROUND_BOTTOM RGB(20,26,38)
#define COLOR_BACKGROUND RGB(15,20,30)
#define COLOR_DARK_ELEMENT RGB(20,28,40)
#define COLOR_GLASS_ELEMENT RGB(25,35,50)
#define COLOR_BUTTON_TEXT RGB(234,234,234)
#define COLOR_NEON_CYAN RGB(0,175,255)
#define COLOR_NEON_TEAL RGB(0,255,183)
#define COLOR_CONNECTED_GREEN RGB(34,197,94)
#define COLOR_CONNECTED_GLOW RGB(20,150,70)
#define COLOR_ERROR_RED RGB(239,68,68)
#define COLOR_DISCONNECTED RGB(70,80,95)
#define COLOR_DISCONNECTED_HOVER RGB(85,95,110)
#define COLOR_HEADER RGB(12,18,28)
#define COLOR_HEADER_TEXT RGB(234,234,234)
#define COLOR_CONNECTION_TEXT RGB(156,163,175)
#define COLOR_BUTTON_HOVER RGB(30,40,55)
#define COLOR_CLOSE_HOVER RGB(239,68,68)
#define COLOR_MINIMIZE_HOVER RGB(70,80,95)
#define COLOR_ACCENT_BLUE RGB(0,175,255)
#define COLOR_ACCENT_TEAL RGB(0,255,183)
#define COLOR_SHADOW RGB(0,0,0)


// Определение констант для заголовка окна
#define TITLEBAR_HEIGHT 30
#define APP_WIDTH 350
#define APP_HEIGHT 550

// Идентификатор таймера
#define ID_CONNECTION_TIMER 1001
#define ID_TRAFFIC_TIMER 1002
#define ID_ANIMATION_TIMER 1003
#define ID_RECONNECT_TIMER 1004

// Глобальные переменные
HINSTANCE hInst;                   
WCHAR szTitle[MAX_LOADSTRING];       
WCHAR szWindowClass[MAX_LOADSTRING];   
WireGuardIntegration wireguard;             
std::chrono::steady_clock::time_point connectionStartTime;
bool isConnected = false;        
bool isTitleBarHover = false;                 
bool isConnectHover = false;                  
bool isSettingsHover = false;              
bool isLocationHover = false;            
HWND hoverButton = NULL;                  
uint64_t downloadedBytes = 0;             
uint64_t uploadedBytes = 0;                  
float animationPhase = 0.0f;
float rotationAngle = 0.0f;
bool isConnecting = false;
int reconnectAttempts = 0;
bool autoReconnect = true;
bool g_isDragging = false;
POINT g_dragOffset;

HWND g_hWnd;                
HWND g_hConnectButton;                    
HWND g_hSettingsButton;        
HWND g_hLocationButton;             
HWND g_hStatusBar;            
HWND g_hTimerText;                             
HWND g_hProtocolText;                
HWND g_hReconnectButton;              
HWND g_hCloseButton;                
HWND g_hMinimizeButton;                 
HWND g_hIPText;                        
HWND g_hUsageText;                          

HBRUSH g_hBackgroundBrush;
HBRUSH g_hDarkElementBrush;
HBRUSH g_hConnectedBrush;
HBRUSH g_hDisconnectedBrush;
HBRUSH g_hHeaderBrush;
HBRUSH g_hButtonHoverBrush;
HBRUSH g_hCloseHoverBrush;
HBRUSH g_hMinimizeHoverBrush;
HPEN g_hConnectedPen;
HPEN g_hDisconnectedPen;
HFONT g_hButtonFont;
HFONT g_hHeaderFont;
HFONT g_hStatusFont;
HFONT g_hSmallFont;
HFONT g_hTinyFont;
HBITMAP g_hGermanyFlag;
HBITMAP g_hShieldIcon;
HBITMAP g_hSettingsIcon;
HBITMAP g_hAppIcon;

// Объявления функций
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                UpdateStatus();
void                InitializeUI();
void                CleanupUI();
HFONT               CreateCustomFont(const wchar_t* fontName, int size, bool bold = false);
void                DrawRoundedRectangle(HDC hdc, RECT rect, int radius, COLORREF color);
void                DrawTitleBar(HDC hdc);
void                DrawConnectButton(HDC hdc);
void                DrawStatusBar(HDC hdc);
void                DrawGradientRect(HDC hdc, RECT rect, COLORREF startColor, COLORREF middleColor, COLORREF endColor);
void                UpdateConnectionTimer();
void                UpdateTrafficStats();
void                ToggleConnection();
HBITMAP             CreateFlagBitmap();
HBITMAP             CreateShieldBitmap();
HBITMAP             CreateSettingsBitmap();
HBITMAP             CreateAppIcon();
void                DrawBitmap(HDC hdc, HBITMAP hBitmap, int x, int y, int width, int height);
std::wstring        FormatBytes(uint64_t bytes);
void                MakeWindowRounded();
void                DrawCloseButton(HDC hdc);
void                DrawMinimizeButton(HDC hdc);
bool                IsPointInRect(POINT pt, RECT rect);
HWND                GetButtonAtPoint(int x, int y);
void                UpdateButtonHoverState(int x, int y);

// Точка входа в приложение
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация Common Controls для современного вида элементов управления
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FREEDOMVPN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FREEDOMVPN));

    MSG msg;

    // Цикл основного сообщения
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Очистка ресурсов UI перед выходом
    CleanupUI();

    return (int)msg.wParam;
}

// Регистрация класса окна
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
    wcex.hbrBackground = CreateSolidBrush(COLOR_BACKGROUND);
    wcex.lpszMenuName = NULL; // Удаляем меню для минималистичного дизайна
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// Создание и инициализация экземпляра главного окна приложения
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

    // Создаем окно без стандартной рамки для кастомного заголовка
    g_hWnd = CreateWindowW(szWindowClass, szTitle,
        WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, 0, APP_WIDTH, APP_HEIGHT, nullptr, nullptr, hInstance, nullptr);

    if (!g_hWnd)
    {
        return FALSE;
    }

    // Делаем углы окна закругленными
    MakeWindowRounded();

    // Создание кистей и шрифтов
    g_hBackgroundBrush = CreateSolidBrush(COLOR_BACKGROUND);
    g_hDarkElementBrush = CreateSolidBrush(COLOR_DARK_ELEMENT);
    g_hDisconnectedBrush = CreateSolidBrush(COLOR_DISCONNECTED);
    g_hHeaderBrush = CreateSolidBrush(COLOR_HEADER);
    g_hButtonHoverBrush = CreateSolidBrush(COLOR_BUTTON_HOVER);
    g_hCloseHoverBrush = CreateSolidBrush(COLOR_CLOSE_HOVER);
    g_hMinimizeHoverBrush = CreateSolidBrush(COLOR_MINIMIZE_HOVER);
    g_hDisconnectedPen = CreatePen(PS_SOLID, 1, COLOR_DISCONNECTED);

    g_hButtonFont = CreateCustomFont(L"Segoe UI", 22);
    g_hHeaderFont = CreateCustomFont(L"Segoe UI", 18, true);
    g_hStatusFont = CreateCustomFont(L"Segoe UI", 16);
    g_hSmallFont = CreateCustomFont(L"Segoe UI", 12);
    g_hTinyFont = CreateCustomFont(L"Segoe UI", 10);

    g_hGermanyFlag = CreateFlagBitmap();
    g_hShieldIcon = CreateShieldBitmap();
    g_hSettingsIcon = CreateSettingsBitmap();
    g_hAppIcon = CreateAppIcon();

    // Инициализация интерфейса
    InitializeUI();

    const char* defaultConfig =
        "[Interface]\n"
        "PrivateKey = 8H/QQV752yFaUcbFbwopabG1arkG3Thh/VmJe3DQiV0=\n"
        "Address = 10.7.0.2/24,fddd:2c4:2c4:2c4::2/64\n"
        "DNS = 1.1.1.1, 1.0.0.1\n"  // Cloudflare DNS - быстрее для игр
        "MTU = 1420\n"  // Оптимальный MTU для предотвращения фрагментации
        "\n"
        "[Peer]\n"
        "PublicKey = lvCMCOhewd4s3LA0jLk6iHSlsEBJzm6VYyU29qYUfw4=\n"
        "PresharedKey = kmCrmIk2iiX2FVwcPvUqvIVqXhnJFXFPH3VHRnVShJs=\n"
        "Endpoint = 72.61.154.191:51820\n"
        "AllowedIPs = 0.0.0.0/0, ::0/0\n"
        "PersistentKeepalive = 25\n";  

    wireguard.LoadConfigFromString(defaultConfig);
    UpdateStatus();
    SetTimer(g_hWnd, ID_ANIMATION_TIMER, 16, NULL);
    SetTimer(g_hWnd, ID_RECONNECT_TIMER, 5000, NULL);
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    return TRUE;
}

// Сделать окно с закругленными углами
void MakeWindowRounded()
{
    RECT rect;
    GetWindowRect(g_hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Корректируем регион с учетом толщины рамок и titlebar
    HRGN region = CreateRoundRectRgn(0, 0, width + 1, height + 1, 20, 20);
    SetWindowRgn(g_hWnd, region, TRUE);
    // region не удаляем здесь, так как после SetWindowRgn система сама освободит память
}

// Инициализация пользовательского интерфейса
void InitializeUI() {
    // Кнопки управления окном в заголовке
    g_hCloseButton = CreateWindowW(L"BUTTON", L"×",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
        APP_WIDTH - 30, 0, 30, 30, g_hWnd, (HMENU)IDC_CLOSE_BUTTON, hInst, NULL);
    
    g_hMinimizeButton = CreateWindowW(L"BUTTON", L"−",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
        APP_WIDTH - 60, 0, 30, 30, g_hWnd, (HMENU)IDC_MINIMIZE_BUTTON, hInst, NULL);
    
    // Устанавливаем шрифты для кнопок
    SendMessage(g_hCloseButton, WM_SETFONT, (WPARAM)g_hHeaderFont, TRUE);
    SendMessage(g_hMinimizeButton, WM_SETFONT, (WPARAM)g_hHeaderFont, TRUE);

    // Create location panel with flag and country

    g_hLocationButton = CreateWindowW(L"BUTTON", L"Germany",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,
        20, 70, 200, 40, g_hWnd, (HMENU)IDC_LOCATION_BUTTON, hInst, NULL);

    // Кнопка настроек
    g_hSettingsButton = CreateWindowW(L"BUTTON", L"",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,
        APP_WIDTH - 60, 70, 40, 40, g_hWnd, (HMENU)IDC_SETTINGS_BUTTON, hInst, NULL);

    // В InitializeUI() - исправленное создание кнопки:
    g_hConnectButton = CreateWindowW(L"BUTTON", L"Connect",
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,  // Только BS_OWNERDRAW без BS_PUSHBUTTON
        (APP_WIDTH - 170) / 2, 140, 170, 170, g_hWnd, (HMENU)IDC_CONNECT_BUTTON, hInst, NULL);

    // Нижняя панель статуса (скрыта при отключении)
    g_hStatusBar = CreateWindowW(L"STATIC", L"",
        WS_VISIBLE | WS_CHILD | SS_OWNERDRAW,
        20, 340, APP_WIDTH - 40, 50, g_hWnd, NULL, hInst, NULL);

    // 🌐 IP Address display
    g_hIPText = CreateWindowW(L"STATIC", L"🌐 IP: ---.---.---.---",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 400, 310, 20, g_hWnd, (HMENU)IDC_IP_TEXT, hInst, NULL);
    SendMessage(g_hIPText, WM_SETFONT, (WPARAM)g_hSmallFont, TRUE);

    // 🔒 Protocol info
    g_hProtocolText = CreateWindowW(L"STATIC", L"🔒 Protocol: WireGuard",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 420, 160, 20, g_hWnd, (HMENU)IDC_PROTOCOL_TEXT, hInst, NULL);
    SendMessage(g_hProtocolText, WM_SETFONT, (WPARAM)g_hSmallFont, TRUE);

    // ⏱️ Connection timer
    g_hTimerText = CreateWindowW(L"STATIC", L"⏱️ Time: 00:00:00",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 440, 310, 20, g_hWnd, (HMENU)IDC_TIMER_TEXT, hInst, NULL);
    SendMessage(g_hTimerText, WM_SETFONT, (WPARAM)g_hSmallFont, TRUE);
    // 📊 Traffic usage
    g_hUsageText = CreateWindowW(L"STATIC", L"📊 Traffic: 0 B ↓ | 0 B ↑",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 460, 310, 20, g_hWnd, (HMENU)IDC_USAGE_TEXT, hInst, NULL);
    SendMessage(g_hUsageText, WM_SETFONT, (WPARAM)g_hSmallFont, TRUE);

    // Кнопка переподключения
    g_hReconnectButton = CreateWindowW(L"BUTTON", L"Reconnect",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        APP_WIDTH - 120, 500, 100, 30, g_hWnd, (HMENU)IDC_RECONNECT_BUTTON, hInst, NULL);
    SendMessage(g_hReconnectButton, WM_SETFONT, (WPARAM)g_hSmallFont, TRUE);

    // По умолчанию скрываем информационные панели, которые появятся только при подключении
    ShowWindow(g_hStatusBar, SW_HIDE);
    ShowWindow(g_hReconnectButton, SW_HIDE);
    ShowWindow(g_hProtocolText, SW_HIDE);
    ShowWindow(g_hTimerText, SW_HIDE);
    ShowWindow(g_hIPText, SW_HIDE);
    ShowWindow(g_hUsageText, SW_HIDE);
}

// Очистка ресурсов UI
void CleanupUI() {
    // Освобождаем все кисти
    DeleteObject(g_hBackgroundBrush);
    DeleteObject(g_hDarkElementBrush);
    DeleteObject(g_hConnectedBrush);
    DeleteObject(g_hDisconnectedBrush);
    DeleteObject(g_hHeaderBrush);
    DeleteObject(g_hButtonHoverBrush);
    DeleteObject(g_hCloseHoverBrush);
    DeleteObject(g_hMinimizeHoverBrush);
    DeleteObject(g_hConnectedPen);
    DeleteObject(g_hDisconnectedPen);
    
    // Освобождаем шрифты
    DeleteObject(g_hButtonFont);
    DeleteObject(g_hHeaderFont);
    DeleteObject(g_hStatusFont);
    DeleteObject(g_hSmallFont);
    DeleteObject(g_hTinyFont);
    
    DeleteObject(g_hGermanyFlag);
    DeleteObject(g_hShieldIcon);
    DeleteObject(g_hSettingsIcon);
    DeleteObject(g_hAppIcon);

    // Убедимся, что таймеры остановлены
    KillTimer(g_hWnd, ID_CONNECTION_TIMER);
    KillTimer(g_hWnd, ID_TRAFFIC_TIMER);
    KillTimer(g_hWnd, ID_ANIMATION_TIMER);
    KillTimer(g_hWnd, ID_RECONNECT_TIMER);
}

// Создание пользовательского шрифта
HFONT CreateCustomFont(const wchar_t* fontName, int size, bool bold) {
    return CreateFontW(
        -MulDiv(size, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), // Размер шрифта
        0, 0, 0,                                                    // Ширина, наклон, ориентация
        bold ? FW_BOLD : FW_NORMAL,                                 // Жирность
        FALSE, FALSE, FALSE,                                        // Курсив, подчеркивание, перечеркивание
        DEFAULT_CHARSET,                                           // Набор символов
        OUT_TT_PRECIS,                                             // Точность вывода
        CLIP_DEFAULT_PRECIS,                                       // Точность отсечения
        CLEARTYPE_QUALITY,                                         // Качество
        DEFAULT_PITCH | FF_DONTCARE,                               // Шаг и семейство шрифта
        fontName                                                   // Имя шрифта
    );
}

// Отрисовка скругленного прямоугольника
void DrawRoundedRectangle(HDC hdc, RECT rect, int radius, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    // Рисуем скругленный прямоугольник
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

// Отрисовка градиентного прямоугольника без использования GradientFill
void DrawGradientRect(HDC hdc, RECT rect, COLORREF startColor, COLORREF middleColor, COLORREF endColor) {
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    // Создаем промежуточные цвета для более плавного градиента
    const int steps = 20;
    int stepWidth = width / steps;
    
    // Первая половина градиента (от startColor до middleColor)
    for (int i = 0; i < steps / 2; i++) {
        float ratio = (float)i / (steps / 2);
        int r = GetRValue(startColor) + (int)((GetRValue(middleColor) - GetRValue(startColor)) * ratio);
        int g = GetGValue(startColor) + (int)((GetGValue(middleColor) - GetGValue(startColor)) * ratio);
        int b = GetBValue(startColor) + (int)((GetBValue(middleColor) - GetBValue(startColor)) * ratio);
        
        COLORREF color = RGB(r, g, b);
        HBRUSH brush = CreateSolidBrush(color);
        
        RECT stepRect = {
            rect.left + i * stepWidth,
            rect.top,
            rect.left + (i + 1) * stepWidth,
            rect.bottom
        };
        
        FillRect(hdc, &stepRect, brush);
        DeleteObject(brush);
    }
    
    // Вторая половина градиента (от middleColor до endColor)
    for (int i = 0; i < steps / 2; i++) {
        float ratio = (float)i / (steps / 2);
        int r = GetRValue(middleColor) + (int)((GetRValue(endColor) - GetRValue(middleColor)) * ratio);
        int g = GetGValue(middleColor) + (int)((GetGValue(endColor) - GetGValue(middleColor)) * ratio);
        int b = GetBValue(middleColor) + (int)((GetBValue(endColor) - GetBValue(middleColor)) * ratio);
        
        COLORREF color = RGB(r, g, b);
        HBRUSH brush = CreateSolidBrush(color);
        
        RECT stepRect = {
            rect.left + (steps / 2 + i) * stepWidth,
            rect.top,
            rect.left + (steps / 2 + i + 1) * stepWidth,
            rect.bottom
        };
        
        FillRect(hdc, &stepRect, brush);
        DeleteObject(brush);
    }
    
    // Заполняем оставшуюся часть (если есть)
    if (rect.left + steps * stepWidth < rect.right) {
        RECT remainingRect = {
            rect.left + steps * stepWidth,
            rect.top,
            rect.right,
            rect.bottom
        };
        
        HBRUSH brush = CreateSolidBrush(endColor);
        FillRect(hdc, &remainingRect, brush);
        DeleteObject(brush);
    }
}

// Рисование заголовка окна
void DrawTitleBar(HDC hdc) {
    // Получаем размеры окна
    RECT clientRect;
    GetClientRect(g_hWnd, &clientRect);
    
    // Рисуем фон заголовка
    RECT titleBarRect = { 0, 0, clientRect.right, TITLEBAR_HEIGHT };
    FillRect(hdc, &titleBarRect, g_hHeaderBrush);
    
    // Рисуем иконку приложения
    DrawBitmap(hdc, g_hAppIcon, 8, 7, 16, 16);
    
    // Рисуем текст заголовка
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_HEADER_TEXT);
    HFONT oldFont = (HFONT)SelectObject(hdc, g_hSmallFont);
    
    RECT textRect = { 30, 0, clientRect.right - 70, TITLEBAR_HEIGHT };
    DrawTextW(hdc, L"Freedom VPN", -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, oldFont);
    
    // Рисуем кнопки управления окном (закрытие и сворачивание)
    DrawCloseButton(hdc);
    DrawMinimizeButton(hdc);
}
void DrawCloseButton(HDC hdc) {
    RECT rect;
    GetWindowRect(g_hCloseButton, &rect);
    POINT pt = { rect.left, rect.top };
    ScreenToClient(g_hWnd, &pt);
    rect.left = pt.x;
    rect.top = pt.y;
    pt.x = rect.right;
    pt.y = rect.bottom;
    ScreenToClient(g_hWnd, &pt);
    rect.right = pt.x;
    rect.bottom = pt.y;
    HBRUSH brush = (hoverButton == g_hCloseButton) ? g_hCloseHoverBrush : g_hHeaderBrush;
    FillRect(hdc, &rect, brush);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_BUTTON_TEXT);
    HFONT oldFont = (HFONT)SelectObject(hdc, g_hHeaderFont);
    DrawTextW(hdc, L"×", 1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
}
void DrawMinimizeButton(HDC hdc) {
    RECT rect;
    GetWindowRect(g_hMinimizeButton, &rect);
    POINT pt = { rect.left, rect.top };
    ScreenToClient(g_hWnd, &pt);
    rect.left = pt.x;
    rect.top = pt.y;
    pt.x = rect.right;
    pt.y = rect.bottom;
    ScreenToClient(g_hWnd, &pt);
    rect.right = pt.x;
    rect.bottom = pt.y;
    HBRUSH brush = (hoverButton == g_hMinimizeButton) ? g_hMinimizeHoverBrush : g_hHeaderBrush;
    FillRect(hdc, &rect, brush);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_BUTTON_TEXT);
    HFONT oldFont = (HFONT)SelectObject(hdc, g_hHeaderFont);
    DrawTextW(hdc, L"−", 1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE); 
    SelectObject(hdc, oldFont);
}


void DrawConnectButton(HDC hdc, RECT* pRect) {
    int centerX = (pRect->left + pRect->right) / 2;
    int centerY = (pRect->top + pRect->bottom) / 2;
    int baseRadius = min((pRect->right - pRect->left) / 2, (pRect->bottom - pRect->top) / 2) - 10;
    COLORREF mainColor, glowColor, borderColor;
    if (isConnected) {
        mainColor = COLOR_CONNECTED_GREEN;
        glowColor = COLOR_CONNECTED_GLOW;
        borderColor = COLOR_NEON_TEAL;
    }
    else if (isConnecting) {
        int pulse = (int)(15 * sin(animationPhase * 3.14159 * 2));
        mainColor = RGB(0 + pulse, 175 + pulse, 255);
        glowColor = COLOR_NEON_CYAN;
        borderColor = COLOR_ACCENT_TEAL;
    }
    else {
        mainColor = isConnectHover ? COLOR_DISCONNECTED_HOVER : COLOR_DISCONNECTED;
        glowColor = COLOR_BUTTON_HOVER;
        borderColor = isConnectHover ? COLOR_NEON_CYAN : COLOR_DISCONNECTED;
    }
    if (isConnected || isConnecting) {
        int pulseOffset = (int)(8 * sin(animationPhase * 3.14159 * 2));
        for (int i = 6; i >= 0; i--) {
            int alpha = 40 - (i * 5);
            if (alpha < 5)alpha = 5;
            int r = (GetRValue(glowColor) * alpha) / 100;
            int g = (GetGValue(glowColor) * alpha) / 100;
            int b = (GetBValue(glowColor) * alpha) / 100;
            HPEN glowPen = CreatePen(PS_SOLID, 3, RGB(r, g, b));
            HPEN oldPen = (HPEN)SelectObject(hdc, glowPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            int glowRadius = baseRadius + 15 + (i * 4) + pulseOffset;
            Ellipse(hdc, centerX - glowRadius, centerY - glowRadius, centerX + glowRadius, centerY + glowRadius);
            SelectObject(hdc, oldPen);
            DeleteObject(glowPen);
        }
    }
    HBRUSH mainBrush = CreateSolidBrush(COLOR_DARK_ELEMENT);
    HPEN mainPen = CreatePen(PS_SOLID, 4, borderColor);
    SelectObject(hdc, mainBrush);
    SelectObject(hdc, mainPen);
    Ellipse(hdc, centerX - baseRadius, centerY - baseRadius, centerX + baseRadius, centerY + baseRadius);
    DeleteObject(mainBrush);
    DeleteObject(mainPen);
    if (isConnecting) {
        HPEN rotatingPen = CreatePen(PS_SOLID, 5, COLOR_NEON_TEAL);
        HPEN oldPen = (HPEN)SelectObject(hdc, rotatingPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        for (int i = 0; i < 4; i++) {
            float startAngle = rotationAngle + (i * 90.0f);
            float endAngle = startAngle + 60.0f;
            int arcRadius = baseRadius + 6;
            POINT arcStart = { centerX + (int)(arcRadius * cos(startAngle * 3.14159f / 180.0f)),centerY + (int)(arcRadius * sin(startAngle * 3.14159f / 180.0f)) };
            MoveToEx(hdc, arcStart.x, arcStart.y, NULL);
            for (float angle = startAngle; angle <= endAngle; angle += 3.0f) {
                int x = centerX + (int)(arcRadius * cos(angle * 3.14159f / 180.0f));
                int y = centerY + (int)(arcRadius * sin(angle * 3.14159f / 180.0f));
                LineTo(hdc, x, y);
            }
        }
        SelectObject(hdc, oldPen);
        DeleteObject(rotatingPen);
    }
    int innerRadius = baseRadius - 12;
    HBRUSH innerBrush = CreateSolidBrush(mainColor);
    HPEN innerPen = CreatePen(PS_SOLID, 2, mainColor);
    SelectObject(hdc, innerBrush);
    SelectObject(hdc, innerPen);
    Ellipse(hdc, centerX - innerRadius, centerY - innerRadius, centerX + innerRadius, centerY + innerRadius);
    DeleteObject(innerBrush);
    DeleteObject(innerPen);
    DrawBitmap(hdc, g_hShieldIcon, centerX - 25, centerY - 35, 50, 50);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_BUTTON_TEXT);
    HFONT oldFont = (HFONT)SelectObject(hdc, g_hButtonFont);
    std::wstring buttonText;
    if (isConnecting)buttonText = L"Connecting...";
    else if (isConnected)buttonText = L"Disconnect";
    else buttonText = L"Connect";
    RECT textRect = { pRect->left,centerY + 25,pRect->right,pRect->bottom };
    DrawTextW(hdc, buttonText.c_str(), -1, &textRect, DT_CENTER);
    SelectObject(hdc, oldFont);
}

// Рисование панели статуса
void DrawStatusBar(HDC hdc) {
    // Получаем координаты панели статуса
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

        // Neon gradient background for connected state

        DrawGradientRect(hdc, rect, COLOR_CONNECTED_GLOW, COLOR_CONNECTED_GREEN, COLOR_NEON_TEAL);



        // Draw Germany flag and "Connected" status

        DrawBitmap(hdc, g_hGermanyFlag, rect.left + 20, rect.top + 10, 30, 30);



        SetBkMode(hdc, TRANSPARENT);

        SetTextColor(hdc, COLOR_BUTTON_TEXT);

        HFONT oldFont = (HFONT)SelectObject(hdc, g_hStatusFont);



        RECT textRect = { rect.left + 60, rect.top, rect.right, rect.bottom };

        DrawTextW(hdc, L"✓ Connected", -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, oldFont);

    }

    else {

        // Hide panel when disconnected
        return;
    }
}

HBITMAP CreateFlagBitmap() {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 30, 30);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
    RECT topRect = { 0,0,30,10 };
    RECT middleRect = { 0,10,30,20 };
    RECT bottomRect = { 0,20,30,30 };
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


// Создание битмапа со щитом
HBITMAP CreateShieldBitmap() {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 50, 50);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Заполняем прозрачным фоном
    RECT bmpRect = { 0, 0, 50, 50 };
    FillRect(hdcMem, &bmpRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Рисуем щит
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(220, 220, 220));
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);
    HBRUSH hBrush = CreateSolidBrush(RGB(180, 200, 220));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

    // Контур щита
    POINT shield[8] = {
        {25, 5},   // верх
        {45, 15},  // правый верхний угол
        {45, 30},  // правый середина
        {25, 45},  // низ
        {5, 30},   // левый середина
        {5, 15},   // левый верхний угол
        {25, 5}    // замыкаем обратно к верху
    };
    Polyline(hdcMem, shield, 7);

    // Закрасим щит
    HRGN hRgn = CreatePolygonRgn(shield, 6, ALTERNATE);
    FillRgn(hdcMem, hRgn, hBrush);

    // Рисуем замочную скважину
    SetTextColor(hdcMem, RGB(50, 50, 50));
    SetBkMode(hdcMem, TRANSPARENT);
    HFONT hFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

    TextOutW(hdcMem, 18, 15, L"🔑", 1);

    // Очистка
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

// Создание битмапа с настройками
HBITMAP CreateSettingsBitmap() {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 30, 30);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Заполняем прозрачным фоном
    RECT bmpRect = { 0, 0, 30, 30 };
    FillRect(hdcMem, &bmpRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Рисуем иконку шестеренки
    SetTextColor(hdcMem, RGB(180, 180, 180));
    SetBkMode(hdcMem, TRANSPARENT);
    HFONT hFont = CreateFontW(25, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

    TextOutW(hdcMem, 2, 2, L"⚙", 1);

    // Очистка
    SelectObject(hdcMem, hOldFont);
    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hFont);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return hBitmap;
}

// Создание иконки приложения
HBITMAP CreateAppIcon() {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 16, 16);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Заполняем фоном
    RECT bmpRect = { 0, 0, 16, 16 };
    FillRect(hdcMem, &bmpRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Рисуем щит (упрощенный)
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 120, 215));
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 120, 215));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

    // Контур щита
    POINT shield[7] = {
        {8, 1},   // верх
        {14, 4},  // правый верхний угол
        {14, 10}, // правый середина
        {8, 15},  // низ
        {2, 10},  // левый середина
        {2, 4},   // левый верхний угол
        {8, 1}    // замыкаем обратно к верху
    };
    Polyline(hdcMem, shield, 7);

    // Закрасим щит
    HRGN hRgn = CreatePolygonRgn(shield, 6, ALTERNATE);
    FillRgn(hdcMem, hRgn, hBrush);

    // Рисуем замок внутри
    SetTextColor(hdcMem, RGB(255, 255, 255));
    SetBkMode(hdcMem, TRANSPARENT);
    HFONT hFont = CreateFontW(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

    TextOutW(hdcMem, 6, 4, L"F", 1);

    // Очистка
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

// Отрисовка битмапа на DC
void DrawBitmap(HDC hdc, HBITMAP hBitmap, int x, int y, int width, int height) {
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Получаем размеры битмапа
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);

    // Рисуем битмап с растягиванием до нужного размера
    SetStretchBltMode(hdc, HALFTONE);
    StretchBlt(hdc, x, y, width, height, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
}

// Обновление статуса подключения в UI
void UpdateStatus() {
    bool wasConnected = isConnected;
    isConnected = wireguard.IsConnected();
    
    // Если статус изменился
    if (wasConnected != isConnected) {
        if (isConnected) {
            // Запускаем таймеры для отображения времени соединения и статистики трафика
            connectionStartTime = std::chrono::steady_clock::now();
            SetTimer(g_hWnd, ID_CONNECTION_TIMER, 1000, NULL);  // Обновление времени каждую секунду
            SetTimer(g_hWnd, ID_TRAFFIC_TIMER, 2000, NULL);     // Обновление статистики трафика каждые 2 секунды
            
            // Показываем информационные панели
            ShowWindow(g_hStatusBar, SW_SHOW);
            ShowWindow(g_hReconnectButton, SW_SHOW);
            ShowWindow(g_hProtocolText, SW_SHOW);
            ShowWindow(g_hTimerText, SW_SHOW);
            ShowWindow(g_hIPText, SW_SHOW);
            ShowWindow(g_hUsageText, SW_SHOW);
            
            // Устанавливаем IP-адрес
            SetWindowTextW(g_hIPText, L"🌐 IP: 46.254.107.229");
            
            // Сбрасываем счетчики трафика
            downloadedBytes = 0;
            uploadedBytes = 0;
            UpdateTrafficStats();
        } else {
            // Останавливаем таймеры
            KillTimer(g_hWnd, ID_CONNECTION_TIMER);
            KillTimer(g_hWnd, ID_TRAFFIC_TIMER);
            
            // Скрываем информационные панели
            ShowWindow(g_hStatusBar, SW_HIDE);
            ShowWindow(g_hReconnectButton, SW_HIDE);
            ShowWindow(g_hProtocolText, SW_HIDE);
            ShowWindow(g_hTimerText, SW_HIDE);
            ShowWindow(g_hIPText, SW_HIDE);
            ShowWindow(g_hUsageText, SW_HIDE);
        }
        
        // Перерисовываем окно
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
    
    // Если подключены, обновляем таймер подключения
    if (isConnected) {
        UpdateConnectionTimer();
    }
}

// Обновление таймера подключения
void UpdateConnectionTimer() {
    if (!isConnected) return;

    // Вычисляем прошедшее время
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - connectionStartTime);

    // Форматируем время в HH:MM:SS
    int hours = static_cast<int>(elapsed.count() / 3600);
    int minutes = static_cast<int>((elapsed.count() % 3600) / 60);
    int seconds = static_cast<int>(elapsed.count() % 60);

    wchar_t timeText[50];
    swprintf_s(timeText, L"⏱️ Time: %02d:%02d:%02d", hours, minutes, seconds);

    SetWindowTextW(g_hTimerText, timeText);
}

// Обновление статистики трафика (симуляция)
void UpdateTrafficStats() {
    if (!isConnected) return;

    // Симулируем увеличение трафика
    downloadedBytes += (rand() % 100000) + 50000;  // ~50-150 KB за раз
    uploadedBytes += (rand() % 50000) + 5000;      // ~5-55 KB за раз

    // Форматируем и выводим статистику
    std::wstring downStr = FormatBytes(downloadedBytes);
    std::wstring upStr = FormatBytes(uploadedBytes);

    wchar_t usageText[100];
    swprintf_s(usageText, L"📊 Traffic: %ls ↓ | %ls ↑", downStr.c_str(), upStr.c_str());

    SetWindowTextW(g_hUsageText, usageText);
}

// Форматирование размера данных в читаемый вид
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
    } else {
        swprintf_s(buffer, L"%.2f %ls", size, suffixes[suffixIndex]);
    }

    return std::wstring(buffer);
}

// Переключение состояния подключения
void ToggleConnection() {
    if (isConnecting) {
        return;
    }

    if (isConnected) {
        if (wireguard.Disconnect()) {
            MessageBoxW(g_hWnd, L"✓ Disconnected successfully!", L"Success", MB_OK | MB_ICONINFORMATION);

            reconnectAttempts = 0;

        }

        else {

            MessageBoxW(g_hWnd, L"❌ Disconnection error.", L"Error", MB_OK | MB_ICONERROR);

        }

    }

    else {

        isConnecting = true;

        InvalidateRect(g_hConnectButton, NULL, TRUE);



        // Connect in separate thread to avoid blocking UI

        if (wireguard.Connect()) {

            MessageBoxW(g_hWnd, L"✓ Connected successfully!", L"Success", MB_OK | MB_ICONINFORMATION);

            reconnectAttempts = 0;

        }

        else {

            MessageBoxW(g_hWnd,

                L"❌ Connection error. Check settings and make sure WireGuard is installed.",

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

// Получение кнопки под указанной точкой
HWND GetButtonAtPoint(int x, int y) {
    POINT pt = { x, y };
    
    // Проверяем все интерактивные элементы
    HWND buttons[] = {
        g_hConnectButton,
        g_hSettingsButton,
        g_hLocationButton,
        g_hCloseButton,
        g_hMinimizeButton,
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
        isSettingsHover = (hoverButton == g_hSettingsButton);
        isLocationHover = (hoverButton == g_hLocationButton);
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
        } else if (wParam == ID_TRAFFIC_TIMER) {
            UpdateTrafficStats();
        }
        else if (wParam == ID_ANIMATION_TIMER) {
            animationPhase += 0.05f;

            if (animationPhase >= 1.0f) animationPhase = 0.0f;
            if (isConnecting) {
                rotationAngle += 2.0f;  
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
        
        // Обновляем состояние наведения
        UpdateButtonHoverState(x, y);
        
        // Обработка перетаскивания окна
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
        
        // Проверяем, находится ли курсор в заголовке
        else if (y <= TITLEBAR_HEIGHT && x > 30 && x < APP_WIDTH - 70) {
            isTitleBarHover = true;
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        } else {
            isTitleBarHover = false;
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }
    break;

    case WM_LBUTTONDOWN:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        
        // Начинаем перетаскивание, если клик в области заголовка
        if (y <= TITLEBAR_HEIGHT && x > 30 && x < APP_WIDTH - 70) {
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
            COLORREF bgColor = isLocationHover ? COLOR_GLASS_ELEMENT : COLOR_DARK_ELEMENT;

            COLORREF borderColor = isLocationHover ? COLOR_NEON_CYAN : COLOR_DARK_ELEMENT;



            // Draw glass-effect card with neon border

            DrawRoundedRectangle(pDIS->hDC, rect, 8, bgColor);



            // Draw neon border on hover

            if (isLocationHover) {

                HPEN borderPen = CreatePen(PS_SOLID, 2, borderColor);

                HPEN oldPen = (HPEN)SelectObject(pDIS->hDC, borderPen);

                SelectObject(pDIS->hDC, GetStockObject(NULL_BRUSH));

                RoundRect(pDIS->hDC, rect.left, rect.top, rect.right, rect.bottom, 8, 8);

                SelectObject(pDIS->hDC, oldPen);

                DeleteObject(borderPen);

            }



            // Draw Germany flag

            DrawBitmap(pDIS->hDC, g_hGermanyFlag, rect.left + 15, (rect.top + rect.bottom - 20) / 2, 24, 24);



            SetBkMode(pDIS->hDC, TRANSPARENT);

            SetTextColor(pDIS->hDC, COLOR_BUTTON_TEXT);

            HFONT oldFont = (HFONT)SelectObject(pDIS->hDC, g_hStatusFont);

            RECT textRect = { rect.left + 48, rect.top, rect.right, rect.bottom };

            DrawTextW(pDIS->hDC, L"🇩🇪 Frankfurt", -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SelectObject(pDIS->hDC, oldFont);
            return TRUE;
        }
        else if (pDIS->CtlID == IDC_SETTINGS_BUTTON) {
            RECT rect = pDIS->rcItem;
            COLORREF bgColor = isSettingsHover ? COLOR_GLASS_ELEMENT : COLOR_DARK_ELEMENT;

            COLORREF borderColor = isSettingsHover ? COLOR_NEON_CYAN : COLOR_DARK_ELEMENT;



            DrawRoundedRectangle(pDIS->hDC, rect, 8, bgColor);



            // Neon border on hover

            if (isSettingsHover) {

                HPEN borderPen = CreatePen(PS_SOLID, 2, borderColor);

                HPEN oldPen = (HPEN)SelectObject(pDIS->hDC, borderPen);

                SelectObject(pDIS->hDC, GetStockObject(NULL_BRUSH));

                RoundRect(pDIS->hDC, rect.left, rect.top, rect.right, rect.bottom, 8, 8);

                SelectObject(pDIS->hDC, oldPen);

                DeleteObject(borderPen);

            }
            DrawBitmap(pDIS->hDC, g_hSettingsIcon, rect.left + 5, rect.top + 5, 30, 30);
            return TRUE;
        }
    }
    break;

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;

        // Устанавливаем цвет текста и фона для статических элементов
        SetTextColor(hdcStatic, COLOR_CONNECTION_TEXT);
        SetBkColor(hdcStatic, COLOR_BACKGROUND);
        return (LRESULT)g_hBackgroundBrush;
    }

    case WM_CTLCOLORBTN:
    {
        HDC hdcBtn = (HDC)wParam;
        HWND hwndBtn = (HWND)lParam;

        // Устанавливаем прозрачный фон для кнопок
        SetBkMode(hdcBtn, TRANSPARENT);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case IDC_CLOSE_BUTTON:
            DestroyWindow(hWnd);
            break;

        case IDC_MINIMIZE_BUTTON:
            ShowWindow(hWnd, SW_MINIMIZE);
            break;

        case IDC_CONNECT_BUTTON:
            ToggleConnection();
            break;

        case IDC_RECONNECT_BUTTON:
            if (isConnected) {
                if (wireguard.Disconnect()) {
                    if (wireguard.Connect()) {
                        MessageBoxW(g_hWnd, L"✓ Reconnected successfully!", L"Success", MB_OK | MB_ICONINFORMATION);

                    }

                }

                UpdateStatus();

            }

            break;



        case IDC_LOCATION_BUTTON:

            // Show location selection dialog

            MessageBoxW(hWnd, L"🇩🇪 Current location: Frankfurt, Germany\n\nMore locations coming soon!", L"Location", MB_OK | MB_ICONINFORMATION);
            break;

        case IDC_SETTINGS_BUTTON:
            // Покажем диалог с настройками
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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

        DrawGradientRect(hdc, clientRect, COLOR_BACKGROUND_TOP, COLOR_BACKGROUND, COLOR_BACKGROUND_BOTTOM);



        // Draw title bar

        DrawTitleBar(hdc);



        // Draw header panel (glass effect)

        RECT headerRect = { 0, TITLEBAR_HEIGHT, APP_WIDTH, TITLEBAR_HEIGHT + 40 };

        HBRUSH headerBrush = CreateSolidBrush(COLOR_HEADER);

        FillRect(hdc, &headerRect, headerBrush);

        DeleteObject(headerBrush);



        // Draw app title with neon glow effect

        SetBkMode(hdc, TRANSPARENT);

        SetTextColor(hdc, COLOR_NEON_CYAN);

        HFONT oldFont = (HFONT)SelectObject(hdc, g_hHeaderFont);

        RECT titleRect = { 0, TITLEBAR_HEIGHT + 5, APP_WIDTH, TITLEBAR_HEIGHT + 35 };

        DrawTextW(hdc, L"⚡ FREEDOM VPN", -1, &titleRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        SelectObject(hdc, oldFont);
        EnumChildWindows(hWnd, [](HWND hwnd, LPARAM lParam) -> BOOL {
            InvalidateRect(hwnd, NULL, TRUE);
            return TRUE;
            }, 0);

        EndPaint(hWnd, &ps);
        return 0;
    }
    break;

    case WM_DESTROY:
        // Убедимся, что VPN отключен перед выходом
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

// Обработчик сообщений для окна "О программе"
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Устанавливаем шрифт для диалога О программе
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
        SetTextColor(hdcStatic, RGB(220, 220, 220));
        SetBkColor(hdcStatic, COLOR_DARK_ELEMENT);
        return (INT_PTR)g_hDarkElementBrush;
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