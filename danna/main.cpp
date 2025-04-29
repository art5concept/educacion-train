#include <windows.h>

// Identificadores únicos para los botones y el cuadro de texto
#define ID_BUTTON1 1
#define ID_BUTTON2 2
#define ID_EDIT    3

// Declaraciones
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG msg;

    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = "WindowClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "WindowClass", "Mi primer programa GUI",
        WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

// Variables globales para los controles
HWND hButton1, hButton2, hEdit;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch(Message) {
        
        case WM_CREATE:
            // Crear cuadro de texto
            hEdit = CreateWindowEx(0, "EDIT", "",
                                   WS_CHILD | WS_VISIBLE | WS_BORDER,
                                   50, 50, 300, 25,
                                   hwnd, (HMENU)ID_EDIT, NULL, NULL);
            
            // Crear botón 1
            hButton1 = CreateWindowEx(0, "BUTTON", "Mostrar Hola",
                                      WS_CHILD | WS_VISIBLE,
                                      50, 100, 120, 30,
                                      hwnd, (HMENU)ID_BUTTON1, NULL, NULL);
            
            // Crear botón 2
            hButton2 = CreateWindowEx(0, "BUTTON", "Mostrar Adiós",
                                      WS_CHILD | WS_VISIBLE,
                                      200, 100, 120, 30,
                                      hwnd, (HMENU)ID_BUTTON2, NULL, NULL);
            break;
        
        case WM_COMMAND:
            // Identificar qué botón se presionó
            switch(LOWORD(wParam)) {
                case ID_BUTTON1:
                    SetWindowText(hEdit, "Hola Mundo!");
                    break;
                case ID_BUTTON2:
                    SetWindowText(hEdit, "Adiós Mundo!");
                    break;
            }
            break;
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        
        default:
            return DefWindowProc(hwnd, Message, wParam, lParam);
    }
    return 0;
}