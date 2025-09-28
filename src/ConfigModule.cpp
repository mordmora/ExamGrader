#include"exam_grader/ConfigModule.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include<ini.h>
#include<optional>

static int config_handler(void* user, const char* section, const char* name, const char* value){
    ConfigModule* cfg_m = static_cast<ConfigModule*>(user);

    long tmp = std::stol(value);

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    try{
        if(MATCH("Preprocessing", "gaussian_blur_kernel")){
            if(tmp < 0 || tmp > std::numeric_limits<unsigned short int>::max()){
                std::cerr << "gaussian_blur_kernel out of range" << std::endl;
                return -1;
            }
            cfg_m->gaussian_blur_k = static_cast<unsigned short int>(tmp);
        }else if(MATCH("Preprocessing", "adaptive_thresh_block_size")){
            if(tmp < 0 || tmp > std::numeric_limits<unsigned short int>::max()){
                std::cerr << "adaptive_tresh_block_size out of range" << std::endl;
                return -1;
            }
            cfg_m->adaptive_thresh_b_size = static_cast<unsigned short int>(tmp);
        }else if(MATCH("Preprocessing", "adaptive_thresh_c")){
            cfg_m->adaptive_thresh_C = static_cast<double>(tmp);
        }else if(MATCH("Contours", "min_bubble_area")){
            cfg_m->min_bubble_area = static_cast<double>(tmp);
        }else if(MATCH("Contours", "max_bubble_area")){
            cfg_m->max_bubble_area = static_cast<double>(tmp);
        }else if(MATCH("Contours", "bubble_aspect_ratio_tolerance")){
            cfg_m->bubble_aspect_ratio_tolerance = static_cast<double>(tmp);
        }else if(MATCH("Grading", "filled_percentage_threshold")){
            cfg_m->fill_threshold = std::stod(value);
        }else{
            return 0;
        }
    } catch (const std::invalid_argument& iearg){
        (void)iearg;
        std::cerr << "Error de conversion en [" << section << "] " << name << " con valor '" << value << "'" << std::endl;
        return -1;
    }

    return 1;

}   

std::optional<ConfigModule> init_config(const std::string& config_path){
    ConfigModule cfg = {};

    int result = ini_parse(config_path.c_str(), config_handler, &cfg);

    if(result < 0){
        std::cerr << "Error al leer el archivo de configuración" << std::endl;
        return std::nullopt;
    }

    if(result > 0){
        std::cerr << "Error en la línea " << result << " del archivo INI: " << config_path << std::endl;
        return std::nullopt;
    }

    return cfg;
}

