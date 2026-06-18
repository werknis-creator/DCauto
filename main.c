#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/kgl.h>
#include <math.h>

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
    glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float fovy = 45.0f * 3.14159f / 180.0f;
    float f = 1.0f / tanf(fovy / 2.0f);
    float m[16] = {
        f / (640.0f/480.0f), 0, 0, 0,
        0, f, 0, 0,
        0, 0, -101.0f / 99.0f, -1.0f,
        0, 0, -2.0f * 100.0f / 99.0f, 0
    };
    glLoadMatrixf(m);
    
    float px = 0, py = 0, pz = 0, yaw = 0, vy = 0;
    int on_ground = 1;
    
    while(1) {
        maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        if(cont) {
            cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
            if(state) {
                float lx = (state->joyx - 128) / 128.0f;
                float ly = (state->joyy - 128) / 128.0f;
                if(fabsf(lx) < 0.2f) lx = 0;
                if(fabsf(ly) < 0.2f) ly = 0;
                
                yaw -= lx * 2.0f * 0.016f;
                px += sinf(yaw) * (-ly) * 5.0f * 0.016f;
                pz += cosf(yaw) * (-ly) * 5.0f * 0.016f;
                
                if((state->buttons & CONT_X) && on_ground) { vy = 8.0f; on_ground = 0; }
                if((state->buttons & CONT_START) && (state->buttons & CONT_A)) break;
            }
        }
        
        if(!on_ground) vy -= 20.0f * 0.016f;
        py += vy * 0.016f;
        if(py <= 0) { py = 0; vy = 0; on_ground = 1; }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glTranslatef(0, -4.0f, -8.0f);
        glRotatef(-yaw * 180.0f / 3.14159f, 0, 1, 0);
        glTranslatef(-px, -py, -pz);
        
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glVertex3f(-100, 0, -100); glVertex3f(100, 0, -100); 
        glVertex3f(100, 0, 100); glVertex3f(-100, 0, 100);
        glEnd();
        
        glColor3f(0.8f, 0.2f, 0.2f);
        draw_box(-20, 0, 20, 10, 15, 10);
        glColor3f(0.2f, 0.8f, 0.2f);
        draw_box(15, 0, -30, 12, 25, 12);
        
        glPushMatrix();
        glTranslatef(px, py, pz);
        glRotatef(yaw * 180.0f / 3.14159f, 0.0f, 1.0f, 0.0f);
        glColor3f(0.1f, 0.5f, 0.1f); 
        draw_box(-0.4f, 0.0f, -0.3f, 0.8f, 1.2f, 0.6f);
        glColor3f(0.8f, 0.6f, 0.4f); 
        draw_box(-0.25f, 1.2f, -0.25f, 0.5f, 0.5f, 0.5f);
        glPopMatrix();
        
        glKosSwapBuffers();
    }
    return 0;
}
