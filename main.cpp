#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <linux/reboot.h>
#include <sys/reboot.h>

int main(int argc, char const* argv[]) {
    const int port = 8000;

    bool shutdown = false;
    struct sockaddr_in address;
    int new_socket, input;
    int opt = 1;
    int addrlen = sizeof(address);
    char *response = "shutdown command received";

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0) {
        std::cout << "Socket Creation Failed" << std::endl;
        exit(-1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cout << "setsockopt failed";
        exit(-1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr *) &address, sizeof(address))<0) {
        std::cout << "Bind failed" << std::endl;
        exit(-1);
    }

    if(listen(sockfd, 3) < 0){
        std::cout << "Listen failed" << std::endl;
        exit(-1);
    }

    while(1) {
        char buffer[1024] = {0};

        if ((new_socket = accept(sockfd, (struct sockaddr*) &address, (socklen_t*)&addrlen))<0) {
            std::cout << "accept failed" << std::endl;
            exit(-1);
        }

        input = read( new_socket , buffer, 1024);
        if(input <= 0) { break;}

        if(strcmp(buffer, "shutdown -local") == 0) {
            send(new_socket, response, strlen(response), 0);

            shutdown = true;
            close(new_socket);
            break;
        } else {
            send(new_socket, "hello", strlen("hello"),0);
            close(new_socket);
        }

    }

    if(shutdown) {
        std::cout << "Remote shutdown requested" << std::endl;
//        reboot(LINUX_REBOOT_CMD_POWER_OFF);
        reboot(LINUX_REBOOT_CMD_SW_SUSPEND);
    }
    return 0;
}
