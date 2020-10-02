#ifndef PLANET_H
#define PLANET_H

#include "base.h"
#include "vec3d.h"
#include <qdebug.h>

class Planet : public Base {
protected:
    Vec3d m_loc;
    Vec3d m_vel;
    Vec3d m_acc;
    double m_mass;
    QString m_name;
    double m_diameter;
    unsigned int m_color;
public:
    Planet(double m, double x, double y, double z);
    Planet(double m, QString name,
           double diameter, unsigned int color,
           double x, double y, double z,
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
    unsigned int getColor();
    QString getName();
    double getDiameter();
    void setX(double);
    void setZ(double);
    void setVx(double);
    void setVz(double);
    void setAx(double);
    void setAz(double);
};

#endif // PLANET_H
