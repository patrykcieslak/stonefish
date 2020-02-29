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
//  AcousticModem.h
//  Stonefish
//
//  Created by Patryk Cieślak on 26/02/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_AcousticModem__
#define __Stonefish_AcousticModem__

#include <map>
#include "comms/Comm.h"

namespace sf
{
    //! An abstract class representing an acoustic modem.
    class AcousticModem : public Comm
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the comm device
         \param deviceId an identification code of the device
         \param horizontalFOVDeg the horizontal beam angle [deg]
         \param verticalFOVDeg the vertical beam angle [deg]
         \param operatingRange the operating range [m]
         \param frequency the update frequency of the comm device [Hz] (-1 if updated every simulation step)
         */
        AcousticModem(std::string uniqueName, uint64_t deviceId, Scalar horizontalFOVDeg, Scalar verticalFOVDeg, Scalar operatingRange, Scalar frequency = Scalar(-1));
        
        //! A destructor.
        virtual ~AcousticModem();
        
        //! A method performing internal comm state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt);
        
        //! A method used to update position of the modem based on measurements from USBL or another device.
        /*!
         \param pos Cartesian position [m]
         \param absolute a flag to indicate if the position is absolute
         \param referenceFrame name of the frame that the position is defined in
         */
        void UpdatePosition(Vector3 pos, bool absolute, std::string referenceFrame = std::string(""));
        
        //! A method implementing the rendering of the comm device.
        std::vector<Renderable> Render();
        
        //! A method to retrieve the position of the device in the designated reference frame.
        /*!
         \param pos a pointer to the position vector
         \param referenceFrame a pointer to the name of the reference frame
         */
        void getPosition(Vector3& pos, std::string& referenceFrame);
        
        //! A method used to check if the device can talk to the connected device. 
        bool isConnectionAlive();
        
        //! A method returning the type of the comm.
        virtual CommType getType();
        
    protected:
        static AcousticModem* getNode(uint64_t deviceId);
        
    private:
        Scalar range;
        Scalar hFov2, vFov2;
        Vector3 position;
        std::string frame;
        bool connection;
        
        bool isReceptionPossible(Vector3 dir, Scalar distance);
        
        static std::map<uint64_t, AcousticModem*> nodes;
        static void addNode(AcousticModem* node);
        static void removeNode(uint64_t deviceId);
        static bool mutualContact(uint64_t device1Id, uint64_t device2Id);
    };
}
    
#endif