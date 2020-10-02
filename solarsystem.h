 #pragma once

#include <vector>

#include "base.h"
#include "planet.h"
#include "sun.h"
#include <string>

class SolarSystem : public Base {
private:
    std::vector<Planet*> m_planets;
    Sun* m_sun;

public:
    SolarSystem(double sunMass, double sunDiameter, unsigned int sunColor,
                std::vector<double>& planetMass, QStringList& planetName,
                std::vector<double>& planetDiameter, std::vector<unsigned int>& planetColor,
                std::vector<double>& X, std::vector<double>& Y, std::vector<double>& Z, double G);
    virtual ~SolarSystem();

	void clear();
    int size();
    Planet* at(int idx);

	void update(double dt);	
    Sun* getSun (void);
};

