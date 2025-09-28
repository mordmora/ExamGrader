#ifndef __CONFIG_MODULE__H
#define __CONFIG_MODULE__H

typedef unsigned short int us_int;
#include<optional>
#include<string>

struct ConfigModule{
    us_int gaussian_blur_k; //Tamaño de kernel para blurs
    us_int adaptive_thresh_b_size; //Tamaño del vecindario para el umbral adaptativo
    double adaptive_thresh_C;
    double min_bubble_area;
    double max_bubble_area;
    double bubble_aspect_ratio_tolerance;
    double fill_threshold; //Porcentaje para considerar circulo valido o no
};

std::optional<ConfigModule> init_config(const std::string& json_config_file_path);

#endif