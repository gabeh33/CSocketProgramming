#include <stdio.h>
#include <stdlib.h>
#define TOKEN_SIZE 128
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
#include <sodium.h>

struct message {
    int hacker_id; /* this is just the number part of the ID */
    int status;
    unsigned char nonce[crypto_secretbox_NONCEBYTES]; //crypto_secretbox_NONCEBYTES
    unsigned char payload[PAYLOAD_SIZE];
};

int main() {
    if (sodium_init() < 0) {
        /* panic! the library couldn't be initialized, it is not safe to use */
        printf("Unable to set up library, exiting...");
        return 1;
    }

    /*
     * #define TOKEN_SIZE 128
        #define PAYLOAD_SIZE crypto_secretbox_MACBYTES + TOKEN_SIZE
        #define MSG_ASK "Can I get the solution to the challenge, please?"
     */
    struct message my_message;
    my_message.hacker_id = 19;

    //Nonce
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);
    memcpy(my_message.nonce, nonce, crypto_secretbox_NONCEBYTES);

    //Message buffer
    unsigned char message_buffer[TOKEN_SIZE]; // Create the buffer
    strcpy(message_buffer, MSG_ASK);

    //Create the key
    FILE *fptr;
    unsigned char key[crypto_secretbox_KEYBYTES];
    fptr = fopen("key","rb");

    if (fptr == NULL) {
        printf("Error opening file");
        return 3;
    }

    fread(key, sizeof(key), sizeof(key), fptr);
    crypto_secretbox_easy(my_message.payload, message_buffer, TOKEN_SIZE, nonce, key);


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

    struct message server_response;

    // Send a message over the socket
    if(send(sock, (void*)&my_message , sizeof(struct message), 0) < 0)
    {
        printf("Send failed\n");
        return 1;
    }

    // Read in from the server
    recvfrom(sock, &server_response, sizeof(struct message), 0, (struct sockaddr *)&sock,
             (socklen_t *)sizeof(sock));

    unsigned char decrypted[TOKEN_SIZE];
    for (;;) {
        if (crypto_secretbox_open_easy(decrypted, server_response.payload,
                                       PAYLOAD_SIZE, server_response.nonce, key) != 0) {
            //printf("Failure\n");
        } else{
            //printf("Success\n");
            break;
        }

        // Send a message over the socket
        if(send(sock, (void*)&my_message , sizeof(struct message), 0) < 0)
        {
            printf("Send failed\n");
            return 1;
        }

        // Read in from the server
        recvfrom(sock, &server_response, sizeof(struct message), 0, (struct sockaddr *)&sock,
                 (socklen_t *)sizeof(sock));
        //printf("Server response: %s\n", server_response.payload);


    }

    //Decrypt
    unsigned char hash[crypto_generichash_BYTES];
    crypto_generichash(hash, sizeof hash,
                       decrypted, sizeof(decrypted),
                       NULL, 0);

    unsigned char b64[128];
    int x = sodium_base64_ENCODED_LEN(sizeof(hash), sodium_base64_VARIANT_ORIGINAL);

    sodium_bin2base64(&b64, x, &hash, sizeof(hash), sodium_base64_VARIANT_ORIGINAL);
    for(int i = 0; i < sizeof(b64); i++) {
        printf("%c", b64[i]);
    }
    printf("\n");
    close(sock);
    //printf("Socket Closed\n");
    return 0;
}
