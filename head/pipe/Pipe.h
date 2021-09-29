#ifndef PIPE
#define PIPE

#include <iostream>
#include <chrono>
#include <random>

class RandomGenerator
{
private:
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine = std::default_random_engine(seed);
    std::uniform_real_distribution<double> distr = std::uniform_real_distribution<double>(0, 420);

public:
    double getRandomNumber()
    {
        return distr(engine);
    }

};

class Pipe
{
private:
    RandomGenerator rng;

public:
    Pipe();
    Pipe(Pipe& other) = default;

public:
    //int holeSize = 300;
    int holeSize = 200;
    int size = 200;
    int width = 200;

    double holeLocation = rng.getRandomNumber();
    double position = 0;

    bool passed = false;
    bool moving = false;
};

#endif 