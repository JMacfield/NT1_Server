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
        printf("WinSock�̏������Ɏ��s���܂���\n");
        return 1;
    }

    int mode;
    printf("���[�h��I�����Ă������� (1: �T�[�o�[, 2: �N���C�A���g): ");
    std::cin >> mode;

    if (mode == 1) // �T�[�o�[���[�h
    {
        u_short port;
        printf("�T�[�o�[�̃|�[�g�ԍ�����͂��Ă�������: ");
        std::cin >> port;
        RunServer(port);
    }
    else if (mode == 2) // �N���C�A���g���[�h
    {
        char server_ip[16];
        u_short port;
        printf("�ڑ���̃T�[�o�[IP�A�h���X����͂��Ă�������: ");
        std::cin >> server_ip;
        printf("�ڑ���̃|�[�g�ԍ�����͂��Ă�������: ");
        std::cin >> port;
        RunClient(server_ip, port);
    }
    else
    {
        printf("�����ȃ��[�h�ł�\n");
    }

    WSACleanup();
    return 0;
}

// �T�[�o�[�̎��s�֐�
void RunServer(u_short port)
{
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        printf("�\�P�b�g�̍쐬�Ɏ��s���܂���\n");
        return;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("�\�P�b�g�̃o�C���h�Ɏ��s���܂���\n");
        closesocket(server_socket);
        return;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR)
    {
        printf("�\�P�b�g�̃��b�X���Ɏ��s���܂���\n");
        closesocket(server_socket);
        return;
    }

    printf("�N���C�A���g����̐ڑ���҂��Ă��܂�...\n");

    sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_socket == INVALID_SOCKET)
    {
        printf("�N���C�A���g�̐ڑ��Ɏ��s���܂���\n");
        closesocket(server_socket);
        return;
    }

    printf("�N���C�A���g�Ɛڑ����܂���\n");

    while (true)
    {
        char recv_buf[1024] = { 0 };
        int nRcv = recv(client_socket, recv_buf, sizeof(recv_buf) - 1, 0);

        if (nRcv == SOCKET_ERROR || nRcv == 0)
        {
            printf("�N���C�A���g�Ƃ̐ڑ����؂�܂���\n");
            break;
        }

        recv_buf[nRcv] = '\0';
        printf("�N���C�A���g --> %s\n", recv_buf);

        printf("�T�[�o�[ --> ");
        char send_buf[1024] = { 0 };
        std::cin >> send_buf;

        int nSent = send(client_socket, send_buf, (int)strlen(send_buf), 0);
        if (nSent == SOCKET_ERROR)
        {
            printf("�f�[�^�̑��M�Ɏ��s���܂���\n");
            break;
        }
    }

    closesocket(client_socket);
    closesocket(server_socket);
}

// �N���C�A���g�̎��s�֐�
void RunClient(const char* server_ip, u_short port)
{
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        printf("�\�P�b�g�̍쐬�Ɏ��s���܂���\n");
        return;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("�T�[�o�[�ւ̐ڑ��Ɏ��s���܂���\n");
        closesocket(client_socket);
        return;
    }

    printf("�T�[�o�[�ɐڑ����܂���\n");

    while (true)
    {
        printf("�N���C�A���g --> ");
        char send_buf[1024] = { 0 };
        std::cin >> send_buf;

        int nSent = send(client_socket, send_buf, (int)strlen(send_buf), 0);
        if (nSent == SOCKET_ERROR)
        {
            printf("�f�[�^�̑��M�Ɏ��s���܂���\n");
            break;
        }

        char recv_buf[1024] = { 0 };
        int nRcv = recv(client_socket, recv_buf, sizeof(recv_buf) - 1, 0);
        if (nRcv == SOCKET_ERROR || nRcv == 0)
        {
            printf("�T�[�o�[�Ƃ̐ڑ����؂�܂���\n");
            break;
        }

        recv_buf[nRcv] = '\0';
        printf("�T�[�o�[ --> %s\n", recv_buf);
    }

    closesocket(client_socket);
}
