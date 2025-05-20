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
//  Sample.h
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014-2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sample__
#define __Stonefish_Sample__

#include "StonefishCommon.h"

namespace sf
{
    //! A class representing a single measurement.
    class Sample
    {
    public:
        //! A constructor.
        /*!
         \param data a vector of values of the measurement
         \param invalid a flag to mark if it is and invalid output
         \param index a number specifying the id of the sample
         */
        Sample(const std::vector<Scalar>& data, bool invalid = false, uint64_t index = 0);
        
        //! A copy constructor.
        /*!
         \param other a reference to a sample object
         \param index a number specifying the id of the sample
         */
        Sample(const Sample& other, uint64_t index = 0);
        
        //! A method returning the timestamp of the sample.
        Scalar getTimestamp() const;
        
        //! A method returning a value of the single dimension of the measurement.
        /*!
         \param dimension the index of the dimension
         \return the value of the measurement for the dimension
         */
        Scalar getValue(size_t dimension) const;
        
        //! A method returning the full data of the measurement.
        std::vector<Scalar> getData() const;
        
        //! A method returning the number of dimensions of the measurement.
        size_t getNumOfDimensions() const;
        
        //! A method returning a pointer to the sample data.
        Scalar* getDataPointer();

        //! A method returning the id of the sample.
        uint64_t getId() const;
        
    private:
        Scalar timestamp;
        std::vector<Scalar> data;
        uint64_t id;
    };
}

#endif
