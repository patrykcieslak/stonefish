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
//  DebugUtil.hpp
//  Stonefish
//
//  Created by Patryk Cieslak on 14/11/25.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "StonefishCommon.h"
#include <iostream>

#if __cplusplus >= 202002L
    #include <format>
    #include <string_view>
#else
    #include <string>
    #include <iomanip>
#endif

namespace sf
{
#if __cplusplus >= 202002L
void printTransform(std::string_view title, const Transform& t, unsigned int precision = 3)
{
    std::cout << std::format("---- {} ----\n", title);
    std::cout << std::format("Origin:\n [{1:.{0}f}, {2:.{0}f}, {3:.{0}f}]\n", precision, t.getOrigin().x(), t.getOrigin().y(), t.getOrigin().z());
    Matrix3 r = t.getBasis();
    std::cout << "Basis:\n";
    std::cout << std::format("[{1:.{0}f}, {2:.{0}f}, {3:.{0}f}]\n", precision, r[0].x(), r[0].y(), r[0].z());
    std::cout << std::format("[{1:.{0}f}, {2:.{0}f}, {3:.{0}f}]\n", precision, r[1].x(), r[1].y(), r[1].z());
    std::cout << std::format("[{1:.{0}f}, {2:.{0}f}, {3:.{0}f}]\n", precision, r[2].x(), r[2].y(), r[2].z());
}

void printVector3(std::string_view title, const Vector3& v, unsigned int precision = 3)
{
    std::cout << std::format("{0}: [{2:.{1}f}, {3:.{1}f}, {4:.{1}f}]\n", title, precision, v.getX(), v.getY(), v.getZ());
}

void printMatrix3(std::string_view title, const Matrix3& m, unsigned int precision = 3)
{
    std::cout << std::format("---- {} ----\n", title);
    std::cout << std::format("[{1:.{0}f}, {2:.{0}f}, {3:.{0}f}]\n", precision, m[0].x(), m[0].y(), m[0].z());
    std::cout << std::format("[{1:.{0}f}, {2:.{0}f}, {3:.{0}f}]\n", precision, m[1].x(), m[1].y(), m[1].z());
    std::cout << std::format("[{1:.{0}f}, {2:.{0}f}, {3:.{0}f}]\n", precision, m[2].x(), m[2].y(), m[2].z());
}

bool testScalar(std::string_view title, const Scalar& value, const Scalar& expected, Scalar tolerance = 1e-6)
{
    bool success = btFabs(value - expected) < tolerance;
    std::cout << std::format("{}: {}: {:.6f} (expected {:.6f})\n", (success ? "[PASS]" : "[FAIL]"), title, value, expected);
    return success;
}

bool testVector3(std::string_view title, const Vector3& value, const Vector3& expected, Scalar tolerance = 1e-6)
{
    bool success = btFabs(value.getX() - expected.getX()) < tolerance 
                && btFabs(value.getY() - expected.getY()) < tolerance 
                && btFabs(value.getZ() - expected.getZ()) < tolerance;
    std::cout << std::format("{}: {}: [{:.6f}, {:.6f}, {:.6f}] (expected [{:.6f}, {:.6f}, {:.6f}])\n", 
        (success ? "[PASS]" : "[FAIL]"), title, value.getX(), value.getY(), value.getZ(), expected.getX(), expected.getY(), expected.getZ());
    return success;
}
#else
void printTransform(const std::string& title, const Transform& t, unsigned int precision = 3)
{
    std::cout << "---- " << title << " ----\n";
    std::cout << std::setprecision(precision) << "Origin:\n [" << t.getOrigin().x() <<  ", " << t.getOrigin().y() << ", " << t.getOrigin().z() << "]\n";
    Matrix3 r = t.getBasis();
    std::cout << "Basis:\n";
    std::cout << std::setprecision(precision) << "[" << r[0].x() << ", " << r[0].y() << ", " << r[0].z() << "]\n"
                                              << "[" << r[1].x() << ", " << r[1].y() << ", " << r[1].z() << "]\n"
                                              << "[" << r[2].x() << ", " << r[2].y() << ", " << r[2].z() << "]\n";
}

void printVector3(const std::string& title, const Vector3& v, unsigned int precision = 3)
{
    std::cout << std::setprecision(precision) << title << ": [" << v.x() << ", " << v.y() << ", " << v.z() << "]\n";
}

void printMatrix3(const std::string& title, const Matrix3& m, unsigned int precision = 3)
{
    std::cout << "---- " << title << " ----\n";
    std::cout << std::setprecision(precision) << "[" << m[0].x() << ", " << m[0].y() << ", " << m[0].z() << "]\n"
                                              << "[" << m[1].x() << ", " << m[1].y() << ", " << m[1].z() << "]\n"
                                              << "[" << m[2].x() << ", " << m[2].y() << ", " << m[2].z() << "]\n";
}

bool testScalar(const std::string& title, const Scalar& value, const Scalar& expected, Scalar tolerance = 1e-6)
{
    bool success = btFabs(value - expected) < tolerance;
    std::cout << std::setprecision(6) << (success ? "[PASS] " : "[FAIL] ") << title << ": " << value << " (expected " << expected << ")\n";
    return success;
}

bool testVector3(const std::string& title, const Vector3& value, const Vector3& expected, Scalar tolerance = 1e-6)
{
    bool success = btFabs(value.getX() - expected.getX()) < tolerance 
                && btFabs(value.getY() - expected.getY()) < tolerance 
                && btFabs(value.getZ() - expected.getZ()) < tolerance;
    std::cout << std::setprecision(6) << (success ? "[PASS] " : "[FAIL] ") << title 
        << ": [" << value.x() << ", " << value.y() << ", " << value.z() << "]" 
        << " (expected [" << expected.x() << ", " << expected.y() << ", " << expected.z() << "])\n";
    return success;
}
#endif
}