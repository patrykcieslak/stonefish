#ifndef BATTERY_H
#define BATTERY_H

#include <vector>
#include "sensors/Sensor.h"
#include <iostream>

namespace sf
{

class Battery
{
private:
    float voltage;         // Battery voltage in volts
    float capacity;        // Battery capacity in ampere-hours (Ah)
    float maxCapacity;     // Maximum battery capacity in ampere-hours
    double powerDraw;       // Current power draw in watts (calculated)
    double energyRemaining; // Energy remaining in watt-hours (Wh)

public:
    Battery(float voltage, float capacity);

    // Copy Constructor
    Battery(const Battery& other);

    // Assignment Operator
    Battery& operator=(const Battery& other);
    
    float getVoltage() const;
    float getCapacity() const;
    double getEnergyRemaining() const;
    void consume(const std::vector<Sensor*>& sensors, float timeStep); // timeStep in seconds
    void setVoltage(Scalar v);
    void setMaxCapacity(Scalar mc);
};

} // namespace sf

#endif // BATTERY_H

