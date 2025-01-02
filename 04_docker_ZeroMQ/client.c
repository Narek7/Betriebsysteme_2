#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MESSAGE_SIZE 1
#define MESSAGE_COUNT 100000

int main() {
    // ZeroMQ-Kontext erstellen
    void *context = zmq_ctx_new();
    // Client-Socket erstellen und mit Server verbinden
    void *socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, "tcp://zmq-server:5555");

    char message[MESSAGE_SIZE] = {0};
    struct timespec start, end;

    // Zeitmessung starten
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < MESSAGE_COUNT; i++) {
        zmq_send(socket, message, MESSAGE_SIZE, 0); // Nachricht senden
        zmq_recv(socket, message, MESSAGE_SIZE, 0); // Antwort empfangen
    }

    // Zeitmessung beenden
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Latenz berechnen
    double elapsed = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;
    double average_latency = elapsed / MESSAGE_COUNT;

    printf("Average latency (tcp): %.2f µs\n", average_latency);

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
