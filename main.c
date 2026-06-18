#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <GL/gl.h>
#include <GL/glkos.h>
#include <math.h>

#define PI 3.14159265f

void draw_box(float x, float y, float z, float w, float h, float d, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(x, y, z + d); glVertex3f(x + w, y, z + d); glVertex3f(x + w, y + h, z + d); glVertex3f(x, y + h, z + d);
    glVertex3f(x + w, y, z); glVertex3f(x, y, z); glVertex3f(x, y + h, z); glVertex3f(x + w, y + h, z);
    glVertex3f(x, y, z); glVertex3f(x, y, z + d); glVertex3f(x, y + h, z + d); glVertex3f(x, y + h, z);
    glVertex3f(x + w, y, z + d); glVertex3f(x + w, y, z); glVertex3f(x + w, y + h, z); glVertex3f(x + w, y + h, z + d);
    glVertex3f(x, y + h, z + d); glVertex3f(x + w, y + h, z + d); glVertex3f(x + w, y + h, z); glVertex3f(x, y + h, z);
    glVertex3f(x, y, z); glVertex3f(x + w, y, z); glVertex3f(x + w, y, z + d); glVertex3f(x, y, z + d);
    glEnd();
}

int main(int argc, char **argv) {
    glKosInit();
    glClearColor(0.4f, 0.7f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float fovy = 60.0f * PI / 180.0f;
    float f = 1.0f / tanf(fovy / 2.0f);
    float aspect = 640.0f / 480.0f;
    float zNear = 0.1f, zFar = 1000.0f;
    
    float m[16] = {
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f,
        0.0f, 0.0f, (2.0f * zFar * zNear) / (zNear - zFar), 0.0f
    };
    glLoadMatrixf(m);

    float px = 0.0f, py = 0.0f, pz = 0.0f;
    float yaw = 0.0f, vy = 0.0f;
    int on_ground = 1;
    
    while(1) {
        maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        if(cont) {
            cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
            if(state) {
                float lx = state->joyx / 128.0f;
                float ly = state->joyy / 128.0f;
                if(fabsf(lx) < 0.2f) lx = 0.0f;
                if(fabsf(ly) < 0.2f) ly = 0.0f;
                
                yaw -= lx * 2.5f * 0.016f; 
                px += sinf(yaw) * (-ly) * 6.0f * 0.016f;
                pz += cosf(yaw) * (-ly) * 6.0f * 0.016f;
                
                if((state->buttons & CONT_X) && on_ground) { vy = 8.0f; on_ground = 0; }
                if((state->buttons & CONT_START) && (state->buttons & CONT_A)) break;
            }
        }
        
        if(!on_ground) vy -= 20.0f * 0.016f; 
        py += vy * 0.016f;
        if(py <= 0.0f) { py = 0.0f; vy = 0.0f; on_ground = 1; }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glTranslatef(0.0f, -4.0f, -8.0f);
        glRotatef(-yaw * 180.0f / PI, 0.0f, 1.0f, 0.0f);
        glTranslatef(-px, -py, -pz);
        
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glVertex3f(-100.0f, 0.0f, -100.0f); glVertex3f( 100.0f, 0.0f, -100.0f);
        glVertex3f( 100.0f, 0.0f,  100.0f); glVertex3f(-100.0f, 0.0f,  100.0f);
        glEnd();
        
        draw_box(-20.0f, 0.0f,  20.0f, 10.0f, 15.0f, 10.0f, 0.8f, 0.2f, 0.2f);
        draw_box( 15.0f, 0.0f, -30.0f, 12.0f, 25.0f, 12.0f, 0.2f, 0.8f, 0.2f);
        draw_box( 30.0f, 0.0f,  10.0f,  8.0f, 10.0f,  8.0f, 0.2f, 0.2f, 0.8f);
        
        glPushMatrix();
        glTranslatef(px, py, pz);
        glRotatef(yaw * 180.0f / PI, 0.0f, 1.0f, 0.0f);
        draw_box(-0.4f, 0.0f, -0.3f, 0.8f, 1.2f, 0.6f, 0.1f, 0.5f, 0.1f);
        draw_box(-0.25f, 1.2f, -0.25f, 0.5f, 0.5f, 0.5f, 0.8f, 0.6f, 0.4f);
        glPopMatrix();
        
        glKosSwapBuffers();
    }
    return 0;
}
