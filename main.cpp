#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

float distance(sf::Vector2f a, sf::Vector2f b){
    return std::sqrt(std::pow((a.x-b.x), 2) + std::pow((a.y-b.y), 2));
}

float ratio(float value, float zero, float one){
    return (value-zero)/(one-zero);
}
float ratio(sf::Time value, sf::Time zero, sf::Time one){
    return (value-zero)/(one-zero);
}

struct data_set{
    sf::Vector2f position;
    float speed;
    float car_distance;
    float track_distance;
};

struct TRACK{
    sf::VertexArray in_track;
    sf::VertexArray out_track;

    void load_from_file(std::string file_path){
        std::string trash;
        std::ifstream file;
        file.open(file_path);
        if(file.is_open()){
            in_track.setPrimitiveType(sf::LinesStrip);
            out_track.setPrimitiveType(sf::LinesStrip);
            file >> trash;
            sf::Vector2f pt;
            int in_nbpts, out_nbpts;
            file >> in_nbpts;
            in_track.resize(in_nbpts);
            for(int i = 0; i < in_nbpts; ++i){
                file >> in_track[i].position.x >> in_track[i].position.y;
                in_track[i].color = sf::Color(255, 255, 255);
            }
            file >> trash;
            file >> out_nbpts;
            out_track.resize(out_nbpts);
            for(int i = 0; i < out_nbpts; ++i){
                file >> out_track[i].position.x >> out_track[i].position.y;
                out_track[i].color = sf::Color(255, 255, 255);
            }
        }else
            std::cout << "Erreur à l'ouverture du fichier de circuit." << std::endl;    
        }
};

struct TELEMETRY{
    int size;
    sf::Time * times;
    data_set * datas;
    sf::Time lap_time;

    void set_track_distances(){
        datas[0].track_distance = 0;
        for(int i = 1; i < size; ++i){
            datas[i].track_distance = datas[i-1].track_distance + distance(datas[i].position, datas[i-1].position);
        }
    }

    void load_from_file(std::string file_path){
        std::ifstream file;
        file.open(file_path);
        if(file.is_open()){
            file >> size;
            times = new sf::Time[size];
            datas = new data_set[size];
            for(int i = 0; i < size; ++i){
                float seconds;
                file >> seconds;
                times[i] = sf::seconds(seconds);
                file >> datas[i].position.x >> datas[i].position.y >> datas[i].car_distance >> datas[i].speed;
            }
            lap_time = times[size-1];
            set_track_distances();
        }else
            std::cout << "Erreur d'ouverture du fichier : " << file_path << std::endl;
    }

    void set_pos_and_speed_anim(sf::Vector2f & pos, float & speed, sf::Time time){
        int time_index = 0;
        while(time_index < size and times[time_index+1] < time){
            ++time_index;
        }

        //Pourcentage d'avancement entre les deux acquisitions
        float time_ratio = ratio(time, times[time_index], times[time_index +1]);
        //Distance parcouru depuis le début du tour
        float distance_driven = datas[time_index].car_distance + (datas[time_index+1].car_distance - datas[time_index].car_distance) * time_ratio;
        
        // Pourcentage du circuit completé
        float lap_progress = distance_driven / datas[size-1].track_distance;
        
        // Distance parcouru avec ce pourcentage dans notre circuit polygone
        float on_screen_distance_driven = lap_progress * datas[size-1].track_distance;
        
        // On regarde cherche l'index ou on est a cette distance
        int track_distances_index = 0;
        while(track_distances_index + 1 < size and datas[track_distances_index+1].track_distance < on_screen_distance_driven)
            ++track_distances_index;

        //Si on est pas à la fin
        if(track_distances_index + 1 < size){
            sf::Vector2f prev_pos = pos;
            // ratio de distance entre deux aquisition dans notre circuit polygone
            float dist_ratio = ratio(on_screen_distance_driven,datas[track_distances_index].track_distance, datas[track_distances_index+1].track_distance);
            //On se place au bon endroit cette fois sur le circuit polygone
            pos = datas[track_distances_index].position + (datas[track_distances_index+1].position - datas[track_distances_index].position)*dist_ratio;
            speed = datas[track_distances_index].speed;
        }
    }
};

class CAR : public sf::CircleShape{
private:
    TELEMETRY car_tel;
    sf::Time last_update_time;
public:
    CAR(sf::Color color, float radius, std::string tel_file) : sf::CircleShape(radius){
        this->setFillColor(color);
        car_tel.load_from_file(tel_file);
        this->setFillColor(sf::Color(255, 0, 0));
        this->setOrigin(radius, radius);
    }
    sf::Vector2f pos;
    float speed;

    void update_for_anim(sf::Time elapsed_time){
        car_tel.set_pos_and_speed_anim(pos, speed, elapsed_time);
        this->setPosition(pos);
        std::cout << "CAR INFO UPDATED " << pos.x << " " << pos.y << " " << speed << std::endl;
    }
    
};

int main(){
    TRACK my_track;
    my_track.load_from_file("data/track.txt");

    CAR rus_car(sf::Color(0, 0, 255), 2.f, "data/rus_tel.txt");
    CAR ham_car(sf::Color(255, 0, 0), 2.f, "data/ham_tel.txt");

    sf::ContextSettings settings;
    settings.antialiasingLevel = 6;
    sf::RenderWindow window(sf::VideoMode(500, 500), "F1 APP", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);
   
    sf::Clock clock;
    sf::View view;
    view.reset(sf::FloatRect(0.f, 0.f, 100.f, 100.f));
    view.setCenter(sf::Vector2f(0, 0));

    sf::Vector2f baseSize(100.f, 100.f);
    
    std::cout << "LANCEMENT DE L'APP" << std::endl;
    while( window.isOpen() ){
        sf::Event event;
        while( window.pollEvent(event)){
            if (event.type == sf::Event::Closed)
                window.close();
        }
        sf::Time elapsed_time = clock.getElapsedTime();
        rus_car.update_for_anim(elapsed_time);
        ham_car.update_for_anim(elapsed_time);
        view.setCenter(view.getCenter() + (rus_car.pos-view.getCenter())*0.8f);

        float zoomFactor = rus_car.speed / 200.f;
        sf::Vector2f targetSize = baseSize + (baseSize * zoomFactor);
        sf::Vector2f currentSize = view.getSize();
        view.setSize(currentSize + (targetSize - currentSize) * 0.05f);
        window.setView(view);
        window.clear();
        window.draw(ham_car);
        window.draw(rus_car);
        window.draw(my_track.in_track);
        window.draw(my_track.out_track);
        window.display();
        
    }
    return 0;
}