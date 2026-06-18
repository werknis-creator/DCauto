#include <simulant/simulant.h>
#include <simulant/nodes/camera.h>
#include <simulant/nodes/stage_node.h>
#include <simulant/nodes/light.h>
#include <simulant/input.h>
#include <iostream>
#include <cmath>

using namespace smlt;

class GTASAGame : public Application {
public:
    GTASAGame(const AppConfig& config) : Application(config) {}

    bool init(const AppConfig& config) override {
        if(!Application::init(config)) return false;
        
        window->set_title("GTA SA Dreamcast");
        window->signal_scene_changed.connect([this]() {
            auto scene = window->scene();
            
            // 1. Konfiguracja Kamery (3. osoba)
            auto camera = scene->new_camera();
            scene->globals()->main_camera()->set_active_camera(camera);
            camera->move_to(Vec3(0, 5, -10));
            camera->look_at(Vec3(0, 0, 0));
            
            // 2. Swiatlo sloneczne
            auto light = scene->new_light();
            light->set_light_type(LIGHT_TYPE_DIRECTIONAL);
            light->move_to(Vec3(0, 20, 10));
            light->look_at(Vec3(0, 0, 0));
            
            // 3. Tworzenie mapy (podloga i budynki w stylu Los Santos)
            // (Tutaj Simulant generuje prosta siatke)
            auto floor = scene->new_mesh();
            auto floor_mesh = floor->mesh();
            auto floor_data = floor_mesh->shared_data();
            floor_data->set_colors({Color(0.4, 0.4, 0.4, 1.0)}); // Asfalt
        });
        return true;
    }

    void update(float dt) override {
        Application::update(dt);
        
        // Pobranie stanu pada Dreamcast
        auto controllers = window->input_state()->controllers();
        if(!controllers.empty()) {
            auto ctrl = controllers[0];
            
            // LEWY ANALOG: Chodzenie
            float lx = ctrl->axis_state(0);
            float ly = ctrl->axis_state(1);
            float speed = sqrt(lx*lx + ly*ly) * 5.0f * dt;
            
            if(speed > 0.1f) {
                float moveAngle = atan2(lx, -ly);
                // Tutaj logika przesuwania wektora gracza
                // player_x += sin(moveAngle) * speed;
            }
            
            // PRAWY ANALOG: Obracanie kamery wokol gracza
            float rx = ctrl->axis_state(2);
            float ry = ctrl->axis_state(3);
            // camera->rotate_y(rx * dt);
            
            // PRZYCISK X: Skakanie
            if(ctrl->is_pressed(1)) { // Kod przycisku X
                // player_vy = 15.0f; // Nadanie predkosci pionowej
            }
            
            // Wyjscie z gry (START + A)
            if(ctrl->is_pressed(0) && ctrl->is_pressed(12)) { // Start + A
                // window->close();
            }
        }
        
        // Fizyka grawitacji
        // player_vy -= 9.81f * dt;
        // player_y += player_vy * dt;
    }
};

int main(int argc, char* argv[]) {
    AppConfig config;
    config.title = "GTA SA Dreamcast";
    config.width = 640;
    config.height = 480;
    return Application::run<GTASAGame>(argc, argv, config);
}
