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

HINSTANCE g_hInstance;
HBITMAP hBitmap;
POINT imagePos = { 50,50 };
SOCKET g_socket;
bool isServer;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void LoadBitmapResource(int resourceID)
{
	hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(resourceID));

	if (!hBitmap)
	{
		MessageBox(NULL, _T("ビットマップのロードに失敗"), _T("エラー"), MB_ICONERROR);
	}
}

void NetworkThread()
{
	char buffer[BUFFER_SIZE];

	while (true)
	{
		if (isServer)
		{
			printf_s(buffer, "%d %d", imagePos.x, imagePos.y);
			send(g_socket, buffer, sizeof(buffer), 0);
		}
		else
		{
			recv(g_socket, buffer, sizeof(buffer), 0);
			scanf_s(buffer, "%d %d", &imagePos.x, &imagePos.y);
		}

		Sleep(30);
	}
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		MessageBox(NULL, _T("WSAStartupに失敗"), _T("エラー"), MB_ICONERROR);

		return 1;
	}

	g_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (g_socket == INVALID_SOCKET)
	{
		MessageBox(NULL, _T("ソケットの作成に失敗"), _T("エラー"), MB_ICONERROR);
		WSACleanup();

		return 1;
	}

	int choice = MessageBox(NULL, _T("サーバーとして開始しますか？"), _T("ネットワークセットアップ"), MB_YESNO);
	isServer = (choice == IDYES);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = isServer ? INADDR_ANY : inet_addr(SERVER_IP);

	if (isServer)
	{
		bind(g_socket, (sockaddr*)&addr, sizeof(addr));
		listen(g_socket, 1);

		sockaddr_in clientAddr;

		int clientAddrSize = sizeof(clientAddr);
		g_socket = accept(g_socket, (sockaddr*)&clientAddr, &clientAddrSize);
	}
	else
	{
		connect(g_socket, (sockaddr*)&addr, sizeof(addr));
	}

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = _T("NetwordWindow");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	RegisterClass(&wc);

	HWND hwnd = CreateWindow(wc.lpszClassName, _T("Network Bitmap"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	LoadBitmapResource(isServer ? IDB_BITMAP1 : IDB_BITMAP2);

	std::thread networkThread(NetworkThread);
	networkThread.detach();

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	closesocket(g_socket);
	WSACleanup();

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		HDC hdcMem = CreateCompatibleDC(hdc);
		SelectObject(hdcMem, hBitmap);
		BitBlt(hdc, imagePos.x, imagePos.y, 100, 100, hdcMem, 0, 0, SRCCOPY);
		DeleteDC(hdcMem);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wparam)
		{
		case VK_LEFT: imagePos.x -= 5; break;
		case VK_RIGHT: imagePos.x += 5; break;
		case VK_UP: imagePos.y -= 5; break;
		case VK_DOWN: imagePos.y += 5; break;
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