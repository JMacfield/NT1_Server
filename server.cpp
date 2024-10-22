#define STRICT
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <Windows.h>
#include <tchar.h>
#include <thread>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

#include "resource.h"

#define PORT 8000
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define SYNC_TIMER_ID 1
#define SYNC_INTERVAL_MS 30  // 30�~���b���Ƃɓ���

HINSTANCE hInstanceGlobal;
HBITMAP serverBitmap, clientBitmap;
POINT serverBitmapPos = { 50, 50 };
POINT clientBitmapPos = { 150, 50 };
SOCKET networkSocket;
bool isServerMode;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void LoadBitmaps()
{
    serverBitmap = LoadBitmap(hInstanceGlobal, MAKEINTRESOURCE(101));  // �T�[�o�[�p�r�b�g�}�b�v
    clientBitmap = LoadBitmap(hInstanceGlobal, MAKEINTRESOURCE(102));  // �N���C�A���g�p�r�b�g�}�b�v

    if (!serverBitmap || !clientBitmap)
    {
        MessageBox(NULL, _T("�r�b�g�}�b�v�̃��[�h�Ɏ��s"), _T("�G���["), MB_ICONERROR);
    }
}

void NetworkSyncThread()
{
    char buffer[BUFFER_SIZE];

    while (true)
    {
        if (isServerMode)
        {
            // �T�[�o�[���̃r�b�g�}�b�v�ʒu�𑗐M
            sprintf_s(buffer, "%d %d", serverBitmapPos.x, serverBitmapPos.y);
            send(networkSocket, buffer, sizeof(buffer), 0);

            // �N���C�A���g���̃r�b�g�}�b�v�ʒu����M
            recv(networkSocket, buffer, sizeof(buffer), 0);
            sscanf_s(buffer, "%d %d", &clientBitmapPos.x, &clientBitmapPos.y);
        }
        else
        {
            // �N���C�A���g���̃r�b�g�}�b�v�ʒu�𑗐M
            sprintf_s(buffer, "%d %d", clientBitmapPos.x, clientBitmapPos.y);
            send(networkSocket, buffer, sizeof(buffer), 0);

            // �T�[�o�[���̃r�b�g�}�b�v�ʒu����M
            recv(networkSocket, buffer, sizeof(buffer), 0);
            sscanf_s(buffer, "%d %d", &serverBitmapPos.x, &serverBitmapPos.y);
        }

        Sleep(SYNC_INTERVAL_MS);
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    hInstanceGlobal = hInstance;

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        MessageBox(NULL, _T("WSAStartup�Ɏ��s"), _T("�G���["), MB_ICONERROR);
        return 1;
    }

    networkSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (networkSocket == INVALID_SOCKET)
    {
        MessageBox(NULL, _T("�\�P�b�g�̍쐬�Ɏ��s"), _T("�G���["), MB_ICONERROR);
        WSACleanup();
        return 1;
    }

    int choice = MessageBox(NULL, _T("�T�[�o�[�Ƃ��ĊJ�n���܂����H"), _T("�l�b�g���[�N�Z�b�g�A�b�v"), MB_YESNO);
    isServerMode = (choice == IDYES);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = isServerMode ? INADDR_ANY : inet_addr(SERVER_IP);

    if (isServerMode)
    {
        bind(networkSocket, (sockaddr*)&addr, sizeof(addr));
        listen(networkSocket, 1);

        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        networkSocket = accept(networkSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    }
    else
    {
        connect(networkSocket, (sockaddr*)&addr, sizeof(addr));
    }

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("NetworkWindow");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, _T("Network Bitmap"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    LoadBitmaps();

    std::thread networkThread(NetworkSyncThread);
    networkThread.detach();

    // 30�~���b���ƂɃ^�C�}�[�C�x���g�𔭐�������
    SetTimer(hwnd, SYNC_TIMER_ID, SYNC_INTERVAL_MS, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closesocket(networkSocket);
    WSACleanup();

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_TIMER:
    {
        // �^�C�}�[�C�x���g�ŉ�ʂ������ĕ`��
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC hdcMem = CreateCompatibleDC(hdc);

        // �T�[�o�[�̃r�b�g�}�b�v��`��
        SelectObject(hdcMem, serverBitmap);
        BitBlt(hdc, serverBitmapPos.x, serverBitmapPos.y, 100, 100, hdcMem, 0, 0, SRCCOPY);

        // �N���C�A���g�̃r�b�g�}�b�v��`��
        SelectObject(hdcMem, clientBitmap);
        BitBlt(hdc, clientBitmapPos.x, clientBitmapPos.y, 100, 100, hdcMem, 0, 0, SRCCOPY);

        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_KEYDOWN:
    {
        if (isServerMode)
        {
            switch (wparam)
            {
            case VK_LEFT: serverBitmapPos.x -= 5; break;
            case VK_RIGHT: serverBitmapPos.x += 5; break;
            case VK_UP: serverBitmapPos.y -= 5; break;
            case VK_DOWN: serverBitmapPos.y += 5; break;
            }
        }
        else
        {
            switch (wparam)
            {
            case VK_LEFT: clientBitmapPos.x -= 5; break;
            case VK_RIGHT: clientBitmapPos.x += 5; break;
            case VK_UP: clientBitmapPos.y -= 5; break;
            case VK_DOWN: clientBitmapPos.y += 5; break;
            }
        }

        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}
