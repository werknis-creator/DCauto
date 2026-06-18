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

    // Stan gry i fizyki
    StageNode* player_node_ = nullptr;
    Camera* camera_ = nullptr;
    
    Vec3 player_pos = Vec3(0, 0, 0);
    Vec3 player_vel = Vec3(0, 0, 0);
    float camera_yaw = 0.0f;
    bool is_on_ground = true;

    bool init(const AppConfig& config) override {
        if(!Application::init(config)) return false;
        
        window->set_title("GTA SA DC");
        window->signal_scene_changed([this]() {
            auto scene = window->scene();
            
            // 1. Konfiguracja Kamery
            camera_ = scene->new_camera();
            scene->globals()->main_camera()->set_active_camera(camera_);
            
            // 2. Oswietlenie (Slonce)
            auto light = scene->new_light();
            light->set_light_type(LIGHT_TYPE_DIRECTIONAL);
            light->move_to(Vec3(0.5, -1.0, 0.5));
            
            // 3. Tworzenie Mapy (Wielka plaszczyzna - asfalt w Los Santos)
            auto floor = scene->new_mesh();
            auto f_data = floor->mesh()->shared_data();
            f_data->set_colors({Color(0.3, 0.3, 0.3, 1.0)});
            f_data->set_positions({
                Vec3(-200, 0, -200), Vec3(200, 0, -200), 
                Vec3(200, 0, 200), Vec3(-200, 0, 200)
            });
            f_data->set_indices({0, 1, 2, 0, 2, 3});
            f_data->set_normals({Vec3(0,1,0), Vec3(0,1,0), Vec3(0,1,0), Vec3(0,1,0)});
            
            // 4. Tworzenie Ludzika (CJ) - Zbudowanego z dwoch prostopadloscianow
            player_node_ = scene->new_stage_node();
            
            // Tulow (Zielona bluza)
            auto body = scene->new_mesh();
            auto b_data = body->mesh()->shared_data();
            b_data->set_colors({Color(0.0, 0.6, 0.0, 1.0)});
            b_data->set_positions({
                Vec3(-0.5, 0, -0.5), Vec3(0.5, 0, -0.5), Vec3(0.5, 2, -0.5), Vec3(-0.5, 2, -0.5),
                Vec3(-0.5, 0, 0.5), Vec3(0.5, 0, 0.5), Vec3(0.5, 2, 0.5), Vec3(-0.5, 2, 0.5)
            });
            b_data->set_indices({
                0,1,2, 0,2,3, 4,6,5, 4,7,6, // Przod i tyl
                0,4,5, 0,5,1, 2,6,7, 2,7,3, // Boki
                3,7,4, 3,4,0, 1,5,6, 1,6,2  // Gora i dol
            });
            body->parent()->set_parent(player_node_);
            
            // Glowka (Czarny kolor)
            auto head = scene->new_mesh();
            auto h_data = head->mesh()->shared_data();
            h_data->set_colors({Color(0.1, 0.1, 0.1, 1.0)});
            h_data->set_positions({
                Vec3(-0.3, 2, -0.3), Vec3(0.3, 2, -0.3), Vec3(0.3, 2.6, -0.3), Vec3(-0.3, 2.6, -0.3),
                Vec3(-0.3, 2, 0.3), Vec3(0.3, 2, 0.3), Vec3(0.3, 2.6, 0.3), Vec3(-0.3, 2.6, 0.3)
            });
            h_data->set_indices({
                0,1,2, 0,2,3, 4,6,5, 4,7,6,
                0,4,5, 0,5,1, 2,6,7, 2,7,3,
                3,7,4, 3,4,0, 1,5,6, 1,6,2
            });
            head->parent()->set_parent(player_node_);
        });
        
        return true;
    }

    void update(float dt) override {
        Application::update(dt);
        
        auto controllers = window->input_state()->controllers();
        if(!controllers.empty()) {
            auto ctrl = controllers[0];
            
            // Pobieranie danych z analogow (osie 0,1 to Lewy, 2,3 to Prawy)
            float lx = ctrl->axis_state(0);
            float ly = ctrl->axis_state(1);
            float rx = ctrl->axis_state(2);
            
            // Martwa strefa (deadzone), aby postac nie chodzila sama z powodu bledu kalibracji
            if(fabs(lx) < 0.2) lx = 0;
            if(fabs(ly) < 0.2) ly = 0;
            if(fabs(rx) < 0.2) rx = 0;

            // Obracanie kamery wokol osi Y (Prawy Analog)
            camera_yaw -= rx * 3.0f * dt;
            
            // Wyznaczanie kierunku ruchu (W GTA SA postac obraca sie tam gdzie kamera)
            float move_angle = camera_yaw;
            Vec3 forward(sin(move_angle), 0, cos(move_angle));
            Vec3 right(cos(move_angle), 0, -sin(move_angle));
            
            // Chodzenie (Lewy Analog)
            float speed = 8.0f;
            player_pos += forward * (-ly * speed * dt);
            player_pos += right * (lx * speed * dt);
            
            // Skakanie (Przycisk X)
            // Na padzie Dreamcast X to zazwyczaj kontroller 8 (CONT_X)
            if(ctrl->is_pressed(8) && is_on_ground) { 
                player_vel.y = 10.0f;
                is_on_ground = false;
            }
            
            // Wyjscie z gry (Start + A)
            if(ctrl->is_pressed(16) && ctrl->is_pressed(4)) { 
                window->close();
            }
        }
        
        // Fizyka Grawitacji
        if(!is_on_ground) {
            player_vel.y -= 20.0f * dt; // Przyspieszenie ziemskie
        }
        player_pos.y += player_vel.y * dt;
        
        // Kolizja z podloga
        if(player_pos.y <= 0.0f) {
            player_pos.y = 0.0f;
            player_vel.y = 0.0f;
            is_on_ground = true;
        }
        
        // Aktualizacja pozycji i rotacji gracza
        if(player_node_) {
            player_node_->move_to(player_pos);
            player_node_->set_rotation(Quat::angle_axis(camera_yaw, Vec3(0, 1, 0)));
        }
        
        // Logika Kamery w 3. Osobie (TPS)
        if(camera_) {
            float cam_dist = 10.0f;
            float cam_height = 4.0f;
            
            // Kamera znajduje sie za plecami gracza
            Vec3 cam_offset(
                -sin(camera_yaw) * cam_dist,
                cam_height,
                -cos(camera_yaw) * cam_dist
            );
            
            camera_->move_to(player_pos + cam_offset);
            camera_->look_at(player_pos + Vec3(0, 2, 0)); // Celuje w klatke piersiowa postaci
        }
    }
};

int main(int argc, char* argv[]) {
    AppConfig config;
    config.title = "GTA SA Dreamcast";
    config.width = 640;
    config.height = 480;
    return Application::run<GTASAGame>(argc, argv, config);
}
