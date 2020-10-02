#include "planet.h"

#include "utils.h"

Planet::Planet(double m, double x, double y, double z)
{
    m_loc.setComponents(x, y, z);
    m_vel.setComponents(random(-1.0, 1.0)*5, 0.0, random(-1.0, 1.0)*5);
    m_acc.setComponents(0.0, 0.0, 0.0);
    m_mass = m;
}

Planet::Planet(double m, QString name,
               double diameter, unsigned int color,
               double x, double y, double z,
               double vx, double vy, double vz,
               double ax, double ay, double az)
{
    m_loc.setComponents(x, y, z);
    m_vel.setComponents(vx, vy, vz);
    m_acc.setComponents(ax, ay, az);
    m_name = name;
    m_mass = m;
    m_diameter = diameter;
    m_color = color;
}

Vec3d Planet::location()
{
    return m_loc;
}

double Planet::mass()
{
    return m_mass;
}

void Planet::applyForce(const Vec3d& force)
{
    Vec3d a = force / m_mass;
    m_acc += a;
}

void Planet::update(double dt)
{
    m_vel += m_acc * dt;
    m_loc += m_vel * dt;
    m_acc.setComponents(0.0, 0.0, 0.0);
}

double Planet::getX(){
    return m_loc.x();
}

double Planet::getZ(){
    return m_loc.z();
}
double Planet::getVx(){
    return m_vel.x();
}

double Planet::getVz(){
    return m_vel.z();
}

double Planet::getAx(){
    return m_acc.x();
}

double Planet::getAz(){
    return m_acc.z();
}
unsigned int Planet::getColor()
{
    return m_color;
}

QString Planet::getName()
{
    return m_name;
}
double Planet::getDiameter()
{
    return m_diameter;
}

void Planet::setX(double x){
    m_loc.setX(x);
}
void Planet::setZ(double z){
    m_loc.setZ(z);
}
void Planet::setVx(double vx){
    m_vel.setX(vx);
}
void Planet::setVz(double vz){
    m_vel.setZ(vz);
}
void Planet::setAx(double ax){
    m_acc.setX(ax);
}
void Planet::setAz(double az){
    m_acc.setZ(az);
}
