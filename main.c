#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/video.h>
#include <math.h>
#include <string.h>

uint16 *vram;
#define W 640
#define H 480

// Silnik rysowania linii (Algorytm Bresenhama)
void draw_line(int x1, int y1, int x2, int y2, uint16 color) {
    int dx = abs(x2-x1), sx = x1<x2 ? 1 : -1;
    int dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1; 
    int err = dx+dy, e2;
    while(1) {
        if(x1 >= 0 && x1 < W && y1 >= 0 && y1 < H) vram[y1 * W + x1] = color;
        if (x1==x2 && y1==y2) break;
        e2 = 2*err;
        if (e2 > dy) { err += dy; x1 += sx; }
        else if (e2 < dx) { err += dx; y1 += sy; }
    }
}

// Silnik 3D: Projekcja i rysowanie pudelka
void draw_box_3d(float x, float y, float z, float w, float h, float d, float cx, float cy, float cz, float cyaw, uint16 col) {
    float vertices[8][3] = {
        {x, y, z}, {x+w, y, z}, {x+w, y+h, z}, {x, y+h, z},
        {x, y, z+d}, {x+w, y, z+d}, {x+w, y+h, z+d}, {x, y+h, z+d}
    };
    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7}
    };
    float cos_y = cos(cyaw), sin_y = sin(cyaw);
    int sx[8], sy[8], valid[8];
    
    for(int i=0; i<8; i++) {
        float rx = (vertices[i][0] - cx) * cos_y - (vertices[i][2] - cz) * sin_y;
        float rz = (vertices[i][0] - cx) * sin_y + (vertices[i][2] - cz) * cos_y;
        float ry = vertices[i][1] - cy;
        if(rz < 1.0) { valid[i] = 0; continue; } // Clipping
        valid[i] = 1;
        sx[i] = (rx * 400.0 / rz) + W/2;
        sy[i] = (ry * 400.0 / rz) + H/2;
    }
    for(int i=0; i<12; i++) {
        if(valid[edges[i][0]] && valid[edges[i][1]]) {
            draw_line(sx[edges[i][0]], sy[edges[i][0]], sx[edges[i][1]], sy[edges[i][1]], col);
        }
    }
}

// Silnik 3D: Rysowanie podlogi (siatki)
void draw_floor(float cx, float cy, float cz, float cyaw, uint16 col) {
    float cos_y = cos(cyaw), sin_y = sin(cyaw);
    int step = 10, range = 100;
    for(int i=-range; i<=range; i+=step) {
        float x1 = i, z1 = -range, x2 = i, z2 = range;
        float rx1 = (x1 - cx) * cos_y - (z1 - cz) * sin_y;
        float rz1 = (x1 - cx) * sin_y + (z1 - cz) * cos_y;
        float rx2 = (x2 - cx) * cos_y - (z2 - cz) * sin_y;
        float rz2 = (x2 - cx) * sin_y + (z2 - cz) * cos_y;
        if(rz1 > 1.0 && rz2 > 1.0) {
            draw_line((rx1*400.0/rz1)+W/2, ((0-cy)*400.0/rz1)+H/2, (rx2*400.0/rz2)+W/2, ((0-cy)*400.0/rz2)+H/2, col);
        }
        x1 = -range; z1 = i; x2 = range; z2 = i;
        rx1 = (x1 - cx) * cos_y - (z1 - cz) * sin_y;
        rz1 = (x1 - cx) * sin_y + (z1 - cz) * cos_y;
        rx2 = (x2 - cx) * cos_y - (z2 - cz) * sin_y;
        rz2 = (x2 - cx) * sin_y + (z2 - cz) * cos_y;
        if(rz1 > 1.0 && rz2 > 1.0) {
            draw_line((rx1*400.0/rz1)+W/2, ((0-cy)*400.0/rz1)+H/2, (rx2*400.0/rz2)+W/2, ((0-cy)*400.0/rz2)+H/2, col);
        }
    }
}

int main(int argc, char **argv) {
    vram = vram_s; // Bezposredni dostep do pamieci wideo Dreamcasta
    
    float px = 0, py = 0, pz = 0, yaw = 0, vy = 0;
    int on_ground = 1;
    
    while(1) {
        maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        if(cont) {
            cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
            if(state) {
                float lx = (state->joyx - 128) / 128.0f;
                float ly = (state->joyy - 128) / 128.0f;
                if(fabs(lx) < 0.2) lx = 0;
                if(fabs(ly) < 0.2) ly = 0;
                
                yaw -= lx * 2.0f * 0.016f;
                px += sin(yaw) * (-ly) * 5.0f * 0.016f;
                pz += cos(yaw) * (-ly) * 5.0f * 0.016f;
                
                if((state->buttons & CONT_X) && on_ground) { vy = 8.0f; on_ground = 0; }
                if((state->buttons & CONT_START) && (state->buttons & CONT_A)) break;
            }
        }
        
        if(!on_ground) vy -= 20.0f * 0.016f;
        py += vy * 0.016f;
        if(py <= 0) { py = 0; vy = 0; on_ground = 1; }
        
        memset(vram, 0, W * H * 2); // Czyszczenie ekranu (czarny)
        
        float cam_x = px - sin(yaw) * 10.0f;
        float cam_z = pz - cos(yaw) * 10.0f;
        float cam_y = py + 5.0f;
        
        draw_floor(cam_x, cam_y, cam_z, yaw, 0x7BEF); // Szara siatka
        draw_box_3d(px, py, pz, 1.0, 2.0, 1.0, cam_x, cam_y, cam_z, yaw, 0xFFFF); // Gracz (Bialy)
        draw_box_3d(-20, 0, 20, 10, 15, 10, cam_x, cam_y, cam_z, yaw, 0xF800); // Budynek (Czerwony)
        draw_box_3d(15, 0, -30, 12, 25, 12, cam_x, cam_y, cam_z, yaw, 0x07E0); // Budynek (Zielony)
        
        vid_waitvbl();
    }
    return 0;
}
