//
//  Polynomials.h
//  Stonefish
//
//  Created by Patryk Cieslak on 23/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Polynomials__
#define __Stonefish_Polynomials__

#include <LinearMath/btMatrixX.h>

namespace sf
{

/*! Polynomial described by coefficient vector */
class Polynomial
{
public:
    Polynomial(const btVectorXu& coeffs) : c(coeffs), order(coeffs.size()-1) {}
    
    Scalar Value(Scalar x)
    {
        Scalar xPower = Scalar(1.);
        Scalar value = Scalar(0.);
        
        for(int i = 0; i <= order; ++i)
        {
            value += c[order - i] * xPower;
            xPower *= x;
        }
        
        return value;
    }
    
private:
    btVectorXu c;
    unsigned int order;
};

/*! Polynomial surface described by coefficient matrix */
class PolySurface
{
public:
    PolySurface(const btMatrixXu& coeffs) : C(coeffs), orderX(coeffs.cols()-1), orderY(coeffs.rows()-1) {}
    
    Scalar Value(Scalar x, Scalar y)
    {
        btVectorXu xPowers(orderX + 1);
        xPowers[0] = Scalar(1.);
        for(int i = 1; i <= orderX; ++i)
            xPowers[i] = xPowers[i-1] * x;
        
        btVectorXu yPowers(orderY + 1);
        yPowers[0] = Scalar(1.);
        for(int i = 1; i <= orderY; ++i)
            yPowers[i] = yPowers[i-1] * y;
        
        Scalar value = Scalar(0.);
        
        for(int i = 0; i <= orderY; ++i)
        {
            Scalar yPoly = Scalar(0.);
            
            for(int h = 0; h <= orderX; ++h)
                yPoly += C(i,h) * xPowers[orderX - h];
            
            value += yPoly * yPowers[orderY - i];
        }
        
        return value;
    }
    
private:
    btMatrixXu C;
    unsigned int orderX;
    unsigned int orderY;
};

}
    
#endif
