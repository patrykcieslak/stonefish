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
//  OpticalModem.h
//  Stonefish
//
//  Created by Patryk Cieślak on 14/05/2025.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpticalModem__
#define __Stonefish_OpticalModem__

#include <map>
#include "comms/Comm.h"

namespace sf
{
    //! An abstract class representing a visual light communication modem.
    class OpticalModem : public Comm
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the comm device
         \param deviceId an identification code of the device
         \param fovDeg field of view of the diodes (conical) [deg]
         \param operatingRange the maximum ideal operating range [m]
         \param ambientLightSensitivity an empirical unitless factor [0.0, 1.0] specifying how ambient light impacts the device operation
         */
        OpticalModem(std::string uniqueName, uint64_t deviceId, Scalar fovDeg, Scalar operatingRange, Scalar ambientLightSensitivity);
        
        //! A destructor.
        virtual ~OpticalModem();

        //! A method performing internal comm state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt);
        
        //! A method implementing the rendering of the comm device.
        std::vector<Renderable> Render();
        
        //! A method returnign the reception quality.
        Scalar getReceptionQuality() const;

        //! A method returning the type of the comm.
        virtual CommType getType() const;
        
    protected:
        virtual void MessageReceived(std::shared_ptr<CommDataFrame> message);
        bool isReceptionPossible(Vector3 worldDir, Scalar distance);
        
        static OpticalModem* getNode(uint64_t deviceId);
        static std::vector<uint8_t> introduceErrors(const std::vector<uint8_t>& data, Scalar linkQuality);
        
    private:
        Scalar maxRange;
        Scalar fov;
        Scalar ambientLightSens;
        Scalar receptionQuality;
        Scalar trueRange;
        
        static void addNode(OpticalModem* node);
        static void removeNode(uint64_t deviceId);
        static std::vector<uint64_t> getNodeIds();

        static std::map<uint64_t, OpticalModem*> nodes;
    };
}
    
#endif