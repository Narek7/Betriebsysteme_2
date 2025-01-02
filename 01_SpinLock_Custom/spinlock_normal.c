#include <stdatomic.h> // Enthält Definitionen für atomare Variablentypen und Funktionen (z.B. atomic_flag)
#include <stdio.h>     // Für printf-Ausgaben
#include <stdlib.h>    // Für exit, falls Fehlerbehandlung erforderlich wäre
#include <time.h>      // Für clock_gettime, um die Zeit präzise zu messen
#include <pthread.h>   // Für Thread-Verwaltung (pthread_create, pthread_join)

// -----------------------------------------------------------------------------
// Globale Variable: "lock" als Spinlock über ein atomares Flag
// -----------------------------------------------------------------------------
atomic_flag lock = ATOMIC_FLAG_INIT; 
/*
 * 'atomic_flag' ist ein spezieller Typ für atomare 
 * Test-und-Set-Operationen. ATOMIC_FLAG_INIT initialisiert 
 * das Flag auf "nicht gesetzt", d.h. der Lock ist anfangs frei.
 */

// -----------------------------------------------------------------------------
// Funktion: spin_lock
// -----------------------------------------------------------------------------
void spin_lock(atomic_flag *lock) {
    // Solange das Flag bereits gesetzt ist, wird in der Schleife "busy gewartet".
    // atomic_flag_test_and_set(lock) setzt das Flag und gibt den alten Wert zurück.
    // - War das Flag vorher 'false' (= nicht gesetzt), wird es nun auf 'true' gesetzt,
    //   und der Aufruf gibt 'false' zurück -> Lock wurde erfolgreich erworben.
    // - War das Flag vorher 'true' (= schon gesetzt), wird erneut 'true' zurückgegeben,
    //   und wir warten weiter in der while-Schleife.
    while (atomic_flag_test_and_set(lock))
        ; // Busy-Wait (aktives Warten)
}

// -----------------------------------------------------------------------------
// Funktion: spin_unlock
// -----------------------------------------------------------------------------
void spin_unlock(atomic_flag *lock) {
    // Gibt den Lock frei, indem das Flag auf 'false' zurückgesetzt wird.
    // Andere Threads können das Flag nun setzen (Lock erwerben).
    atomic_flag_clear(lock);
}

// -----------------------------------------------------------------------------
// Thread-Funktion zur Messung der Spinlock-Latenz
// -----------------------------------------------------------------------------
void *thread_func(void *arg) {
    struct timespec start, end;
    int iterations = 1000000; // Anzahl, wie oft Lock gesetzt/gelöst wird

    // Startzeit ermitteln (CLOCK_MONOTONIC ist monoton steigend, ideal für Latenzmessung)
    clock_gettime(CLOCK_MONOTONIC, &start);

    // In einer Schleife wiederholt Lock setzen und freigeben
    for (int i = 0; i < iterations; i++) {
        spin_lock(&lock);   // Lock akquirieren
        spin_unlock(&lock); // Lock wieder freigeben
    }

    // Endzeit ermitteln
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Latenz berechnen
    // Differenz in Nanosekunden: 
    // (Sekundendifferenz * 1e9) + (Nanosekundendifferenz)
    double total_time = (end.tv_sec - start.tv_sec) * 1e9 
                      + (end.tv_nsec - start.tv_nsec);
    // Durchschnittliche Zeit pro Lock/Unlock
    double avg_time = total_time / iterations;

    // Ergebnis ausgeben (Durchschnittliche Zeit pro Spinlock-Operation in ns)
    printf("Durchschnittliche Latenz pro Spinlock-Operation: %.2f ns\n", avg_time);

    return NULL;
}

// -----------------------------------------------------------------------------
// Hauptfunktion
// -----------------------------------------------------------------------------
int main() {
    pthread_t t1, t2;

    // Es werden zwei Threads erzeugt, die beide die Latenz des Spinlocks messen.
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);

    // Hier warten wir auf das Ende beider Threads
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}

// -----------------------------------------------------------------------------
// Kompilieren und Ausführen (Linux und MacOS) mit:
// gcc -o spinlock_normal SPINLOCK_NORMAL.C -lpthread && ./spinlock_normal
// -----------------------------------------------------------------------------
