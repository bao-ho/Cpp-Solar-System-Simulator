#include "solarsystem.h"
#include "utils.h"
#include "vec3d.h"
#include <QDebug>
#include <iostream>
#include <QtMath>
#include <vector>

using namespace std;

SolarSystem::SolarSystem(double sunMass, double sunDiameter, unsigned int sunColor,
                         std::vector<double>& planetMass, QStringList& planetName,
                         std::vector<double>& planetDiameter, std::vector<unsigned int>& planetColor,
                         std::vector<double>& pX, std::vector<double>& pY, std::vector<double>& pZ, double G)
:Base()
{
    m_sun = new Sun(sunMass, sunDiameter, sunColor, G);
    double v, a, vx, vy, vz, ax, ay, az, dist;
    for (int i=0; i<planetMass.size(); i++)
    {
        dist = qSqrt (pX[i]*pX[i] + pY[i]*pY[i] + pZ[i]*pZ[i]);

        ay = 0;
        a  = - m_sun->getG()*m_sun->getM()/(dist*dist);
        ax = a * (pX[i]-m_sun->getX()) / dist;
        az = a * (pZ[i]-m_sun->getZ()) / dist;

        vy = 0;
        v  = qSqrt(-a*dist);
        vx = v * -(pZ[i]-m_sun->getZ()) / dist;//initial v is perpendicular to initial a
        vz = v *  (pX[i]-m_sun->getX()) / dist;

        Planet* planet = new Planet(planetMass[i], planetName.at(i),
                                    planetDiameter[i], planetColor[i],
                                    pX[i], pY[i], pZ[i],
                                    vx, vy, vz,
                                    ax, ay, az);
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
