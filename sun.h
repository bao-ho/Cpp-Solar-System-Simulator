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
    double m_diameter;
    unsigned int m_color;
public:
    Sun(double sunMass, double sunDiameter, unsigned int sunColor, double G);
    double getM();
    void setG (double);
    double getG();
    double getX();
    double getY();
    double getZ();
    unsigned int getColor();
    double getDiameter();
    Vec3d attract(Planet* m);
};

#endif // SUN_H
