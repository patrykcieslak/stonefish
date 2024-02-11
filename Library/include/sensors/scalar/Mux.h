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
//  Mux.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Mux__
#define __Stonefish_Mux__

#include "StonefishCommon.h"

namespace sf
{
    class ScalarSensor;
    
    //! A structure containing data of one mux component.
    struct MuxComponent
    {
        ScalarSensor* sensor;
        unsigned short channel;
    };
   
    //! A class implementing a multiplexer.
    class Mux
    {
    public:
        //! A constructor.
        Mux();
        
        //! A destructor.
        ~Mux();
        
        //! A method to add a sensor channel to the mux.
        /*!
         \param s a pointer to a scalar sensor
         \param channel an index of the sensor channel
         \return if channel successfully added
         */
        bool AddComponent(ScalarSensor* s, unsigned short channel);
        
        //! A method returning a pointer to a mux channel.
        /*!
         \param index an index of the mux channel
         \return a pointer to a mux component
         */
        MuxComponent* getComponent(unsigned int index);
        
        //! A method returning the last sample.
        Scalar* getLastSample();
        
        //! A method returning a number of channels of the mux.
        unsigned int getNumOfComponents();
        
    private:
        std::vector<MuxComponent> components;
    };
}

#endif
