//
//  Sample.h
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
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
         */
        Sample(unsigned short nDimensions, Scalar* values);
        
        //! A copy constructor.
        /*!
         \param other a reference to a sample object
         */
        Sample(const Sample& other);
        
        //! A destructor.
        ~Sample();
        
        //! A method returning the timestamp of the sample.
        Scalar getTimestamp();
        
        //! A method returning a value of the single dimension of the measurement.
        /*!
         \param dimension the index of the dimension
         \return the value of the measurement for the dimension
         */
        Scalar getValue(unsigned short dimension);
        
        //! A method returning the full data of the measurement.
        std::vector<Scalar> getData();
        
        //! A method returning the number of dimensions of the measurement.
        unsigned short getNumOfDimensions();
        
        //! A method returning a pointer to the sample data.
        Scalar* getDataPointer();
        
    private:
        Scalar timestamp;
        unsigned short nDim;
        Scalar* data;
    };
}

#endif
