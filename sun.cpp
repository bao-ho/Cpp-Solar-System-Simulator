#include "sun.h"

double constrain(double value, double minValue, double maxValue)
{
    if (value<minValue)
        return minValue;

    if (value>maxValue)
        return maxValue;

    return value;
}

Sun::Sun(double sunMass, double sunDiameter, unsigned int sunColor, double G)
{
    m_loc.setComponents(0.0, 0.0, 0.0);
    m_mass = sunMass;
    m_G = G;
    m_diameter = sunDiameter;
    m_color    = sunColor;
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

unsigned int Sun::getColor()
{
    return m_color;
}
double Sun::getDiameter()
{
    return m_diameter;
}

Vec3d Sun::attract(Planet* m)
{
    Vec3d distance = m_loc - m->location();
    double d = distance.length();
    //d = constrain(d, 5.0, 25.0);
    double strength = (m_G * m_mass * m->mass()) / (d * d);
    return distance * strength / d;
}
