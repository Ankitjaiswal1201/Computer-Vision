#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>

#define INIT_TIMER auto start = std::chrono::high_resolution_clock::now();
#define START_TIMER  start = std::chrono::high_resolution_clock::now();
#define STOP_TIMER(name)  std::cout << "RUNTIME of " << name << ": " << \
    std::chrono::duration_cast<std::chrono::nanoseconds>( \
            std::chrono::high_resolution_clock::now()-start \
    ).count() << " ns " << std::endl;
#define STOP_TIMER_SEC(name)  std::cout << "RUNTIME of " << name << ": " << \
    ((std::chrono::duration_cast<std::chrono::nanoseconds>( \
            std::chrono::high_resolution_clock::now()-start \
    ).count())/1000000000) << " s " << std::endl;
#define STOP_TIMER_FPS(name)  std::cout << "RUNTIME of " << name << ": " << \
    (1000000000.0/double(std::chrono::duration_cast<std::chrono::nanoseconds>( \
            std::chrono::high_resolution_clock::now()-start \
    ).count())) << " fps " << std::endl;

#endif
