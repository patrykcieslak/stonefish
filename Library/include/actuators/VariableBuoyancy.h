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
//  VariableBuoyancy.h
//  Stonefish
//
//  Created by Patryk Cieslak on 07/11/2019.
//  Copyright (c) 2019-2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_VariableBuoyancy__
#define __Stonefish_VariableBuoyancy__

#include "actuators/LinkActuator.h"
#include "utils/GeometryFileUtil.h"

namespace sf
{
    //! A class implementing a variable buoyancy system (VBS).
    class VariableBuoyancy : public LinkActuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the VBS
         \param volumeMeshPath a list of paths to the meshes representing different states of the buoyancy volume
         \param initialVolume an initial state of the VBS
         */
        VariableBuoyancy(std::string uniqueName, const std::vector<std::string>& volumeMeshPaths, Scalar initialVolume);
        
        //! A method used to update the internal state of the actuator.
        /*!
         \param dt the time step of the simulation [s]
         */
        void Update(Scalar dt);
        
        //! A method implementing the rendering of the VBS.
        std::vector<Renderable> Render();
        
        //! A method used to set the desired flow rate setpoint.
        /*!
         \param rate the desired flow rate of the VBS [m3/s]
          */
        void setFlowRate(Scalar rate);
        
        //! A method returning the current flow rate setpoint.
        Scalar getFlowRate() const;
        
        //! A method returning the current volume of the liquid in the system.
        Scalar getLiquidVolume() const;

        //! A method returning the generated force.
        Scalar getForce() const;
        
        //! A method returning the type of the actuator.
        ActuatorType getType() const;
        
    private:
        void InterpolateVProps(Scalar volume, Scalar& m, Vector3& cg);
    
        Scalar V;
        Vector3 CG;
        Scalar Vmin;
        Scalar Vmax;
        Scalar flowRate;
        Scalar density;
        Vector3 force;
        Vector3 gravity;
        std::vector<MeshProperties> Vprops;
    };
}

#endif