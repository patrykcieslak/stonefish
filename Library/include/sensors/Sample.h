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
//  Copyright (c) 2014-2021 Patryk Cieslak. All rights reserved.
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
         \param nDimensions the number of dimensions of the measurement
         \param values a pointer to the data
         \param invalid a flag to mark if it is and invalid output
         */
        Sample(unsigned short nDimensions, Scalar* values, bool invalid = false);
        
        //! A copy constructor.
        /*!
         \param other a reference to a sample object
         */
        Sample(const Sample& other);
        
        //! A destructor.
        ~Sample();
        
        //! A method returning the timestamp of the sample.
        Scalar getTimestamp() const;
        
        //! A method returning a value of the single dimension of the measurement.
        /*!
         \param dimension the index of the dimension
         \return the value of the measurement for the dimension
         */
        Scalar getValue(unsigned short dimension) const;
        
        //! A method returning the full data of the measurement.
        std::vector<Scalar> getData() const;
        
        //! A method returning the number of dimensions of the measurement.
        unsigned short getNumOfDimensions() const;
        
        //! A method returning a pointer to the sample data.
        Scalar* getDataPointer();
        
    private:
        Scalar timestamp;
        unsigned short nDim;
        Scalar* data;
    };
}

#endif
