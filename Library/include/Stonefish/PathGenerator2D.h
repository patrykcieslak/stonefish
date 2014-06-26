//
//  PathGenerator2D.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PathGenerator2D__
#define __Stonefish_PathGenerator2D__

#include "PathGenerator.h"

/*! Structure representing a two-dimensional point */
struct Point2D
{
    Point2D() : x(btScalar(0.)), y(btScalar(0.)) {}
    Point2D(const btScalar& coordX, const btScalar& coordY) : x(coordX), y(coordY) {}
    
    Point2D& operator=(const Point2D& p)
    {
        this->x = p.x;
        this->y = p.y;
        return *this;
    }
    
    Point2D operator-(const Point2D& p) const
    {
        return Point2D(this->x - p.x, this->y - p.y);
    }
    
    Point2D operator+(const Point2D& p) const
    {
        return Point2D(this->x + p.x, this->y + p.y);
    }
    
    Point2D operator*(btScalar s) const
    {
        return Point2D(s * this->x, s * this->y);
    }
    
    Point2D operator/(btScalar s) const
    {
        return Point2D(this->x/s, this->y/s);
    }
    
    Point2D& operator+=(const Point2D& p)
    {
        this->x += p.x;
        this->y += p.y;
        return *this;
    }
    
    Point2D& operator-=(const Point2D& p)
    {
        this->x -= p.x;
        this->y -= p.y;
        return *this;
    }
    
    Point2D& operator*=(btScalar s)
    {
        this->x *= s;
        this->y *= s;
        return *this;
    }
    
    Point2D& operator/=(btScalar s)
    {
        this->x /= s;
        this->y /= s;
        return *this;
    }
    
    btScalar distance(const Point2D& p) const
    {
        return btSqrt((p.x - this->x)*(p.x - this->x) + (p.y - this->y)*(p.y - this->y));
    }
    
    btScalar x;
    btScalar y;
};

/*! Abstract two-dimensional path base class */
class Path2D
{
public:
    Path2D() : length(btScalar(0.)) {}
    virtual ~Path2D(){}
    
    virtual void FindClosestPoint(const Point2D& position, Point2D& point, Point2D& tangent) = 0;
    virtual void PointAtTime(btScalar t, Point2D& point, Point2D& tangent) = 0;
    
    virtual btScalar getLength() { return length; };
    
protected:
    btScalar length;
};

/*! Two-dimensional piecewise linear path */
class Pwl2D : public Path2D
{
public:
    Pwl2D(const Point2D& start);
    
    void AddLineToPoint(const Point2D& p);
    
    void FindClosestPoint(const Point2D& position, Point2D& point, Point2D& tangent);
    void PointAtTime(btScalar t, Point2D& point, Point2D& tangent);
    
private:
    std::vector<Point2D> points;
};

/*! Two-dimensional arc path */
class Arc2D : public Path2D
{
public:
    Arc2D(const Point2D& center, btScalar radius, btScalar startAngle, btScalar endAngle);
    
    void FindClosestPoint(const Point2D& position, Point2D& point, Point2D& tangent);
    void PointAtTime(btScalar t, Point2D& point, Point2D& tangent);
    
private:
    Point2D c;
    btScalar r;
    Point2D range;
};

/*! Two-dimensional cubic Bezier curve */
class Bezier2D : public Path2D
{
public:
    Bezier2D(const Point2D& start, const Point2D& end, const Point2D& controlOrTangent1, const Point2D& controlOrTangent2, bool useTangents = false);
    
    void FindClosestPoint(const Point2D& position, Point2D& point, Point2D& tangent);
    void PointAtTime(btScalar t, Point2D& point, Point2D& tangent);
    
private:
    unsigned int factorial(unsigned int n);
    btScalar bezierTerm(unsigned int n, unsigned int i, btScalar t);
    
    Point2D points[4];
};

//Akima spline

/*! Two-dimensional path generator */
class PathGenerator2D : public PathGenerator
{
public:
    PathGenerator2D(PlaneType pathOnPlane);
    ~PathGenerator2D();
    
    void AddSubPath(Path2D* sp, bool smoothConnection = true);
    
    void FindClosestPoint(const btVector3& position, btVector3& point, btVector3& tangent);
    void PointAtTime(btScalar t, btVector3& point, btVector3& tangent);
    void Render();
    
private:
    PlaneType plane;
    std::vector<Path2D*> subPaths;
};

#endif
