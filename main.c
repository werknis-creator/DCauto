#include <kos.h>

/* Zmienna kontrolująca pętlę gry */
static int running = 1;

/* Funkcja wywoływana po wciśnięciu kombinacji klawiszy wyjścia */
void exit_callback() {
    running = 0;
}

int main(int argc, char **argv) {
    // Przypisanie wyjścia z programu pod jednoczesne wciśnięcie START + A
    cont_btn_callback(0, CONT_START | CONT_A, (cont_btn_callback_t)exit_callback);
    
    // Inicjalizacja wypisywania tekstu na ekranie (framebuffer)
    dbgio_init();
    dbgio_dev_select("fb");
    
    printf("Witaj na Dreamcast!\n");
    printf("Ta gra zostala skompilowana w calosci w chmurze GitHub!\n");
    printf("Nacisnij START + A aby wyjsc.\n");
    
    // Główna pętla gry
    while(running) {
        thd_sleep(100); // Krótkie uśpienie wątku
    }
    
    return 0;
}
