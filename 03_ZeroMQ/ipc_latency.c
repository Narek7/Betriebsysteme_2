#include "/opt/homebrew/Cellar/zeromq/4.3.5_1/include/zmq.h" // ZeroMQ-Header einbinden
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // Für fork()

#define MESSAGE_SIZE 1      // Nachrichten-Größe in Bytes
#define MESSAGE_COUNT 100000 // Anzahl der zu sendenden Nachrichten

void server_process() {
    // Erstelle einen ZeroMQ-Kontext und einen PAIR-Socket
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_PAIR);

    // Binde den Socket an eine IPC-Adresse (für Prozesse auf demselben Host)
    zmq_bind(socket, "ipc:///tmp/test");

    for (int i = 0; i < MESSAGE_COUNT; ++i) {
        char buffer[MESSAGE_SIZE]; // Puffer für empfangene Daten
        zmq_recv(socket, buffer, MESSAGE_SIZE, 0); // Empfange Nachricht
        zmq_send(socket, buffer, MESSAGE_SIZE, 0); // Sende zurück
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
}

void client_process() {
    // Erstelle einen ZeroMQ-Kontext und einen PAIR-Socket
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_PAIR);

    // Verbinde den Socket mit der IPC-Adresse des Servers
    zmq_connect(socket, "ipc:///tmp/test");

    char message[MESSAGE_SIZE] = {0}; // Nachricht, die gesendet wird
    struct timespec start, end;

    // Starte die Zeitmessung
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < MESSAGE_COUNT; ++i) {
        zmq_send(socket, message, MESSAGE_SIZE, 0); // Sende Nachricht
        zmq_recv(socket, message, MESSAGE_SIZE, 0); // Empfange Antwort
    }

    // Beende die Zeitmessung
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Berechne die Gesamtdauer in Mikrosekunden
    double elapsed = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;
    double average_latency = elapsed / MESSAGE_COUNT;

    printf("Average latency (ipc): %.2f µs\n", average_latency);

    zmq_close(socket);
    zmq_ctx_destroy(context);
}

int main() {
    pid_t pid = fork(); // Erstelle einen neuen Prozess

    if (pid == 0) {
        // Kindprozess: Server
        server_process();
    } else {
        // Elternprozess: Client
        sleep(1); // Warten, bis der Server bereit ist
        client_process();
    }

    return 0;
}

// kompilieren und ausführen mit:
// gcc -I/opt/homebrew/Cellar/zeromq/4.3.5_1/include \
    -L/opt/homebrew/Cellar/zeromq/4.3.5_1/lib \
    -o ipc_latency ipc_latency.c -lzmq && ./ipc_latency
// -----------------------------------------------------------------------------
