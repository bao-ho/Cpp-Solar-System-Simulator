#ifndef SUN_H
#define SUN_H

#include "base.h"
#include "vec3d.h"
#include "planet.h"

class Sun : public Base {
private:
    double m_mass;
    Vec3d m_loc;
    double m_G;
public:
    Sun();
    Sun(double sunMass, double G);
    double getM();
    void setG (double);
    double getG();
    double getX();
    double getY();
    double getZ();
    Vec3d attract(Planet* m);
};

#endif // SUN_H
