#include <kos.h>

static int running = 1;
void exit_callback() { running = 0; }

int main(int argc, char **argv) {
    cont_btn_callback(0, CONT_START | CONT_A, (cont_btn_callback_t)exit_callback);
    dbgio_init();
    dbgio_dev_select("fb");
    printf("Witaj na Dreamcast!\n");
    printf("Ta gra zostala skompilowana w calosci w chmurze GitHub!\n");
    printf("Nacisnij START + A aby wyjsc.\n");
    while(running) { thd_sleep(100); }
    return 0;
}
