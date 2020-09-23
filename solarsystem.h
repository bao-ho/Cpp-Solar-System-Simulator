 #pragma once

#include <vector>

#include "base.h"
#include "planet.h"
#include "sun.h"

class SolarSystem : public Base {
private:
    std::vector<Planet*> m_planets;
    Sun* m_sun;

public:
    SolarSystem(int n = 100);
    SolarSystem(int n, double sunMass, double* pMass, double* pX, double* pY, double* pZ, double G);
    virtual ~SolarSystem();

	void clear();
    int size();
    Planet* at(int idx);

	void update(double dt);	
    Sun* getSun (void);
};

