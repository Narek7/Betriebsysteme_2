#include "/opt/homebrew/Cellar/zeromq/4.3.5_1/include/zmq.h" // ZeroMQ-Header einbinden
#include <pthread.h>   // POSIX-Threads zur parallelen Ausführung
#include <stdio.h>     // Standard-Ein-/Ausgabe
#include <stdlib.h>    // Standard-Bibliotheksfunktionen
#include <time.h>      // Zeitmessung
#include <unistd.h>    // Systemaufrufe

// Definiere die Nachrichten-Größe und die Anzahl der Nachrichten
#define MESSAGE_SIZE 1      // Nachrichten-Größe in Bytes
#define MESSAGE_COUNT 100000 // Anzahl der zu sendenden Nachrichten

// Server-Thread-Funktion
// Diese Funktion wird in einem separaten Thread ausgeführt und fungiert als "Server" innerhalb des gleichen Prozesses
void *server_thread(void *context) {
    // Erstelle einen ZeroMQ-Socket im PAIR-Modus
    void *socket = zmq_socket(context, ZMQ_PAIR);

    // Binde den Socket an eine In-Process-URL (nur innerhalb eines Prozesses nutzbar)
    zmq_bind(socket, "inproc://test");

    // Verarbeite MESSAGE_COUNT Nachrichten
    for (int i = 0; i < MESSAGE_COUNT; ++i) {
        char buffer[MESSAGE_SIZE]; // Puffer für empfangene Daten

        // Empfange eine Nachricht vom Client
        zmq_recv(socket, buffer, MESSAGE_SIZE, 0);

        // Sende die Nachricht zurück an den Client (Echo-Mechanismus)
        zmq_send(socket, buffer, MESSAGE_SIZE, 0);
    }

    // Schließe den Socket, wenn die Arbeit beendet ist
    zmq_close(socket);
    return NULL;
}

// Client-Thread-Funktion
// Diese Funktion wird in einem separaten Thread ausgeführt und fungiert als "Client" innerhalb des gleichen Prozesses
void *client_thread(void *context) {
    // Erstelle einen ZeroMQ-Socket im PAIR-Modus
    void *socket = zmq_socket(context, ZMQ_PAIR);

    // Verbinde den Socket mit der In-Process-URL des Servers
    zmq_connect(socket, "inproc://test");

    char message[MESSAGE_SIZE] = {0}; // Nachricht, die gesendet wird

    struct timespec start, end; // Variablen zur Zeitmessung

    // Starte die Zeitmessung
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Wiederhole die Nachrichtensendung MESSAGE_COUNT Mal
    for (int i = 0; i < MESSAGE_COUNT; ++i) {
        // Sende die Nachricht an den Server
        zmq_send(socket, message, MESSAGE_SIZE, 0);

        // Empfange die Antwort vom Server
        zmq_recv(socket, message, MESSAGE_SIZE, 0);
    }

    // Beende die Zeitmessung
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Berechne die Gesamtdauer in Mikrosekunden
    double elapsed = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;

    // Berechne die durchschnittliche Latenz pro Nachricht in Mikrosekunden
    double average_latency = elapsed / MESSAGE_COUNT;

    // Gebe die durchschnittliche Latenz aus
    printf("Average latency (inproc): %.2f µs\n", average_latency);

    // Schließe den Socket
    zmq_close(socket);
    return NULL;
}

// Hauptprogramm
int main() {
    // Erstelle einen ZeroMQ-Kontext
    void *context = zmq_ctx_new();

    pthread_t server, client; // Thread-Handles für Server und Client

    // Starte den Server-Thread
    pthread_create(&server, NULL, server_thread, context);

    // Starte den Client-Thread
    pthread_create(&client, NULL, client_thread, context);

    // Warte auf die Beendigung des Client-Threads
    pthread_join(client, NULL);

    // Beende den ZeroMQ-Kontext
    zmq_ctx_destroy(context);

    return 0;
}



// -----------------------------------------------------------------------------
// Kompilieren und Ausführen mit:
// gcc -I/opt/homebrew/Cellar/zeromq/4.3.5_1/include \
   -L/opt/homebrew/Cellar/zeromq/4.3.5_1/lib \
   -o inproc_latency inproc_latency.c -lzmq -lpthread && ./inproc_latency
// -----------------------------------------------------------------------------
