// Сhecksum.cpp : Определяет точку входа для приложения.
//
#include <filesystem>
#include <windows.h>
#include <shlobj.h>
#include <windows.h>
#include <shobjidl.h> 
#include <iostream>
#include <fstream>
#include "framework.h"
#include "Сhecksum.h"
#include <string>
#include <iostream>
#include <vector>
#include "MD5.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <numeric>

#define MAX_LOADSTRING 100

namespace fs = std::filesystem;
#pragma warning(disable : 4996)

HINSTANCE hInst;                             
WCHAR szTitle[MAX_LOADSTRING];                 
WCHAR szWindowClass[MAX_LOADSTRING];           
TCHAR* FolderPath;

typedef struct
{
    TCHAR FileName[MAX_PATH];
    TCHAR Hash[MAX_PATH];
}FileHash;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HECKSUM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HECKSUM));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

std::vector<std::string> ListFiles(const std::filesystem::path& path)
{
    std::vector<std::string> files;
    if (!std::filesystem::exists(path))
    {
        std::cout << "Path does not exist: " << path << std::endl;
        return files;
    }

    std::filesystem::recursive_directory_iterator dir(path), end;

    while (dir != end)
    {
        if (std::filesystem::is_regular_file(dir->path()))
        {
            files.push_back(dir->path().string());
        }
        ++dir;
    }
    return files;
}

TCHAR* BrowseForFolder(HWND hwnd)
{
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"Select your folder:";
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != 0)
    {
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path))
        {
            SetWindowText(hwnd, path);
            FolderPath = path;
        }

        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }
        return path;
    }

    return NULL;
}

TCHAR* StringToTCHAR(const std::string& s)
{
    TCHAR* param = new TCHAR[s.size() + 1];
    param[s.size()] = 0;

    // Копируем содержимое строки std::string в массив TCHAR
    std::copy(s.begin(), s.end(), param);

    return param;
}

std::string TCHARToString(const TCHAR* tcharStr) 
{
    std::string str;
#ifdef UNICODE
    size_t len = wcslen(tcharStr) + 1;
    char* cStr = new char[len];
    wcstombs(cStr, tcharStr, len);
    str = cStr;
    delete[] cStr;
#else
    str = tcharStr;
#endif
    return str;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) 
    {
        throw std::runtime_error("Не удалось открыть файл");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HECKSUM));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HECKSUM);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   HWND hwndButton = CreateWindow(
       L"BUTTON",  // Predefined class; Unicode assumed 
       L"Выбрать файл",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       100,         // x position 
       100,         // y position 
       200,        // Button width
       100,        // Button height
       hWnd,     // Parent window
       (HMENU)111,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);   

   HWND FolderButton = CreateWindow(
       L"BUTTON",  // Predefined class; Unicode assumed 
       L"Выбрать папку",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       100,         // x position 
       200,         // y position 
       200,        // Button width
       100,        // Button height
       hWnd,     // Parent window
       (HMENU)115,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);      // Pointer not needed.

   HWND labelHash = CreateWindow(
       L"STATIC",  // Predefined class; Unicode assumed 
       L"Контрольная сумма",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       20,         // x position 
       10,         // y position 
       500,        // Button width
       20,        // Button height
       hWnd,     // Parent window
       NULL,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);  

   HWND labelFolderHash = CreateWindow(
       L"STATIC",  // Predefined class; Unicode assumed 
       L"Контрольная сумма папки",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       600,         // x position 
       400,         // y position 
       500,        // Button width
       20,        // Button height
       hWnd,     // Parent window
       NULL,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   HWND labelFolderList = CreateWindow(
       L"STATIC",  // Predefined class; Unicode assumed 
       L"Список файлов",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       20,         // x position 
       400,         // y position 
       500,        // Button width
       20,        // Button height
       hWnd,     // Parent window
       NULL,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   HWND labelResult = CreateWindow(
       L"EDIT",  // Predefined class; Unicode assumed 
       L"",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       20,         // x position 
       40,         // y position 
       500,        // Button width
       20,        // Button height
       hWnd,     // Parent window
       (HMENU)112,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);      // Pointer not needed.

   HWND labelPath = CreateWindow(
       L"STATIC",  // Predefined class; Unicode assumed 
       L"Файл",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       600,         // x position 
       10,         // y position 
       500,        // Button width
       20,        // Button height
       hWnd,     // Parent window
       NULL,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   HWND labelPathFolder = CreateWindow(
       L"STATIC",  // Predefined class; Unicode assumed 
       L"Папка",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       600,         // x position 
       150,         // y position 
       500,        // Button width
       20,        // Button height
       hWnd,     // Parent window
       NULL,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   HWND labelPathResult = CreateWindow(
       L"EDIT",  // Predefined class; Unicode assumed 
       L"",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       600,         // x position 
       40,         // y position 
       500,        // Button width
       20,        // Button height
       hWnd,     // Parent window
       (HMENU)113,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   HWND labelFolderPathResult = CreateWindow(
       L"EDIT",  // Predefined class; Unicode assumed 
       L"",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       600,         // x position 
       200,         // y position 
       500,        // Button width
       40,        // Button height
       hWnd,     // Parent window
       (HMENU)116,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   HWND labelFolderHashResult = CreateWindow(
       L"EDIT",  // Predefined class; Unicode assumed 
       L"",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       600,         // x position 
       500,         // y position 
       500,        // Button width
       40,        // Button height
       hWnd,     // Parent window
       (HMENU)118,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   HWND pathList = CreateWindow(
       L"LISTBOX",  // Predefined class; Unicode assumed 
       NULL,      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       20,         // x position 
       500,         // y position 
       500,        // Button width
       200,        // Button height
       hWnd,     // Parent window
       (HMENU)117,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId;
    switch (message)
    {
    case WM_COMMAND:
    {
        if (wParam == 111)    // если нажали на кнопку
        {
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                    IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                int strLength
                                    = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1,
                                        nullptr, 0, nullptr, nullptr);

                                // Create a std::string with the determined length 
                                std::string str(strLength, 0);

                                // Perform the conversion from LPCWSTR to std::string 
                                WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &str[0],
                                    strLength, nullptr, nullptr);

                                std::string a = MD5::hash(readFile(str));

                                HWND hwndControl = GetDlgItem(hWnd, 112);


                                SetWindowText(hwndControl, StringToTCHAR(a));

                                hwndControl = GetDlgItem(hWnd, 113);

                                SetWindowText(hwndControl, StringToTCHAR(str));

                                //MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
                                CoTaskMemFree(pszFilePath);
                            }
                            pItem->Release();
                        }
                    }
                    pFileOpen->Release();
                }
                CoUninitialize();
            }
        }
        if (wParam == 115)    // если нажали на кнопку
        {
            HWND hwndControl = GetDlgItem(hWnd, 116);
            TCHAR* a = BrowseForFolder(hwndControl);
            if (a == NULL) 
            {
                break;
            }
            std::string ab = TCHARToString(a);
            TCHAR StrA[200];
            GetWindowText(hwndControl, StrA, 200);
            std::vector<std::string> list = ListFiles(TCHARToString(StrA));
            std::vector<std::string> listHash;


            HWND hwndList = GetDlgItem(hWnd, 117);


            for (int i = 0; i < list.size(); i++)
            {
                std::string hash = MD5::hash(readFile(list[i]));
                int pos = (int)SendMessage(hwndList, LB_ADDSTRING, 0,
                    (LPARAM)StringToTCHAR(list[i]));
                int posHash = (int)SendMessage(hwndList, LB_ADDSTRING, 0,
                    (LPARAM)StringToTCHAR(hash));
                listHash.push_back(hash);
                SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)i);
                SendMessage(hwndList, LB_SETITEMDATA, posHash, (LPARAM)i);
            }

            std::string result = std::accumulate(listHash.begin(), listHash.end(), std::string(""));
            HWND hwndSumHash = GetDlgItem(hWnd, 118);
            SetWindowText(hwndSumHash, StringToTCHAR(MD5::hash(result)));
        }
        break;

        int wmId = LOWORD(wParam);
        // Разобрать выбор в меню:
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
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
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

// Обработчик сообщений для окна "О программе".
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



