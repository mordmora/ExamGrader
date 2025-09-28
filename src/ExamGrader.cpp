#include"exam_grader/ExamGrader.h"
#include<opencv2/opencv.hpp>
#include<algorithm>

cv::Mat ExamGrader::preprocess(const cv::Mat& raw_img){
    cv::Mat gray, blurred, binary;

    cv::cvtColor(raw_img, gray, cv::COLOR_BGR2GRAY);

    cv::GaussianBlur(gray, blurred, cv::Size(config.gaussian_blur_k, config.gaussian_blur_k),0);

    if(config.adaptive_thresh_b_size == 0){
        cv::threshold(blurred, binary, 220, 255, cv::THRESH_BINARY_INV);
    }else{
        cv::adaptiveThreshold(blurred, binary, 255, 
            cv::ADAPTIVE_THRESH_GAUSSIAN_C,
            cv::THRESH_BINARY_INV,
            config.adaptive_thresh_b_size,
            config.adaptive_thresh_C
        );
    }

    cv::Mat kernel  = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

    cv::dilate(binary, binary, kernel, cv::Point(-1, -1), 4);
    return binary;
}

void ExamGrader::process_exam(const std::string& exam_f_name){
    cv::Mat img = cv::imread(exam_f_name);
    if(img.empty()){
        std::cerr << "Error al abrir la imagen" << std::endl;
        return;
    }

    cv::Mat binary_img = preprocess(img);
    cv::imwrite("assets/binary.jpg", binary_img);

    using pointV = std::vector<std::vector<cv::Point>>;
    pointV contours;
    cv::findContours(binary_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::cout << "Cantidad de contornos -> " << contours.size() << std::endl;

    pointV bubble_contours;
    for(const auto& C : contours){
        cv::Rect BB = cv::boundingRect(C);
        double area = cv::contourArea(C);
        double aspect_ratio = static_cast<double>(BB.width) / BB.height;

        //std::cout << "Contorno con area -> " << area << std::endl;
        if(area > config.min_bubble_area && area < config.max_bubble_area 
        && std::abs(1.0 - aspect_ratio) < config.bubble_aspect_ratio_tolerance){
            bubble_contours.push_back(C);
        }
    }

    //std::cout << "Contornos filtrados -> " << bubble_contours.size() << std::endl;

    cv::Mat dump_img = img.clone();
    cv::drawContours(dump_img, bubble_contours, -1, cv::Scalar(0,0,255), 2);
    cv::imwrite("assets/detected_bubbles.jpg", dump_img);

    std::sort(bubble_contours.begin(), bubble_contours.end(),
        [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b ){
            return cv::boundingRect(a).y < cv::boundingRect(b).y;
        }
    );

    for(size_t i = 0; i < bubble_contours.size(); i+=4){
        std::sort(bubble_contours.begin() + i, bubble_contours.begin() + i +4,
            [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b){
                return cv::boundingRect(a).x < cv::boundingRect(b).x;
            }
        );
    }

    cv::Mat labeled_img = img.clone();

    for(size_t i = 0; i < bubble_contours.size(); ++i){
        cv::Rect BB = cv::boundingRect(bubble_contours[i]);
        cv::putText(labeled_img, std::to_string(i), cv::Point(BB.x, BB.y - 5), cv::FONT_HERSHEY_COMPLEX, 0.5,
        cv::Scalar(0,0,255), 2);
    }

    cv::imwrite("assets/labeled_bubbles.jpg", labeled_img);

    /*
    A=0, B=1, C=2, D=3
    */
    //Pregunta: respuesta
    std::map<int, int> answer_k = {
        {0, 0}, {1, 3}, {2, 2}, {3, 2},{4,0},
        {5, 2}, {6,1}, {7, 0}, {8,1}, {9,2}
    };

    double score = 0;

    for(size_t i = 0; i < bubble_contours.size(); i+=4){
        int q_num = i/4;
        int max_p = -1;
        int usr_ans_idx = -1;

        std::vector<int> marked_opt;
        double max_fill_ratio = 0;
        for(size_t j = 0; j < 4; j++){

            const auto& B = bubble_contours[i+j];

            cv::Mat mask = cv::Mat::zeros(binary_img.size(), CV_8UC1);
/*
            const auto& current_contour = bubble_contours[i + j];
            std::cout << "Depurando contorno #" << (i + j) << ": "
              << "Numero de puntos = " << current_contour.size() << ". ";
            cv::Rect bb = cv::boundingRect(current_contour);
            std::cout << "Area del BBox = " << bb.area() << std::endl;

*/
    
            cv::drawContours(mask, bubble_contours, i + j, cv::Scalar(255), cv::FILLED);

            cv::Mat masked_bubble;
            cv::bitwise_and(binary_img, mask, masked_bubble);
            cv::imwrite("assets/masked.jpg", masked_bubble);

            int filled_pixels = cv::countNonZero(masked_bubble);
            double total_area = cv::contourArea(B);
            double fill_ratio = (total_area > 0) ? filled_pixels / total_area : 0;

            if(fill_ratio > config.fill_threshold){
                //std::cout << "fill ratio: " << fill_ratio << ",fill treshold: " << config.fill_threshold << std::endl;
                marked_opt.push_back(j);
            }

            if(fill_ratio > max_fill_ratio){
                max_fill_ratio = fill_ratio;
                usr_ans_idx = j;
            }
        }
        char correct_choice = 'A' + answer_k.at(q_num);
        std::cout << "Pregunta " << (q_num + 1) << ": ";

        if (marked_opt.size() == 0) { // Regla 2: Sin respuesta
            score -= 0.1;
            std::cout << "Sin responder. Correcta: " << correct_choice << " -> PENALIZACION (-0.1)";
        }    
        else if (marked_opt.size() > 1) { // Regla 1: Múltiples respuestas
            std::cout << "Multiples marcas. Anulado.";
        } 
        else { // Caso normal: una sola respuesta marcada
            usr_ans_idx = marked_opt[0];
            bool is_correct = (answer_k.at(q_num) == usr_ans_idx);
            if (is_correct) {
                score += 1.0;
            }
            char user_choice = 'A' + usr_ans_idx;
            std::cout << "Marcado: " << user_choice << " | Correcto: " << correct_choice
                  << " -> " << (is_correct ? "CORRECTO (+1.0)" : "INCORRECTO");
        }
        std::cout << std::endl;
    }

    std::cout << "Nota total: "<<score << std::endl;
}