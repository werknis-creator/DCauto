#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/kgl.h>
#include <math.h>

#define M_PI 3.14159265358979323846

void draw_box(float x, float y, float z, float w, float h, float d) {
    glBegin(GL_QUADS);
    glVertex3f(x, y, z+d); glVertex3f(x+w, y, z+d); glVertex3f(x+w, y+h, z+d); glVertex3f(x, y+h, z+d);
    glVertex3f(x, y, z); glVertex3f(x, y+h, z); glVertex3f(x+w, y+h, z); glVertex3f(x+w, y, z);
    glVertex3f(x, y, z); glVertex3f(x, y, z+d); glVertex3f(x, y+h, z+d); glVertex3f(x, y+h, z);
    glVertex3f(x+w, y, z); glVertex3f(x+w, y+h, z); glVertex3f(x+w, y+h, z+d); glVertex3f(x+w, y, z+d);
    glVertex3f(x, y+h, z); glVertex3f(x, y+h, z+d); glVertex3f(x+w, y+h, z+d); glVertex3f(x+w, y+h, z);
    glVertex3f(x, y, z); glVertex3f(x+w, y, z); glVertex3f(x+w, y, z+d); glVertex3f(x, y, z+d);
    glEnd();
}

int main(int argc, char **argv) {
    glKosInit();
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Ręczne ustawienie perspektywy (odpowiednik gluPerspective, nie wymaga GLU)
    float fovy = 45.0f * M_PI / 180.0f;
    float f = 1.0f / tan(fovy / 2.0f);
    float m[16] = {
        f / (640.0f/480.0f), 0, 0, 0,
        0, f, 0, 0,
        0, 0, (100.0f + 0.1f) / (0.1f - 100.0f), -1.0f,
        0, 0, (2.0f * 100.0f * 0.1f) / (0.1f - 100.0f), 0
    };
    glLoadMatrixf(m);
    
    float player_x = 0.0f, player_y = 0.0f, player_z = 0.0f;
    float player_yaw = 0.0f;
    float vel_y = 0.0f;
    int is_on_ground = 1;
    
    uint32 secs, msecs;
    timer_ms_gettime(&secs, &msecs);
    uint64 last_time = (uint64)secs * 1000 + msecs;
    
    while(1) {
        timer_ms_gettime(&secs, &msecs);
        uint64 current_time = (uint64)secs * 1000 + msecs;
        float dt = (current_time - last_time) / 1000.0f;
        if(dt > 0.1f) dt = 0.1f;
        last_time = current_time;
        
        maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        if(cont) {
            cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
            if(state) {
                float lx = (state->joyx - 128) / 128.0f;
                float ly = (state->joyy - 128) / 128.0f;
                
                if(fabs(lx) < 0.2f) lx = 0.0f;
                if(fabs(ly) < 0.2f) ly = 0.0f;
                
                player_yaw -= lx * 120.0f * dt;
                
                float rad = player_yaw * M_PI / 180.0f;
                float speed = 5.0f;
                player_x += sin(rad) * (-ly) * speed * dt;
                player_z += cos(rad) * (-ly) * speed * dt;
                
                if((state->buttons & CONT_X) && is_on_ground) {
                    vel_y = 8.0f;
                    is_on_ground = 0;
                }
                
                if((state->buttons & CONT_START) && (state->buttons & CONT_A)) break; 
            }
        }
        
        if(!is_on_ground) vel_y -= 20.0f * dt;
        player_y += vel_y * dt;
        if(player_y <= 0.0f) {
            player_y = 0.0f;
            vel_y = 0.0f;
            is_on_ground = 1;
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Kamera TPP (Za plecami gracza)
        float cam_dist = 8.0f;
        float cam_height = 4.0f;
        glTranslatef(0, -cam_height, -cam_dist);
        glRotatef(-player_yaw, 0, 1, 0);
        glTranslatef(-player_x, -player_y, -player_z);
        
        // Podloga
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glVertex3f(-100, 0, -100); glVertex3f(100, 0, -100); 
        glVertex3f(100, 0, 100); glVertex3f(-100, 0, 100);
        glEnd();
        
        // Budynki
        glColor3f(0.6f, 0.5f, 0.4f);
        draw_box(-20, 0, 20, 10, 15, 10);
        draw_box(15, 0, -30, 12, 25, 12);
        draw_box(30, 0, 10, 8, 10, 8);
        
        // Gracz (CJ)
        glPushMatrix();
        glTranslatef(player_x, player_y, player_z);
        glRotatef(player_yaw, 0.0f, 1.0f, 0.0f);
        glColor3f(0.1f, 0.5f, 0.1f); 
        draw_box(-0.4f, 0.0f, -0.3f, 0.8f, 1.2f, 0.6f);
        glColor3f(0.8f, 0.6f, 0.4f); 
        draw_box(-0.25f, 1.2f, -0.25f, 0.5f, 0.5f, 0.5f);
        glPopMatrix();
        
        glKosSwapBuffers();
    }
    return 0;
}
