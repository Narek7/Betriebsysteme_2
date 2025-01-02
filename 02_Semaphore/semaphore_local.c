#include <semaphore.h> // Bibliothek für Semaphore-Funktionen (z.B. sem_open, sem_wait, sem_post)
#include <stdio.h>     // Für printf-Ausgaben
#include <pthread.h>   // Für Thread-Funktionen
#include <stdlib.h>    // Für Standardfunktionen wie exit
#include <fcntl.h>     // Für O_CREAT, um ein benanntes Semaphore zu erstellen

#define SEM_NAME "/local_semaphore" // Name des Semaphores (lokal für Threads im selben Prozess)

// Globale Semaphore
sem_t *semaphore; // Semaphore, das von beiden Threads verwendet wird

// Funktion, die von beiden Threads ausgeführt wird
void *thread_func(void *arg) {
    struct timespec start, end; // Variablen zur Zeitmessung
    int iterations = 1000000;   // Anzahl der Iterationen (kann angepasst werden)

    // Startzeit erfassen
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iterations; i++) {
        sem_wait(semaphore);   // Warten auf das Semaphore (verringert den Zähler)
        sem_post(semaphore);   // Semaphore freigeben (erhöht den Zähler)
    }

    // Endzeit erfassen
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Gesamtzeit in Nanosekunden berechnen
    double total_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    double avg_time = total_time / iterations; // Durchschnittliche Latenzzeit pro Operation

    // Ergebnis ausgeben
    printf("Durchschnittliche Latenz pro Semaphore-Operation: %.2f ns\n", avg_time);

    return NULL; // Thread beendet
}

int main() {
    pthread_t t1, t2; // Thread-Handles für zwei Threads

    // Semaphore initialisieren
    semaphore = sem_open(SEM_NAME, O_CREAT, 0666, 1); // Startwert 1 (binäres Semaphore)
    if (semaphore == SEM_FAILED) { // Fehlerprüfung
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Zwei Threads erstellen
    pthread_create(&t1, NULL, thread_func, NULL); // Erster Thread
    pthread_create(&t2, NULL, thread_func, NULL); // Zweiter Thread

    // Auf beide Threads warten
    pthread_join(t1, NULL); // Hauptthread wartet auf den ersten Thread
    pthread_join(t2, NULL); // Hauptthread wartet auf den zweiten Thread

    // Semaphore schließen und löschen
    sem_close(semaphore);   // Semaphore schließen (Ressourcen freigeben)
    sem_unlink(SEM_NAME);   // Benanntes Semaphore aus dem System entfernen

    return 0; // Hauptprogramm beendet
}

//Kompilierung und Ausführung
//gcc -o semaphore_local semaphore_local.c -lpthread && ./semaphore_local