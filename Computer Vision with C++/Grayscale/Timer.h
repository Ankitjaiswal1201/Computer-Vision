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

#endif