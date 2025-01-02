#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MESSAGE_SIZE 1
#define MESSAGE_COUNT 100000

int main() {
    // ZeroMQ-Kontext erstellen
    void *context = zmq_ctx_new();
    // Server-Socket erstellen und binden
    void *socket = zmq_socket(context, ZMQ_PAIR);
    zmq_bind(socket, "tcp://*:5555");

    for (int i = 0; i < MESSAGE_COUNT; i++) {
        char buffer[MESSAGE_SIZE];
        zmq_recv(socket, buffer, MESSAGE_SIZE, 0); // Nachricht empfangen
        zmq_send(socket, buffer, MESSAGE_SIZE, 0); // Nachricht zurücksenden
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
