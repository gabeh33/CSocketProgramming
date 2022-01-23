#include <stdio.h>
#define TOKEN_SIZE 128
#define crypto_secretbox_MACBYTES 50
#define PAYLOAD_SIZE crypto_secretbox_MACBYTES + TOKEN_SIZE
#define MSG_ASK "Can I get the solution to the challenge, please?"
#define STATUS_BAD 0
#define STATUS_GOOD 1

// Socket stuff
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#define PORT 4000
#define IP "192.168.1.77"

struct message {
    int hacker_id; /* this is just the number part of the ID */
    int status;
    unsigned char nonce[100]; //crypto_secretbox_NONCEBYTES
    unsigned char payload[PAYLOAD_SIZE];
};

int main() {
    unsigned char nonce_random[100] = "nonce";
    unsigned char payload_random[PAYLOAD_SIZE] = "payload";
    struct message test_message;
    test_message.hacker_id = 19;
    test_message.nonce[0] = 20;
    test_message.payload[0] = 35;

    // Create the socket
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("192.168.1.77");;

    // Connect the socket to the right server
    int connection = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (connection == 0) {
        printf("===Connection established===\n");
    }

    // Send a message over the socket
    if( send(sock, &test_message , sizeof(test_message), 0) < 0)
    {
        printf("Send failed\n");
        return 1;
    }
    printf("Send completed\n");

    // Read in from the server
    char server_response[256];
    recv(sock, &server_response, sizeof(server_response), 0);
    for (int i = 0; i < 256; i++) {
        printf("%d", server_response[i]);
    }
    close(sock);

    return 0;
}
