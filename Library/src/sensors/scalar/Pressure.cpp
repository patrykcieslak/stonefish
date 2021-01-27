/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  Pressure.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Pressure.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "sensors/Sample.h"

namespace sf
{

Pressure::Pressure(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Pressure", QuantityType::PRESSURE));
}

void Pressure::InternalUpdate(Scalar dt)
{
    Scalar data(0.); //Gauge pressure //data(101325.); //Pa (1 atm)
    
    Ocean* liq = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(liq != NULL)
        data += liq->GetPressure(getSensorFrame().getOrigin());
    
    //Record sample
    Sample s(1, &data);
    AddSampleToHistory(s);
}

void Pressure::setRange(Scalar max)
{
    channels[0].rangeMin = Scalar(0);
    channels[0].rangeMax = btClamped(max, Scalar(0), Scalar(BT_LARGE_FLOAT));
}
    
void Pressure::setNoise(Scalar pressureStdDev)
{
    channels[0].setStdDev(btClamped(pressureStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Pressure::getScalarSensorType()
{
    return ScalarSensorType::PRESSURE;
}

}
