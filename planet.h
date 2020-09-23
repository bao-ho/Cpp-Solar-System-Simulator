#ifndef PLANET_H
#define PLANET_H

#include "base.h"
#include "vec3d.h"

class Planet : public Base {
protected:
    Vec3d m_loc;
    Vec3d m_vel;
    Vec3d m_acc;
    double m_mass;
public:
    Planet(double m, double x, double y, double z);
    Planet(double m, double x, double y, double z,
           double vx, double vy, double vz,
           double ax, double ay, double az);
    void applyForce(const Vec3d& force);
    void update(double dt);

    Vec3d location();
    double mass();
    double getX();
    double getZ();
    double getVx();
    double getVz();
    double getAx();
    double getAz();
    void setX(double);
    void setZ(double);
    void setVx(double);
    void setVz(double);
    void setAx(double);
    void setAz(double);
};

#endif // PLANET_H
