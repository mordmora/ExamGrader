#include <iostream>
#include <optional>
#include <stdexcept>
#include "exam_grader/ConfigModule.h"
#include "exam_grader/ExamGrader.h"
#define SUCCESS 0
#define FAILURE 1

std::string config_path = "./config.ini";

int main(int argc, char** argv){

    if(argc < 2){
        std::cerr << "Uso: exam_grader <img_path> [opcional: <config_path>]" << std::endl;
        return FAILURE;
    }

    if(argc >= 3){
        config_path = argv[2];
    } 

    auto cfg = init_config(config_path);

    try{
        ExamGrader grader(cfg.value());
        grader.process_exam(argv[1]);

    }catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        return FAILURE;
    }

    return SUCCESS;
}