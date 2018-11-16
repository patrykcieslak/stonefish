//
//  NED.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018. Based on cola2_lib by udg_cirs.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/NED.h"

const btScalar NED::a = btScalar(6378137);
const btScalar NED::b = btScalar(6356752.3142);
const btScalar NED::esq = btScalar(6.69437999014 * 0.001);
const btScalar NED::e1sq = btScalar(6.73949674228 * 0.001);
const btScalar NED::f = btScalar(1.0 / 298.257223563);

NED::NED(const btScalar lat, const btScalar lon, const btScalar height)
{
	// Save NED origin
	_init_lat = lat/180.0 * M_PI;
	_init_lon = lon/180.0 * M_PI;
	_init_h = height;

	// Compute ECEF of NED origin
	geodetic2Ecef(lat, lon, height, _init_ecef_x, _init_ecef_y, _init_ecef_z);

	// Compute ECEF to NED and NED to ECEF matrices
	btScalar phiP = btAtan2(_init_ecef_z, btSqrt(_init_ecef_x*_init_ecef_x + _init_ecef_y*_init_ecef_y));

	_ecef_to_ned_matrix = __nRe__(phiP, _init_lon);
	_ned_to_ecef_matrix = __nRe__(_init_lat, _init_lon).transpose();
}

void NED::geodetic2Ecef(const btScalar lat, const btScalar lon, const btScalar height,
                        btScalar& x, btScalar& y, btScalar& z)
{
	// Convert geodetic coordinates to ECEF.
	// http://code.google.com/p/pysatel/source/browse/trunk/coord.py?r=22
	btScalar lat_rad = lat/180.0 * M_PI;
	btScalar lon_rad = lon/180.0 * M_PI;
	btScalar xi = btSqrt(1 - esq * btSin(lat_rad) * btSin(lat_rad));
	x = (a / xi + height) * btCos(lat_rad) * btCos(lon_rad);
	y = (a / xi + height) * btCos(lat_rad) * btSin(lon_rad);
	z = (a / xi * (1 - esq) + height) * btSin(lat_rad);
}

void NED::ecef2Geodetic(const btScalar x, const btScalar y, const btScalar z,
                        btScalar& lat, btScalar& lon, btScalar& height)
{
	// Convert ECEF coordinates to geodetic.
	// J. Zhu, "Conversion of Earth-centered Earth-fixed coordinates
	// to geodetic coordinates," IEEE Transactions on Aerospace and
	// Electronic Systems, vol. 30, pp. 957-961, 1994.
	btScalar r = btSqrt(x * x + y * y);
	btScalar Esq = a * a - b * b;
	btScalar F = 54 * b * b * z * z;
	btScalar G = r * r + (1 - esq) * z * z - esq * Esq;
	btScalar C = (esq * esq * F * r * r) / btPow(G, 3);
	btScalar S = __cbrt__(1 + C + btSqrt(C * C + 2 * C));
	btScalar P = F / (3 * btPow((S + 1 / S + 1), 2) * G * G);
	btScalar Q = btSqrt(1 + 2 * esq * esq * P);
	btScalar r_0 = -(P * esq * r) / (1 + Q) + btSqrt(0.5 * a * a * (1 + 1.0 / Q) - P * (1 - esq) * z * z / (Q * (1 + Q)) - 0.5 * P * r * r);
	btScalar U = btSqrt(btPow((r - esq * r_0), 2) + z * z);
	btScalar V = btSqrt(btPow((r - esq * r_0), 2) + (1 - esq) * z * z);
	btScalar Z_0 = b * b * z / (a * V);
	height = U * (1 - b * b / (a * V));
	lat = (btAtan((z + e1sq * Z_0) / r))/M_PI * 180.0;
	lon = (btAtan2(y, x))/M_PI * 180.0;
}

void NED::ecef2Ned(const btScalar x, const btScalar y, const btScalar z,
                   btScalar& north, btScalar& east, btScalar& depth)
{
	// Converts ECEF coordinate pos into local-tangent-plane ENU
	// coordinates relative to another ECEF coordinate ref. Returns a tuple
	// (East, North, Up).
	btVector3 vect, ret;
	vect.setX(x - _init_ecef_x);
	vect.setY(y - _init_ecef_y);
	vect.setZ(z - _init_ecef_z);
	ret = _ecef_to_ned_matrix * vect;
	north = ret.getX();
	east = ret.getY();
	depth = -ret.getZ();
}

void NED::ned2Ecef(const btScalar north, const btScalar east, const btScalar depth,
                   btScalar& x, btScalar& y, btScalar& z)
{
	// NED (north/east/down) to ECEF coordinate system conversion.
	btVector3 ned, ret;
	ned.setX(north);
	ned.setY(east);
	ned.setZ(-depth);
	ret = _ned_to_ecef_matrix * ned;
	x = ret.getX() + _init_ecef_x;
	y = ret.getY() + _init_ecef_y;
	z = ret.getZ() + _init_ecef_z;
}

void NED::geodetic2Ned(const btScalar lat, const btScalar lon, const btScalar height,
                       btScalar& north, btScalar& east, btScalar& depth)
{
	// Geodetic position to a local NED system """
	btScalar x, y, z;
	geodetic2Ecef(lat, lon, height, x, y, z);
	ecef2Ned(x, y, z, north, east, depth);
}

void NED::ned2Geodetic(const btScalar north, const btScalar east, const btScalar depth,
                       btScalar& lat, btScalar& lon, btScalar& height)
{
	// Local NED position to geodetic
	btScalar x, y, z;
	ned2Ecef(north, east, depth, x, y, z);
	ecef2Geodetic(x, y, z, lat, lon, height);
}

btScalar NED::__cbrt__(const btScalar x)
{
	if(x >= 0.0)
		return btPow(x, 1.0/3.0);
	else
		return -btPow(btFabs(x), 1.0/3.0);
}

btMatrix3x3 NED::__nRe__(const btScalar lat_rad, const btScalar lon_rad)
{
	btScalar sLat = btSin(lat_rad);
	btScalar sLon = btSin(lon_rad);
	btScalar cLat = btCos(lat_rad);
	btScalar cLon = btCos(lon_rad);

	btMatrix3x3 ret(-sLat*cLon, -sLat*sLon, cLat,
						 -sLon,       cLon,    0,
				     cLat*cLon,  cLat*sLon, sLat);

    return ret;
}
