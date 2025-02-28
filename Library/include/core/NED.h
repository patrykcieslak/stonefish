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
        //! A constructor.
        NED();
        
        //! A method initializing the NED matrices.
        /*!
         \param lat the latitude of the NED origin [deg]
         \param lon the longitude of the NED origin [deg]
         \param height the height above the ground [m]
         */
        void Init(const Scalar lat, const Scalar lon, const Scalar height);
        
        //! A method transforming the geodetic coordinates to ECEF.
        /*!
         \param lat the latitude [deg]
         \param lon the longitude [deg]
         \param height the height above the ground [m]
         \param x coordinate output
         \param y coordinate output
         \param z coordinate output
         */
        void Geodetic2Ecef(const Scalar lat, const Scalar lon, const Scalar height,
                           Scalar& x, Scalar& y, Scalar& z) const;
        
        //! A method transforming from ECEF to the geodetic coordinates.
        /*!
         \param x coordinate
         \param y coordinate
         \param z coordinate
         \param lat the latitude output [deg]
         \param lon the longitude output [deg]
         \param height the height output [m]
         */
        void Ecef2Geodetic(const Scalar x, const Scalar y, const Scalar z,
                           Scalar& lat, Scalar& lon, Scalar& height) const;
        
        //! A method transforming from ECEF to NED.
        /*!
         \param x coordinate
         \param y coordinate
         \param z coordinate
         \param north output [m]
         \param east output [m]
         \param depth output [m]
         */
        void Ecef2Ned(const Scalar x, const Scalar y, const Scalar z,
                      Scalar& north, Scalar& east, Scalar& depth) const;
        
        //! A method transforming from NED to ECEF.
        /*!
         \param north coordinate [m]
         \param east coordinate [m]
         \param depth coordinate [m]
         \param x coordinate output
         \param y coordinate output
         \param z coordinate output
         */
        void Ned2Ecef(const Scalar north, const Scalar east, const Scalar depth,
                      Scalar& x, Scalar& y, Scalar& z) const;
        
        //! A method transforming from geodetic coordinates to NED.
        /*!
         \param lat the latitude [deg]
         \param lon the longitude [deg]
         \param height the height above the ground [m]
         \param north coordinate output [m]
         \param east coordinate output [m]
         \param depth coordinate output [m]
         */
        void Geodetic2Ned(const Scalar lat, const Scalar lon, const Scalar height,
                          Scalar& north, Scalar& east, Scalar& depth) const;
        
        //! A method transforming from NED to the geodetic coordinates.
        /*!
         \param north coordinate [m]
         \param east coordinate [m]
         \param depth coordinate [m]
         \param lat the latitude output [deg]
         \param lon the longitude output [deg]
         \param height the height output [m]
         */
        void Ned2Geodetic(const Scalar north, const Scalar east, const Scalar depth,
                          Scalar& lat, Scalar& lon, Scalar& height) const;
        
    private:
        Scalar _init_lat;
        Scalar _init_lon;
        Scalar _init_h;
        Scalar _init_ecef_x;
        Scalar _init_ecef_y;
        Scalar _init_ecef_z;
        Matrix3 _ecef_to_ned_matrix;
        Matrix3 _ned_to_ecef_matrix;
        
        Scalar __cbrt__(const Scalar x) const;
        Matrix3 __nRe__(const Scalar lat_rad, const Scalar lon_rad) const;
        
        //World Geodetic System 1984 (WGS84)
        static const Scalar a;
        static const Scalar b;
        static const Scalar esq;
        static const Scalar e1sq;
        static const Scalar f;
    };
}

#endif
