#include <windows.h>
#include <sstream>
#include <string>

using namespace std;

// Identificadores únicos para los botones y el cuadro de texto
#define ID_BUTTON1 1
#define ID_BUTTON2 2
#define ID_EDIT    3
#define ID_LABEL1  4
#define ID_LABEL2  5
#define ID_LABEL3  6

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
HWND hButton1, hButton2, hEdit, hLabel1, hLabel2, hLabel3;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch(Message) {
        
        case WM_CREATE:
        	// Crear etiqueta de texto
        	hLabel1 = CreateWindowEx(0,"STATIC", "Almacen El Fuerte.",
									WS_CHILD | WS_VISIBLE | SS_CENTER,
									50,10,300,20,
									hwnd, (HMENU)ID_LABEL1,NULL,NULL);
			
			// Crear etiqueta de texto
        	hLabel2 = CreateWindowEx(0,"STATIC", "Telefono 774-3923.",
									WS_CHILD | WS_VISIBLE | SS_CENTER,
									50,35,300,20,
									hwnd, (HMENU)ID_LABEL2,NULL,NULL);
			
			// Crear etiqueta de texto
        	hLabel3 = CreateWindowEx(0,"STATIC", "Avenida del Sol 456, Colonia Jardines.",
									WS_CHILD | WS_VISIBLE | SS_CENTER,
									50,60,300,20,
									hwnd, (HMENU)ID_LABEL3,NULL,NULL);
									
            // Crear cuadro de texto
            hEdit = CreateWindowEx(0, "EDIT", "",
                                   WS_CHILD | WS_VISIBLE | WS_BORDER,
                                   50, 90, 300, 25,
                                   hwnd, (HMENU)ID_EDIT, NULL, NULL);
            
            // Crear botón 1
            hButton1 = CreateWindowEx(0, "BUTTON", "Calcular Aumento",
                                      WS_CHILD | WS_VISIBLE,
                                      50, 130, 120, 30,
                                      hwnd, (HMENU)ID_BUTTON1, NULL, NULL);
            
            // Crear botón 2
            hButton2 = CreateWindowEx(0, "BUTTON", "Limpiar",
                                      WS_CHILD | WS_VISIBLE,
                                      200, 130, 120, 30,
                                      hwnd, (HMENU)ID_BUTTON2, NULL, NULL);
            break;
        
        case WM_COMMAND:
            // Identificar qué botón se presionó
            switch(LOWORD(wParam)) {
                case ID_BUTTON1:
                	{
                		
					
                	char buffer[100];
                	GetWindowText(hEdit, buffer, 100);
                	double salario = atof(buffer);
                	double aumento = 0.0;
                	double por_aumento;
					double total = 0.0;
					
					if (salario <= 0) {
						    MessageBox(hwnd, "Por favor ingrese un salario válido.", "Error", MB_OK | MB_ICONWARNING);
						    break;
						}

                	
                	if (salario<= 500.00){
                		por_aumento=0.20;
					}else if (salario<=1000.00){
						por_aumento=0.10;
					}else if (salario<=2000.00){
						por_aumento=0.05;
					}else {
						por_aumento=0.03;
					}
				
					aumento=por_aumento*salario;
					total= salario+ aumento;
					
					stringstream ss;
					ss.precision(2);
					ss<<fixed;
					ss<< "Aumento: $"<< aumento <<"  | Total: $"<<total;
					
                    SetWindowText(hEdit, ss.str().c_str());
                    break;
                }
                
                case ID_BUTTON2:
                    SetWindowText(hEdit, "");
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