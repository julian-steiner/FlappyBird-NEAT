#ifndef BIRD
#define BIRD

class Bird
{
public:
    double velocity;
    double position = 600;

public:
    bool alive = true;
    //double clickCooldown = 0.3;
    double clickCooldown = 0.3;
    double clickTimer = 0;
    double size = 20;
    double timeSurvived = 0;
};

#endif