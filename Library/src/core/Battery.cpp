#include "core/Battery.h"
#include <iostream>
#include <iomanip>

namespace sf
{

Battery::Battery(float voltage, float capacity)
    : voltage(voltage), capacity(capacity), maxCapacity(capacity),
      energyRemaining(100) // Initial energy in Wh
{

      std::cout<<"Remaining energy:"<<energyRemaining<<std::endl;
}

// Copy Constructor
Battery::Battery(const Battery& other)
    : maxCapacity(other.maxCapacity), voltage(other.voltage) {
    // Perform a deep copy if there are dynamic members
}

// Assignment Operator
Battery& Battery::operator=(const Battery& other) {
    if (this != &other) { // Prevent self-assignment
        maxCapacity = other.maxCapacity;
        voltage = other.voltage;
        // Deep copy any dynamic members if necessary
    }
    return *this;
}

float Battery::getVoltage() const
{
    return voltage;
}

float Battery::getCapacity() const
{
    return capacity;
}

void Battery::setVoltage(Scalar v) 
{
    voltage=v;
}

void Battery::setMaxCapacity(Scalar mc) 
{ 
    maxCapacity=mc;
    capacity=mc;
}

double Battery::getEnergyRemaining() const
{
    return energyRemaining;
}

void Battery::consume(const std::vector<Sensor*>& sensors, float timeStep)
{
    powerDraw = 0.0f; // Reset power draw before calculation

    // Calculate total power draw from sensors
    for (const auto& sensor : sensors)
    {
        double sensorVoltage = sensor->getVoltage();  // Voltage required by the sensor
        double sensorWattage = sensor->getPower();    // Power required by the sensor in watts
        double dutyCycle = sensor->getDutyCycle();    // Duty cycle as a percentage (0-100)

        double sensorPower = (sensorWattage * dutyCycle) / 100.0; // Adjust power based on duty cycle
        powerDraw += sensorPower;
    }

    // Calculate energy consumed based on power draw and time step
    double totalEnergyCapacity = capacity * voltage; // Total energy capacity in watt-hours
    double energyConsumedWh = powerDraw * (timeStep  / 3600.0); // Energy consumed in watt-hours

    // Convert energy consumed to a percentage of total capacity
    double energyConsumedPercent = (energyConsumedWh / totalEnergyCapacity) * 100.0;

    // Deduct the consumed energy percentage from the remaining energy
    energyRemaining -= energyConsumedPercent;

    // Prevent energy from going below 0%
    if (energyRemaining < 0.0)
        energyRemaining = 0.0;

    // Print battery status with 10 decimal places
    //std::cout << std::fixed << std::setprecision(15); // Set fixed-point notation and 10 decimal places
    //std::cout << "Battery Status: Voltage = " << voltage
    //          << "V, Capacity = " << capacity << "Ah, Energy Remaining = "
    //          << energyRemaining << "%, Power Draw = "
    //          << powerDraw << "W, energyConsumed percent "
    //          << energyConsumedPercent << "%\n";
}


} // namespace sf

