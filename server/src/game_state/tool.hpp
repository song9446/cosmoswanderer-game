#pragma once
#include <random>
#include <iostream>
namespace tool {
    int rand(const int& f, const int& t){
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(f,t);
        return dist(mt);
    }
    template<typename T>
    const T& randomSelect(const T* pool, int size){
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(0,size-1);
        return pool[dist(mt)];
    } 
};
