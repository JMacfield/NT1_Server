#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() 
{
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);
    char buffer[1024];

    // WinSockの初期化
    std::cout << "WinSockを初期化..." << std::endl;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "失敗 エラーコード : " << WSAGetLastError() << std::endl;
        return 1;
    }

    std::cout << "WinSockが初期化されました..." << std::endl;

    // サーバーソケットの作成
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        std::cerr << "ソケットの作成に失敗...: " << WSAGetLastError() << std::endl;

        return 1;
    }

    std::cout << "ソケットを作成" << std::endl;

    // サーバーのIPアドレスとポート番号を設定
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);

    // ソケットをバインド
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "バインドに失敗 エラーコード: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();

        return 1;
    }

    std::cout << "バインドを完了" << std::endl;

    // 接続を待つ
    listen(server_socket, 3);
    std::cout << "メッセージの送信を待機中" << std::endl;

    // クライアントの接続を受け入れる
    client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);

    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "受信に失敗 エラーコード: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();

        return 1;
    }

    std::cout << "接続完了" << std::endl;

    // メッセージ処理ループ
    int recv_size;
    while (true)
    {
        recv_size = recv(client_socket, buffer, 1024, 0);
        if (recv_size == SOCKET_ERROR)
        {
            std::cerr << "受信に失敗 エラーコード: " << WSAGetLastError() << std::endl;
            break;
        }
        else if (recv_size == 0)
        {
            std::cout << "クライアントが切断しました" << std::endl;
            break;
        }

        buffer[recv_size] = '\0';

        std::cout << "クライアントのメッセージ: " << buffer << std::endl;
    }

    // ソケットを閉じる
    closesocket(client_socket);

    closesocket(server_socket);

    WSACleanup();

    return 0;
}
