#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/video.h>
#include <math.h>
#include <string.h>

#define W 640
#define H 480

void draw_pixel(int x, int y, uint16 color) {
    if(x >= 0 && x < W && y >= 0 && y < H) {
        vram_s[y * W + x] = color;
    }
}

void draw_line(int x1, int y1, int x2, int y2, uint16 color) {
    int dx = abs(x2-x1), sx = x1<x2 ? 1 : -1;
    int dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1; 
    int err = dx+dy, e2;
    while(1) {
        draw_pixel(x1, y1, color);
        if (x1==x2 && y1==y2) break;
        e2 = 2*err;
        if (e2 > dy) { err += dy; x1 += sx; }
        else if (e2 < dx) { err += dx; y1 += sy; }
    }
}

int main(int argc, char **argv) {
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
        
        memset(vram_s, 0, W * H * 2); // Czyszczenie ekranu
        
        float cos_y = cosf(yaw), sin_y = sinf(yaw);
        float cam_y = py + 5.0f;
        
        // Rysowanie podlogi 3D (Siatka)
        for(int i = -100; i <= 100; i += 10) {
            float x1 = i - px, z1 = -100 - pz;
            float x2 = i - px, z2 = 100 - pz;
            float rx1 = x1 * cos_y - z1 * sin_y;
            float rz1 = x1 * sin_y + z1 * cos_y;
            float rx2 = x2 * cos_y - z2 * sin_y;
            float rz2 = x2 * sin_y + z2 * cos_y;
            
            if(rz1 > 1 && rz2 > 1) {
                int sx1 = (rx1 * 400.0f / rz1) + W/2;
                int sy1 = ((0 - cam_y) * 400.0f / rz1) + H/2;
                int sx2 = (rx2 * 400.0f / rz2) + W/2;
                int sy2 = ((0 - cam_y) * 400.0f / rz2) + H/2;
                draw_line(sx1, sy1, sx2, sy2, 0x7BEF);
            }
            
            x1 = -100 - px; z1 = i - pz;
            x2 = 100 - px; z2 = i - pz;
            rx1 = x1 * cos_y - z1 * sin_y;
            rz1 = x1 * sin_y + z1 * cos_y;
            rx2 = x2 * cos_y - z2 * sin_y;
            rz2 = x2 * sin_y + z2 * cos_y;
            
            if(rz1 > 1 && rz2 > 1) {
                int sx1 = (rx1 * 400.0f / rz1) + W/2;
                int sy1 = ((0 - cam_y) * 400.0f / rz1) + H/2;
                int sx2 = (rx2 * 400.0f / rz2) + W/2;
                int sy2 = ((0 - cam_y) * 400.0f / rz2) + H/2;
                draw_line(sx1, sy1, sx2, sy2, 0x7BEF);
            }
        }
        
        // Celownik
        draw_line(W/2 - 10, H/2, W/2 + 10, H/2, 0xFFFF);
        draw_line(W/2, H/2 - 10, W/2, H/2 + 10, 0xFFFF);
        
        vid_waitvbl();
    }
    return 0;
}
