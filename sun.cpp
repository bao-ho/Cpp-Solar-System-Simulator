#include "sun.h"

double constrain(double value, double minValue, double maxValue)
{
    if (value<minValue)
        return minValue;

    if (value>maxValue)
        return maxValue;

    return value;
}

Sun::Sun()
{
    m_loc.setComponents(0.0, 0.0, 0.0);
    m_mass = 2000.0;
    m_G = 0.1;
}

Sun::Sun(double sunMass, double G)
{
    m_loc.setComponents(0.0, 0.0, 0.0);
    m_mass = sunMass;
    m_G = G;
}
double Sun::getM()
{
    return m_mass;
}
void Sun::setG(double new_G)
{
    m_G = new_G;
}

double Sun::getG()
{
    return m_G;
}
double Sun::getX()
{
    return m_loc.x();
}
double Sun::getY()
{
    return m_loc.y();
}
double Sun::getZ()
{
    return m_loc.z();
}


Vec3d Sun::attract(Planet* m)
{
    Vec3d force = m_loc - m->location();
    double d = force.length();
    //d = constrain(d, 5.0, 25.0);
    double strength = (m_G * m_mass * m->mass()) / (d * d);
    return force * strength / d;
}
