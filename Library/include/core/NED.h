//
//  NED.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018. Based on cola2_lib by udg_cirs.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_NED__
#define __Stonefish_NED__

#include "StonefishCommon.h"

namespace sf
{
    //! A class representing the North-East-Depth (Down) coordinate system.
    class NED
    {
    public:
        NED(const Scalar lat, const Scalar lon, const Scalar height);
        
        void geodetic2Ecef(const Scalar lat, const Scalar lon, const Scalar height,
                           Scalar& x, Scalar& y, Scalar& z);
        
        void ecef2Geodetic(const Scalar x, const Scalar y, const Scalar z,
                           Scalar& lat, Scalar& lon, Scalar& height);
        
        void ecef2Ned(const Scalar x, const Scalar y, const Scalar z,
                      Scalar& north, Scalar& east, Scalar& depth);
        
        void ned2Ecef(const Scalar north, const Scalar east, const Scalar depth,
                      Scalar& x, Scalar& y, Scalar& z);
        
        void geodetic2Ned(const Scalar lat, const Scalar lon, const Scalar height,
                          Scalar& north, Scalar& east, Scalar& depth);
        
        void ned2Geodetic(const Scalar north, const Scalar east, const Scalar depth,
                          Scalar& lat, Scalar& lon, Scalar& height);
        
    private:
        Scalar _init_lat;
        Scalar _init_lon;
        Scalar _init_h;
        Scalar _init_ecef_x;
        Scalar _init_ecef_y;
        Scalar _init_ecef_z;
        Matrix3 _ecef_to_ned_matrix;
        Matrix3 _ned_to_ecef_matrix;
        
        Scalar __cbrt__(const Scalar x);
        Matrix3 __nRe__(const Scalar lat_rad, const Scalar lon_rad);
        
        //World Geodetic System 1984 (WGS84)
        static const Scalar a;
        static const Scalar b;
        static const Scalar esq;
        static const Scalar e1sq;
        static const Scalar f;
    };
}

#endif
