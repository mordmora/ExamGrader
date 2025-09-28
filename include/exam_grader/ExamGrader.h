#ifndef __EXAM_GRADER__H
#define __EXAM_GRADER__H

#include"exam_grader/ConfigModule.h"
#include<string>
#include<opencv2/opencv.hpp>

class ExamGrader {
    public:
    explicit ExamGrader(const ConfigModule& config) : config(config){}

    void process_exam(const std::string& exam_f_name);

    private:
    ConfigModule config;
    cv::Mat preprocess(const cv::Mat& raw_img);
};

#endif