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
//  NED.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018. Based on cola2_lib by udg_cirs.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "core/NED.h"

namespace sf
{

// WGS-84
const Scalar NED::a = Scalar(6378137);
const Scalar NED::b = Scalar(6356752.3142);
const Scalar NED::esq = Scalar(6.69437999014 * 0.001);
const Scalar NED::e1sq = Scalar(6.73949674228 * 0.001); 
const Scalar NED::f = Scalar(1.0 / 298.257223563);

NED::NED()
{
    Init(50.0, 20.0, 0.0); //Krakow, Poland
}

void NED::Init(const Scalar lat, const Scalar lon, const Scalar height)
{
    // Save NED origin
    _init_lat = lat/Scalar(180) * M_PI;
    _init_lon = lon/Scalar(180) * M_PI;
    _init_h = height;

    // Compute ECEF of NED origin
    Geodetic2Ecef(lat, lon, height, _init_ecef_x, _init_ecef_y, _init_ecef_z);

    // Compute ECEF to NED and NED to ECEF matrices
    Scalar phiP = btAtan2(_init_ecef_z, btSqrt(_init_ecef_x*_init_ecef_x + _init_ecef_y*_init_ecef_y));

    _ecef_to_ned_matrix = __nRe__(phiP, _init_lon);
    _ned_to_ecef_matrix = __nRe__(_init_lat, _init_lon).transpose();
}

void NED::Geodetic2Ecef(const Scalar lat, const Scalar lon, const Scalar height,
                        Scalar& x, Scalar& y, Scalar& z) const
{
    // Convert geodetic coordinates to ECEF.
    // http://code.google.com/p/pysatel/source/browse/trunk/coord.py?r=22
    Scalar lat_rad = lat/180.0 * M_PI;
    Scalar lon_rad = lon/180.0 * M_PI;
    Scalar xi = btSqrt(1 - esq * btSin(lat_rad) * btSin(lat_rad));
    x = (a / xi + height) * btCos(lat_rad) * btCos(lon_rad);
    y = (a / xi + height) * btCos(lat_rad) * btSin(lon_rad);
    z = (a / xi * (1 - esq) + height) * btSin(lat_rad);
}

void NED::Ecef2Geodetic(const Scalar x, const Scalar y, const Scalar z,
                        Scalar& lat, Scalar& lon, Scalar& height) const
{
    // Convert ECEF coordinates to geodetic.
    // J. Zhu, "Conversion of Earth-centered Earth-fixed coordinates
    // to geodetic coordinates," IEEE Transactions on Aerospace and
    // Electronic Systems, vol. 30, pp. 957-961, 1994.
    Scalar r = btSqrt(x * x + y * y);
    Scalar Esq = a * a - b * b;
    Scalar F = 54 * b * b * z * z;
    Scalar G = r * r + (1 - esq) * z * z - esq * Esq;
    Scalar C = (esq * esq * F * r * r) / btPow(G, 3);
    Scalar S = __cbrt__(1 + C + btSqrt(C * C + 2 * C));
    Scalar P = F / (3 * btPow((S + 1 / S + 1), 2) * G * G);
    Scalar Q = btSqrt(1 + 2 * esq * esq * P);
    Scalar r_0 = -(P * esq * r) / (1 + Q) + btSqrt(0.5 * a * a * (1 + 1.0 / Q) - P * (1 - esq) * z * z / (Q * (1 + Q)) - 0.5 * P * r * r);
    Scalar U = btSqrt(btPow((r - esq * r_0), 2) + z * z);
    Scalar V = btSqrt(btPow((r - esq * r_0), 2) + (1 - esq) * z * z);
    Scalar Z_0 = b * b * z / (a * V);
    height = U * (1 - b * b / (a * V));
    lat = (btAtan((z + e1sq * Z_0) / r))/M_PI * 180.0;
    lon = (btAtan2(y, x))/M_PI * 180.0;
}

void NED::Ecef2Ned(const Scalar x, const Scalar y, const Scalar z,
                   Scalar& north, Scalar& east, Scalar& depth) const
{
    // Converts ECEF coordinate pos into local-tangent-plane ENU
    // coordinates relative to another ECEF coordinate ref. Returns a tuple
    // (East, North, Up).
    Vector3 vect, ret;
    vect.setX(x - _init_ecef_x);
    vect.setY(y - _init_ecef_y);
    vect.setZ(z - _init_ecef_z);
    ret = _ecef_to_ned_matrix * vect;
    north = ret.getX();
    east = ret.getY();
    depth = -ret.getZ();
}

void NED::Ned2Ecef(const Scalar north, const Scalar east, const Scalar depth,
                   Scalar& x, Scalar& y, Scalar& z) const
{
    // NED (north/east/down) to ECEF coordinate system conversion.
    Vector3 ned, ret;
    ned.setX(north);
    ned.setY(east);
    ned.setZ(-depth);
    ret = _ned_to_ecef_matrix * ned;
    x = ret.getX() + _init_ecef_x;
    y = ret.getY() + _init_ecef_y;
    z = ret.getZ() + _init_ecef_z;
}

void NED::Geodetic2Ned(const Scalar lat, const Scalar lon, const Scalar height,
                       Scalar& north, Scalar& east, Scalar& depth) const
{
    // Geodetic position to a local NED system """
    Scalar x, y, z;
    Geodetic2Ecef(lat, lon, height, x, y, z);
    Ecef2Ned(x, y, z, north, east, depth);
}

void NED::Ned2Geodetic(const Scalar north, const Scalar east, const Scalar depth,
                       Scalar& lat, Scalar& lon, Scalar& height) const
{
    // Local NED position to geodetic
    Scalar x, y, z;
    Ned2Ecef(north, east, depth, x, y, z);
    Ecef2Geodetic(x, y, z, lat, lon, height);
}

Scalar NED::__cbrt__(const Scalar x) const
{
    if(x >= 0.0)
        return btPow(x, 1.0/3.0);
    else
        return -btPow(btFabs(x), 1.0/3.0);
}

Matrix3 NED::__nRe__(const Scalar lat_rad, const Scalar lon_rad) const
{
    Scalar sLat = btSin(lat_rad);
    Scalar sLon = btSin(lon_rad);
    Scalar cLat = btCos(lat_rad);
    Scalar cLon = btCos(lon_rad);

    Matrix3 ret(-sLat*cLon, -sLat*sLon, cLat,
                         -sLon,       cLon,    0,
                     cLat*cLon,  cLat*sLon, sLat);

    return ret;
}

}