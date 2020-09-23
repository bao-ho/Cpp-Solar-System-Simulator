#include "solarsystem.h"
#include "utils.h"
#include "vec3d.h"
#include <QDebug>
#include <iostream>
#include <QtMath>

using namespace std;

SolarSystem::SolarSystem(int n)
:Base()
{
    m_sun = new Sun();
    double m, x, y, z, v, vx, vy, vz, a, ax, ay, az, dist;
    for (int i=0; i<n; i++)
    {
        m = random(0.5, 2);
        x = randomAbs(5, 25);
        y = 0;
        z = randomAbs(5, 25);
        dist = qSqrt (x*x + y*y + z*z);

        ay = 0;
        a  = - m_sun->getG()*m_sun->getM()/(dist*dist);
        ax = a * (x-m_sun->getX()) / dist;
        az = a * (z-m_sun->getZ()) / dist;

        vy = 0;
        v  = qSqrt(-a*dist);
        vx = v * -(z-m_sun->getZ()) / dist;//initial v is perpendicular to initial a
        vz = v * (x-m_sun->getX()) / dist;

        Planet* planet = new Planet(m, x, y, z, vx, vy, vz, ax, ay, az);
        m_planets.push_back(planet);
    }
}

SolarSystem::SolarSystem(int n, double sunMass, double* pMass, double* pX, double* pY, double* pZ, double G)
:Base()
{
    m_sun = new Sun(sunMass, G);
    for (int i=0; i<n; i++)
    {
        double vx, vy, vz, ax, ay, az, dist;
        dist = qSqrt (pX[i]*pX[i]+pY[i]*pY[i]+pZ[i]*pZ[i]);
        ay = az = 0;
        ax = - m_sun->getG()*m_sun->getM()/(dist*dist);
        vx = vy = 0;
        vz = qSqrt(-ax*dist);
        Planet* planet = new Planet(pMass[i], pX[i], pY[i], pZ[i], vx, vy, vz, ax, ay, az);
        m_planets.push_back(planet);
    }
}

SolarSystem::~SolarSystem()
{
    this->clear();
}

int SolarSystem::size()
{
    return m_planets.size();
}

void SolarSystem::clear()
{
    //for (Planet* planet : m_planets)
    for (auto it= m_planets.begin(); it!=m_planets.end(); it++)
        delete *it;

    m_planets.clear();
}

void SolarSystem::update(double dt)
{
    for (Planet* planet : m_planets)
    //for (auto it=m_planets.begin(); it!=m_planets.end(); it++)
    {
        //Planet* planet = *it;
        Vec3d force = m_sun->attract(planet);
        planet->applyForce(force);
        planet->update(dt);
    }
}

Planet* SolarSystem::at(int idx)
{
    if ((idx>=0)&&(idx<m_planets.size()))
        return m_planets[idx];
}

Sun* SolarSystem::getSun(){
    return m_sun;
}
