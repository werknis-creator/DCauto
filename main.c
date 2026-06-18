#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <kos/timer.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glkos.h>
#include <math.h>

#define M_PI 3.14159265358979323846

/* Funkcja pomocnicza do rysowania 3D (np. budynkow i postaci) */
void draw_box(float x, float y, float z, float w, float h, float d) {
    glBegin(GL_QUADS);
    // Przod
    glVertex3f(x, y, z+d); glVertex3f(x+w, y, z+d); glVertex3f(x+w, y+h, z+d); glVertex3f(x, y+h, z+d);
    // Tyl
    glVertex3f(x, y, z); glVertex3f(x, y+h, z); glVertex3f(x+w, y+h, z); glVertex3f(x+w, y, z);
    // Lewy bok
    glVertex3f(x, y, z); glVertex3f(x, y, z+d); glVertex3f(x, y+h, z+d); glVertex3f(x, y+h, z);
    // Prawy bok
    glVertex3f(x+w, y, z); glVertex3f(x+w, y+h, z); glVertex3f(x+w, y+h, z+d); glVertex3f(x+w, y, z+d);
    // Gora
    glVertex3f(x, y+h, z); glVertex3f(x, y+h, z+d); glVertex3f(x+w, y+h, z+d); glVertex3f(x+w, y+h, z);
    // Dol
    glVertex3f(x, y, z); glVertex3f(x+w, y, z); glVertex3f(x+w, y, z+d); glVertex3f(x, y, z+d);
    glEnd();
}

int main(int argc, char **argv) {
    // Inicjalizacja OpenGL na Dreamcast
    glKosInit();
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Niebo Los Santos
    glEnable(GL_DEPTH_TEST);
    
    // Zmienne gracza i fizyki
    float player_x = 0.0f, player_y = 0.0f, player_z = 0.0f;
    float player_yaw = 0.0f; // Obrot w stopniach
    float vel_y = 0.0f;      // Predkosc pionowa (do skakania)
    int is_on_ground = 1;
    
    uint32 secs, msecs;
    timer_ms_gettime(&secs, &msecs);
    uint64 last_time = (uint64)secs * 1000 + msecs;
    
    // Glowna petla gry
    while(1) {
        // Obliczanie czasu miedzy klatkami (dt)
        timer_ms_gettime(&secs, &msecs);
        uint64 current_time = (uint64)secs * 1000 + msecs;
        float dt = (current_time - last_time) / 1000.0f;
        if(dt > 0.1f) dt = 0.1f; // Zabezpieczenie przed "skokami" czasu
        last_time = current_time;
        
        // Odczyt pada
        maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        if(cont) {
            cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
            if(state) {
                // Lewy Analog (osie X i Y, srodek to 128)
                float lx = (state->joyx - 128) / 128.0f;
                float ly = (state->joyy - 128) / 128.0f;
                
                // Deadzone (martwa strefa)
                if(fabs(lx) < 0.2f) lx = 0.0f;
                if(fabs(ly) < 0.2f) ly = 0.0f;
                
                // Obracanie sie (Lewy Analog X)
                player_yaw -= lx * 120.0f * dt;
                
                // Chodzenie (Lewy Analog Y)
                float rad = player_yaw * M_PI / 180.0f;
                float speed = 5.0f;
                player_x += sin(rad) * (-ly) * speed * dt;
                player_z += cos(rad) * (-ly) * speed * dt;
                
                // Skakanie (Przycisk X)
                if((state->buttons & CONT_X) && is_on_ground) {
                    vel_y = 8.0f;
                    is_on_ground = 0;
                }
                
                // Wyjscie z gry (START + A)
                if((state->buttons & CONT_START) && (state->buttons & CONT_A)) {
                    break; 
                }
            }
        }
        
        // Fizyka grawitacji
        if(!is_on_ground) {
            vel_y -= 20.0f * dt;
        }
        player_y += vel_y * dt;
        if(player_y <= 0.0f) {
            player_y = 0.0f;
            vel_y = 0.0f;
            is_on_ground = 1;
        }
        
        // Renderowanie
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        
        // Kamera w 3. osobie (TPS)
        float cam_dist = 8.0f;
        float cam_height = 4.0f;
        float rad = player_yaw * M_PI / 180.0f;
        float cam_x = player_x - sin(rad) * cam_dist;
        float cam_z = player_z - cos(rad) * cam_dist;
        float cam_y = player_y + cam_height;
        
        gluLookAt(cam_x, cam_y, cam_z, player_x, player_y + 1.5f, player_z, 0.0f, 1.0f, 0.0f);
        
        // Podloga (Asfalt)
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glVertex3f(-100, 0, -100);
        glVertex3f(100, 0, -100);
        glVertex3f(100, 0, 100);
        glVertex3f(-100, 0, 100);
        glEnd();
        
        // Budynki w tle (zeby bylo widac, ze sie poruszasz)
        glColor3f(0.6f, 0.5f, 0.4f);
        draw_box(-20, 0, 20, 10, 15, 10);
        draw_box(15, 0, -30, 12, 25, 12);
        draw_box(30, 0, 10, 8, 10, 8);
        
        // Gracz (CJ)
        glPushMatrix();
        glTranslatef(player_x, player_y, player_z);
        glRotatef(player_yaw, 0.0f, 1.0f, 0.0f);
        glColor3f(0.1f
