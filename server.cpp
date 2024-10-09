#define STRICT

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

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
    int port;

    // WinSock�̏�����
    std::cout << "WinSock��������..." << std::endl;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "���s �G���[�R�[�h : " << WSAGetLastError() << std::endl;
        return 1;
    }

    std::cout << "WinSock������������܂���..." << std::endl;

    // �T�[�o�[�\�P�b�g�̍쐬
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        std::cerr << "�\�P�b�g�̍쐬�Ɏ��s...: " << WSAGetLastError() << std::endl;

        return 1;
    }

    std::cout << "�\�P�b�g���쐬" << std::endl;

    // �|�[�g�̓���
    std::cout << "�T�[�o�̃|�[�g�ԍ�����͂��Ă�������: ";
    std::cin >> port;
    std::cin.ignore();

    // �T�[�o�[��IP�A�h���X�ƃ|�[�g�ԍ���ݒ�
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // �\�P�b�g���o�C���h
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "�o�C���h�Ɏ��s �G���[�R�[�h: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();

        return 1;
    }

    std::cout << "�o�C���h������" << std::endl;

    // �ڑ���҂�
    listen(server_socket, 3);
    std::cout << "���b�Z�[�W�̑��M��ҋ@��" << std::endl;

    // �N���C�A���g�̐ڑ����󂯓����
    client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);

    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "��M�Ɏ��s �G���[�R�[�h: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();

        return 1;
    }

    std::cout << "�ڑ����� (�ڑ����I������ꍇ��'quit'����͂��Ă�������)" << std::endl;

    // ���b�Z�[�W�������[�v
    int recv_size;
    while (true)
    {
        recv_size = recv(client_socket, buffer, 1024, 0);

        if (recv_size == SOCKET_ERROR)
        {
            std::cerr << "��M�Ɏ��s �G���[�R�[�h: " << WSAGetLastError() << std::endl;
            break;
        }
        else if (recv_size == 0)
        {
            std::cout << "�N���C�A���g���ؒf���܂���" << std::endl;
            break;
        }

        buffer[recv_size] = '\0';

        std::cout << "�N���C�A���g�̃��b�Z�[�W: " << buffer << std::endl;

        std::cout << "���M���郁�b�Z�[�W����͂��Ă�������: ";
        std::cin.getline(buffer, 1024);

        if (strcmp(buffer, "quit") == 0)
        {
            std::cout << "�ڑ����I��" << std::endl;
            break;
        }

        if (send(client_socket, buffer, strlen(buffer), 0) < 0)
        {
            std::cerr << "���b�Z�[�W�̑��M�Ɏ��s �G���[�R�[�h: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    // �\�P�b�g�����
    closesocket(client_socket);

    closesocket(server_socket);

    WSACleanup();

    return 0;
}
