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
//  Comm.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/02/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Comm__
#define __Stonefish_Comm__

#include <SDL2/SDL_mutex.h>
#include "StonefishCommon.h"

namespace sf
{
    //! An enum defining types of comms.
    typedef enum {COMM_RADIO = 0, COMM_ACOUSTIC, COMM_VLC} CommType;
    
    struct Renderable;
    class SolidEntity;
    
    //! An abstract class representing a communication device.
    class Comm
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the comm device
         \param deviceId an identification code of the device
         \param frequency the update frequency of the comm [Hz] (-1 if updated every simulation step)
         */
        Comm(std::string uniqueName, uint64_t deviceId, Scalar frequency);
        
        //! A destructor.
        virtual ~Comm();
        
        //! A method to set the connected device id.
        /*!
         \param deviceId an identifier of the connected node
         */
        void Connect(uint64_t deviceId);
        
        //! A method used to attach the comm device to a rigid body.
        /*!
         \param solid a pointer to the rigid body
         \param origin the place where the sensor should be attached in the solid origin frame
         */
        void AttachToSolid(SolidEntity* solid, const Transform& origin);
        
        //! A method implementing the rendering of the comm device.
        virtual std::vector<Renderable> Render();
        
        //! A method that updates the comm state.
        /*!
         \param dt a time step of the simulation [s]
         */
        void Update(Scalar dt);
        
        //! A method used to mark data as old.
        void MarkDataOld();
        
        //! A method to check if new data is available.
        bool isNewDataAvailable();
        
        //! A method to set the update rate of the comm.
        /*!
         \param f the update frequency of the comm [Hz]
         */
        void setUpdateFrequency(Scalar f);
        
        //! A method informing if the comm is renderable.
        bool isRenderable();
        
        //! A method to set if the comm is renderable.
        void setRenderable(bool render);
        
        //! A method returning the current comm device frame in world.
        Transform getDeviceFrame();
        
        //! A method returning the device node id.
        uint64_t getDeviceId();
        
        //! A method returning the conneted device node id.
        uint64_t getConnectedId();
        
        //! A method returning the comm name.
        std::string getName();
        
        //! A method performing an internal update of the comm state.
        /*!
         \param dt the time step of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method returning the type of the comm.
        virtual CommType getType() = 0;
        
    private:
        std::string name;
        uint64_t id;
        uint64_t cId;
        Scalar freq;
        SDL_mutex* updateMutex;
        SolidEntity* attach;
        Transform o2c;
        Scalar eleapsedTime;
        bool renderable;
        bool newDataAvailable;
    };
}

#endif
