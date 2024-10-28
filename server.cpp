#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>  
#include <string>

#pragma comment(lib, "WSock32.lib")

void RunServer(u_short port);
void RunClient(const char* server_ip, u_short port);

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WinSockの初期化に失敗しました\n");
        return 1;
    }

    int mode;
    printf("モードを選択してください (1: サーバー, 2: クライアント): ");
    std::cin >> mode;

    if (mode == 1) // サーバーモード
    {
        u_short port;
        printf("サーバーのポート番号を入力してください: ");
        std::cin >> port;
        RunServer(port);
    }
    else if (mode == 2) // クライアントモード
    {
        char server_ip[16];
        u_short port;
        printf("接続先のサーバーIPアドレスを入力してください: ");
        std::cin >> server_ip;
        printf("接続先のポート番号を入力してください: ");
        std::cin >> port;
        RunClient(server_ip, port);
    }
    else
    {
        printf("無効なモードです\n");
    }

    WSACleanup();
    return 0;
}

// サーバーの実行関数
void RunServer(u_short port)
{
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        printf("ソケットの作成に失敗しました\n");
        return;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("ソケットのバインドに失敗しました\n");
        closesocket(server_socket);
        return;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR)
    {
        printf("ソケットのリッスンに失敗しました\n");
        closesocket(server_socket);
        return;
    }

    printf("クライアントからの接続を待っています...\n");

    sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_socket == INVALID_SOCKET)
    {
        printf("クライアントの接続に失敗しました\n");
        closesocket(server_socket);
        return;
    }

    printf("クライアントと接続しました\n");

    while (true)
    {
        char recv_buf[1024] = { 0 };
        int nRcv = recv(client_socket, recv_buf, sizeof(recv_buf) - 1, 0);

        if (nRcv == SOCKET_ERROR || nRcv == 0)
        {
            printf("クライアントとの接続が切れました\n");
            break;
        }

        recv_buf[nRcv] = '\0';
        printf("クライアント --> %s\n", recv_buf);

        printf("サーバー --> ");
        char send_buf[1024] = { 0 };
        std::cin >> send_buf;

        int nSent = send(client_socket, send_buf, (int)strlen(send_buf), 0);
        if (nSent == SOCKET_ERROR)
        {
            printf("データの送信に失敗しました\n");
            break;
        }
    }

    closesocket(client_socket);
    closesocket(server_socket);
}

// クライアントの実行関数
void RunClient(const char* server_ip, u_short port)
{
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        printf("ソケットの作成に失敗しました\n");
        return;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("サーバーへの接続に失敗しました\n");
        closesocket(client_socket);
        return;
    }

    printf("サーバーに接続しました\n");

    while (true)
    {
        printf("クライアント --> ");
        char send_buf[1024] = { 0 };
        std::cin >> send_buf;

        int nSent = send(client_socket, send_buf, (int)strlen(send_buf), 0);
        if (nSent == SOCKET_ERROR)
        {
            printf("データの送信に失敗しました\n");
            break;
        }

        char recv_buf[1024] = { 0 };
        int nRcv = recv(client_socket, recv_buf, sizeof(recv_buf) - 1, 0);
        if (nRcv == SOCKET_ERROR || nRcv == 0)
        {
            printf("サーバーとの接続が切れました\n");
            break;
        }

        recv_buf[nRcv] = '\0';
        printf("サーバー --> %s\n", recv_buf);
    }

    closesocket(client_socket);
}
