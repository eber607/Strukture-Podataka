#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_NAME 64

typedef struct {
    char ime[MAX_NAME];
    char prezime[MAX_NAME];
    int bodovi;
} Student;

// Funkcija broji koliko redaka (studenata) ima u datoteci "studenti.txt".
// Vraća broj redaka ili -1 ako nije moguće otvoriti datoteku.
int brojRedakaUDatoteci(const char* studenti) {
    FILE* datoteka = fopen("studenti.txt", "r");
    if (!datoteka) {
        printf("Greska pri otvaranju datoteke!\n");
        return -1;
    }
    int broj = 0;
    char buffer[MAX_LINE];
    while (fgets(buffer, sizeof(buffer), datoteka) != NULL) {
        broj++;
    }
    fclose(datoteka);
    return broj;
}

// Funkcija učitava podatke o studentima iz datoteke u dinamički alocirano polje struktura Student.
// Vraća pokazivač na polje studenata ili NULL u slučaju greške.
Student* ucitajStudente(const char* nazivDatoteke, int brojStudenata) {
    FILE* datoteka = fopen(nazivDatoteke, "r");
    if (!datoteka) {
        printf("Greska pri otvaranju datoteke!\n");
        return NULL;
    }
    Student* studenti = (Student*)malloc(brojStudenata * sizeof(Student));
    if (!studenti) {
        printf("Greska pri alokaciji memorije!\n");
        fclose(datoteka);
        return NULL;
    }
    for (int i = 0; i < brojStudenata; ++i) {
        fscanf(datoteka, "%s %s %d", studenti[i].ime, studenti[i].prezime, &studenti[i].bodovi);
    }
    fclose(datoteka);
    return studenti;
}

// Funkcija pronalazi i vraća maksimalan broj bodova među svim studentima.
int maxBodovi(Student* studenti, int brojStudenata) {
    int max = 0;
    for (int i = 0; i < brojStudenata; ++i) {
        if (studenti[i].bodovi > max)
            max = studenti[i].bodovi;
    }
    return max;
}

// Funkcija ispisuje podatke o svim studentima, uključujući relativne bodove u odnosu na maksimalan broj bodova.
void ispisiStudente(Student* studenti, int brojStudenata, int maxBod) {
    printf("Ime\tPrezime\tBodovi\tRelativni bodovi\n");
    for (int i = 0; i < brojStudenata; ++i) {
        double relativni = (maxBod > 0) ? ((double)studenti[i].bodovi / maxBod) * 100.0 : 0.0;
        printf("%s\t%s\t%d\t%.2f%%\n", studenti[i].ime, studenti[i].prezime, studenti[i].bodovi, relativni);
    }
}

// Glavna funkcija programa. Učitava podatke iz datoteke, pronalazi maksimalan broj bodova i ispisuje sve studente.
int main() {
    const char* nazivDatoteke = "studenti.txt";
    int brojStudenata = brojRedakaUDatoteci(nazivDatoteke);
    if (brojStudenata <= 0)
        return 1;

    Student* studenti = ucitajStudente(nazivDatoteke, brojStudenata);
    if (!studenti)
        return 1;

    int maxBod = maxBodovi(studenti, brojStudenata);
    ispisiStudente(studenti, brojStudenata, maxBod);

    free(studenti);
    return 0;
}