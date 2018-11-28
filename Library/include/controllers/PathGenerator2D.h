//
//  PathGenerator2D.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PathGenerator2D__
#define __Stonefish_PathGenerator2D__

#include "controllers/PathGenerator.h"

namespace sf
{

typedef enum {PLANE_XY, PLANE_XZ, PLANE_YZ} PlaneType;
        
/*! Structure representing a two-dimensional point */
struct Point2D
{
    Point2D() : x(Scalar(0.)), y(Scalar(0.)) {}
    Point2D(const Scalar& coordX, const Scalar& coordY) : x(coordX), y(coordY) {}
    
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
    
    Point2D operator*(Scalar s) const
    {
        return Point2D(s * this->x, s * this->y);
    }
    
    Point2D operator/(Scalar s) const
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
    
    Point2D& operator*=(Scalar s)
    {
        this->x *= s;
        this->y *= s;
        return *this;
    }
    
    Point2D& operator/=(Scalar s)
    {
        this->x /= s;
        this->y /= s;
        return *this;
    }
    
    Scalar distance(const Point2D& p) const
    {
        return btSqrt((p.x - this->x)*(p.x - this->x) + (p.y - this->y)*(p.y - this->y));
    }
    
    Scalar x;
    Scalar y;
};

/*! Abstract two-dimensional path base class */
class Path2D
{
public:
    Path2D() : length(Scalar(0.)) {}
    virtual ~Path2D(){}
    
    virtual void FindClosestPoint(const Point2D& position, Point2D& point, Point2D& tangent) = 0;
    virtual void PointAtTime(Scalar t, Point2D& point, Point2D& tangent) = 0;
    
    virtual Scalar getLength() { return length; };
    
protected:
    Scalar length;
};

/*! Two-dimensional piecewise linear path */
class Pwl2D : public Path2D
{
public:
    Pwl2D(const Point2D& start);
    
    void AddLineToPoint(const Point2D& p);
    
    void FindClosestPoint(const Point2D& position, Point2D& point, Point2D& tangent);
    void PointAtTime(Scalar t, Point2D& point, Point2D& tangent);
    
private:
    std::vector<Point2D> points;
};

/*! Two-dimensional arc path */
class Arc2D : public Path2D
{
public:
    Arc2D(const Point2D& center, Scalar radius, Scalar startAngle, Scalar endAngle);
    
    void FindClosestPoint(const Point2D& position, Point2D& point, Point2D& tangent);
    void PointAtTime(Scalar t, Point2D& point, Point2D& tangent);
    
private:
    Point2D c;
    Scalar r;
    Point2D range;
};

/*! Two-dimensional cubic Bezier curve */
class Bezier2D : public Path2D
{
public:
    Bezier2D(const Point2D& start, const Point2D& end, const Point2D& controlOrTangent1, const Point2D& controlOrTangent2, bool useTangents = false);
    
    void FindClosestPoint(const Point2D& position, Point2D& point, Point2D& tangent);
    void PointAtTime(Scalar t, Point2D& point, Point2D& tangent);
    
private:
    unsigned int factorial(unsigned int n);
    Scalar bezierTerm(unsigned int n, unsigned int i, Scalar t);
    
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
    
    void FindClosestPoint(const Vector3& position, Vector3& point, Vector3& tangent);
    void PointAtTime(Scalar t, Vector3& point, Vector3& tangent);
    void Render();
    
    PlaneType getPlane();
    bool is3D();
    
private:
    PlaneType plane;
    std::vector<Path2D*> subPaths;
};
    
}

#endif
