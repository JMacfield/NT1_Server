#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

HWND hwMain;

// ����M������W�f�[�^
struct POS
{
    int x;
    int y;
};
POS pos1P, pos2P, old_pos2P;
RECT rect;

// �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI Threadfunc(void*);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR szCmdLine, _In_ int iCmdShow)
{
    MSG msg;
    WNDCLASS wndclass;
    WSADATA wdData;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "CWindow";

    RegisterClass(&wndclass);

    // Winsock������
    WSAStartup(MAKEWORD(2, 0), &wdData);

    hwMain = CreateWindow("CWindow", "Server",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInstance, NULL);

    // �E�C���h�E�\��
    ShowWindow(hwMain, iCmdShow);

    // �E�B���h�E�̈�X�V(WM_PAINT���b�Z�[�W�𔭍s)
    UpdateWindow(hwMain);

    // ���b�Z�[�W���[�v
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Winsock�I��
    WSACleanup();

    return 0;
}

// �E�C���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc, mdc, mdc2P;
    static PAINTSTRUCT ps;
    static HBITMAP hBitmap, hBitmap2P;
    static char str[256];

    static HANDLE hThread;
    static DWORD dwID;

    // WINDOWS������ŗ��郁�b�Z�[�W�ɑΉ����鏈���̋L�q
    switch (iMsg)
    {
    case WM_CREATE:
        // ���\�[�X����r�b�g�}�b�v��ǂݍ��ށi1P�j
        hBitmap = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP1));

        // �f�B�X�v���C�ƌ݊����̂���_���f�o�C�X�i�f�o�C�X�R���e�L�X�g�j���擾�i1P�j
        mdc = CreateCompatibleDC(NULL);

        // �_���f�o�C�X�ɓǂݍ��񂾃r�b�g�}�b�v��W�J�i1P�j
        SelectObject(mdc, hBitmap);

        // ���\�[�X����r�b�g�}�b�v��ǂݍ��ށi2P�j
        hBitmap2P = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP2));

        // �i2P�j
        mdc2P = CreateCompatibleDC(NULL);

        // �i2P�j
        SelectObject(mdc2P, hBitmap2P);

        // �ʒu����������
        pos1P.x = pos1P.y = 0;
        pos2P.x = pos2P.y = 100;

        // �f�[�^�𑗎�M�������X���b�h�iWinMain�̗���Ɋ֌W�Ȃ����삷�鏈���̗���j�Ƃ��Đ����B
        hThread = CreateThread(NULL, 0, Threadfunc, (LPVOID)&pos1P, 0, &dwID);
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;
        case VK_RIGHT:
            pos1P.x += 5;
            break;
        case VK_LEFT:
            pos1P.x -= 5;
            break;
        case VK_DOWN:
            pos1P.y += 5;
            break;
        case VK_UP:
            pos1P.y -= 5;
            break;
        }

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        // �T�[�o���L�����`��
        BitBlt(hdc, pos1P.x, pos1P.y, 32, 32, mdc, 0, 0, SRCCOPY);
        // �N���C�A���g���L�����`��
        BitBlt(hdc, pos2P.x, pos2P.y, 32, 32, mdc2P, 0, 0, SRCCOPY);

        wsprintf(str, "�T�[�o���FX:%d Y:%d�@�@�N���C�A���g���FX:%d Y:%d", pos1P.x, pos1P.y, pos2P.x, pos2P.y);
        SetWindowText(hwMain, str);

        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        DeleteObject(hBitmap);
        DeleteDC(mdc);
        DeleteObject(hBitmap2P);
        DeleteDC(mdc2P);

        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

// �ʐM�X���b�h�֐�
DWORD WINAPI Threadfunc(void* px)
{
    SOCKET sWait, sConnect;
    WORD wPort = 8000;
    SOCKADDR_IN saConnect, saLocal;
    int iLen, iRecv;

    sWait = socket(AF_INET, SOCK_STREAM, 0);
    ZeroMemory(&saLocal, sizeof(saLocal));

    saLocal.sin_family = AF_INET;
    saLocal.sin_addr.s_addr = INADDR_ANY;
    saLocal.sin_port = htons(wPort);

    if (bind(sWait, (LPSOCKADDR)&saLocal, sizeof(saLocal)) == SOCKET_ERROR)
    {
        closesocket(sWait);
        SetWindowText(hwMain, "�ڑ��ҋ@�\�P�b�g���s");
        return 1;
    }

    if (listen(sWait, 2) == SOCKET_ERROR)
    {
        closesocket(sWait);
        SetWindowText(hwMain, "�ڑ��ҋ@�\�P�b�g���s");
        return 1;
    }

    SetWindowText(hwMain, "�ڑ��ҋ@�\�P�b�g����");

    iLen = sizeof(saConnect);

    sConnect = accept(sWait, (LPSOCKADDR)&saConnect, &iLen);
    closesocket(sWait);

    if (sConnect == INVALID_SOCKET)
    {
        SetWindowText(hwMain, "�\�P�b�g�ڑ����s");
        return 1;
    }

    SetWindowText(hwMain, "�\�P�b�g�ڑ�����");

    while (1)
    {
        old_pos2P = pos2P;
        int nRcv = recv(sConnect, (char*)&pos2P, sizeof(POS), 0);

        if (nRcv == SOCKET_ERROR) break;

        send(sConnect, (const char*)&pos1P, sizeof(POS), 0);

        if (old_pos2P.x != pos2P.x || old_pos2P.y != pos2P.y)
        {
            rect.left = old_pos2P.x - 10;
            rect.top = old_pos2P.y - 10;
            rect.right = old_pos2P.x + 42;
            rect.bottom = old_pos2P.y + 42;
            InvalidateRect(hwMain, &rect, TRUE);
        }
    }

    shutdown(sConnect, 2);
    closesocket(sConnect);

    return 0;
}
