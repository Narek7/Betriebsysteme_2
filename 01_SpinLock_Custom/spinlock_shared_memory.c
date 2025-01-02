#include <stdatomic.h>
#include <fcntl.h>    // Für shm_open
#include <sys/mman.h> // Für mmap
#include <stdio.h>    // Für printf
#include <stdlib.h>   // Für exit
#include <unistd.h>   // Für fork
#include <time.h>     // Für clock_gettime

#define SHM_NAME "/shared_spinlock" // Name des Shared Memory Segments
#define SHM_SIZE sizeof(atomic_flag) // Größe des Shared Memory Segments

// Spinlock-Methoden
void spin_lock(atomic_flag *lock) {
    while (atomic_flag_test_and_set(lock)); // Busy Waiting
}

void spin_unlock(atomic_flag *lock) {
    atomic_flag_clear(lock); // Lock freigeben
}

void measure_latency(atomic_flag *lock) {
    struct timespec start, end;
    int iterations = 1000000; // Anzahl der Iterationen

    // Zeitmessung starten
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iterations; i++) {
        spin_lock(lock);   // Lock setzen
        spin_unlock(lock); // Lock freigeben
    }

    // Zeitmessung stoppen
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Gesamtzeit berechnen
    double total_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    double avg_time = total_time / iterations;

    printf("Durchschnittliche Latenz pro Spinlock-Operation: %.2f ns\n", avg_time);
}

int main() {
    // Shared Memory erstellen
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Größe festlegen
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Shared Memory mappen
    void *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Spinlock initialisieren
    atomic_flag *lock = (atomic_flag *)shm_ptr;
    atomic_flag_clear(lock);

    // Kindprozess erstellen
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // Kindprozess
        measure_latency(lock);
        exit(0);
    } else {
        // Elternprozess
        measure_latency(lock);

        // Warten auf Kindprozess
        wait(NULL);

        // Shared Memory löschen
        shm_unlink(SHM_NAME);
    }

    return 0;
}


// kompilieren und ausführen mit:
// gcc -o spinlock_shared_memory spinlock_shared_memory.c -lpthread && ./spinlock_shared_memory