//
//  NED.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018. Based on cola2_lib by udg_cirs.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_NED__
#define __Stonefish_NED__

#include "common.h"

class NED
{
 public:
    NED(const btScalar lat, const btScalar lon, const btScalar height);

    void geodetic2Ecef(const btScalar lat, const btScalar lon, const btScalar height,
                       btScalar& x, btScalar& y, btScalar& z);

    void ecef2Geodetic(const btScalar x, const btScalar y, const btScalar z,
                       btScalar& lat, btScalar& lon, btScalar& height);

    void ecef2Ned(const btScalar x, const btScalar y, const btScalar z,
                  btScalar& north, btScalar& east, btScalar& depth);

    void ned2Ecef(const btScalar north, const btScalar east, const btScalar depth,
                  btScalar& x, btScalar& y, btScalar& z);

    void geodetic2Ned(const btScalar lat, const btScalar lon, const btScalar height,
                      btScalar& north, btScalar& east, btScalar& depth);

    void ned2Geodetic(const btScalar north, const btScalar east, const btScalar depth,
                      btScalar& lat, btScalar& lon, btScalar& height);

 private:
    btScalar _init_lat;
    btScalar _init_lon;
    btScalar _init_h;
    btScalar _init_ecef_x;
    btScalar _init_ecef_y;
    btScalar _init_ecef_z;
    btMatrix3x3 _ecef_to_ned_matrix;
    btMatrix3x3 _ned_to_ecef_matrix;

    btScalar __cbrt__(const btScalar x);
    btMatrix3x3 __nRe__(const btScalar lat_rad, const btScalar lon_rad);
	
	//World Geodetic System 1984 (WGS84)
	static const btScalar a;
	static const btScalar b;
	static const btScalar esq;
	static const btScalar e1sq;
	static const btScalar f;
};

#endif
