//
//  PathGenerator2D.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "controllers/PathGenerator2D.h"

#include "graphics/OpenGLContent.h"

using namespace sf;

PathGenerator2D::PathGenerator2D(PlaneType pathOnPlane)
{
    plane = pathOnPlane;
}

PathGenerator2D::~PathGenerator2D()
{
    for(unsigned int i = 0; i < subPaths.size(); i++)
        delete subPaths[i];
}

PlaneType PathGenerator2D::getPlane()
{
    return plane;
}

bool PathGenerator2D::is3D()
{
    return false;
}

void PathGenerator2D::AddSubPath(Path2D* sp, bool smoothConnection)
{
    if(sp != NULL)
    {
        //Check if interconnection needed
        if(subPaths.size() > 0)
        {
            Point2D p0;
            Point2D d0;
            Point2D p1;
            Point2D d1;
            subPaths.back()->PointAtTime(Scalar(1.), p0, d0);
            sp->PointAtTime(Scalar(0.), p1, d1);
            
            if(p0.distance(p1) >= SIMD_EPSILON) //Ends of subpaths do not coincide - interpath needed
            {
                if(smoothConnection) //Create connection of class C1
                {
                    Bezier2D* interpath = new Bezier2D(p0, p1, d0, d1, true);
                    subPaths.push_back(interpath);
                }
                else //Create linear connection
                {
                    Pwl2D* interpath = new Pwl2D(p0);
                    interpath->AddLineToPoint(p1);
                    subPaths.push_back(interpath);
                }
                
                //Update length
                length += subPaths.back()->getLength();
            }
        }
        
        //Add subpath to the list
        subPaths.push_back(sp);
        length += subPaths.back()->getLength();
    }
}

void PathGenerator2D::FindClosestPoint(const Vector3& position, Vector3& point, Vector3& tangent)
{
    
}

void PathGenerator2D::PointAtTime(Scalar t, Vector3& point, Vector3& tangent)
{
    if(subPaths.size() == 0)
        return;
    
    //Check and correct time
    t = t > Scalar(1.) ? Scalar(1.) : (t < Scalar(0.) ? Scalar(0.) : t);
    
    //Find point on subpath
    Scalar distance = t * length;
    Scalar lengthSum = Scalar(0.);
    Point2D pointOnSubpath;
    Point2D derivOnSubpath;
    
    for(unsigned int i = 0; i < subPaths.size(); i++) //Go from the beginning of path
    {
        Scalar subLength = subPaths[i]->getLength();
        
        if(distance <= lengthSum + subLength) //Check if point on this subpath
        {
            subPaths[i]->PointAtTime((distance - lengthSum)/subLength, pointOnSubpath, derivOnSubpath);
            break;
        }
        
        lengthSum += subLength;
    }

    //Select appropriate plane
    switch (plane)
    {
        case PLANE_XY:
            point.setX(pointOnSubpath.x);
            point.setY(pointOnSubpath.y);
            point.setZ(0.);
            tangent.setX(derivOnSubpath.x);
            tangent.setY(derivOnSubpath.y);
            tangent.setZ(0.);
            tangent.normalize();
            break;
            
        case PLANE_XZ:
            point.setX(pointOnSubpath.x);
            point.setZ(pointOnSubpath.y);
            point.setY(0.);
            tangent.setX(derivOnSubpath.x);
            tangent.setZ(derivOnSubpath.y);
            tangent.setY(0.);
            tangent.normalize();
            break;
            
        case PLANE_YZ:
            point.setY(pointOnSubpath.x);
            point.setZ(pointOnSubpath.y);
            point.setX(0.);
            tangent.setY(derivOnSubpath.x);
            tangent.setZ(derivOnSubpath.y);
            tangent.setX(0.);
            tangent.normalize();
            break;
    }
}

void PathGenerator2D::Render()
{
    unsigned int steps = floor(length/Scalar(0.01));
    
    std::vector<glm::vec3> vertices;
		
	for(unsigned int i = 0; i <= steps; ++i)
    {
        Vector3 point;
        Vector3 tangent;
        PointAtTime(Scalar(i)/Scalar(steps), point, tangent);
		vertices.push_back(glm::vec3(point.getX(), point.getY(), point.getZ()));
	}
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, CONTACT_COLOR);
}

Pwl2D::Pwl2D(const Point2D& start)
{
    Point2D s(start.x, start.y);
    points.push_back(s);
    length = Scalar(0.);
}

void Pwl2D::AddLineToPoint(const Point2D& p)
{
    //Push point
    Point2D pn(p.x, p.y);
    points.push_back(pn);
    
    //Update length
    length += points[points.size() - 2].distance(points.back());
}

void Pwl2D::FindClosestPoint(const Point2D &position, Point2D &point, Point2D &tangent)
{
    
}

void Pwl2D::PointAtTime(Scalar t, Point2D &point, Point2D &tangent)
{
    if(length == Scalar(0.))
        return;
        
    Scalar distance = t * length;
    Scalar lengthSum = Scalar(0.);
    
    for(unsigned int i = 0; i < points.size(); i++)
    {
        Scalar lineLength = points[i].distance(points[i+1]);
        
        if(distance <= lengthSum + lineLength)
        {
            tangent = (points[i+1] - points[i])/lineLength;
            point = points[i] + tangent * (distance - lengthSum);
            
            //TODO: Think about how to calculate good tangents (maybe a param?)
            tangent *= Scalar(0.2) * lineLength;
            break;
        }
        
        lengthSum += lineLength;
    }
}

Arc2D::Arc2D(const Point2D& center, Scalar radius, Scalar startAngle, Scalar endAngle)
{
    //Set params
    c.x = center.x;
    c.y = center.y;
    r = radius;
    range = Point2D(startAngle, endAngle);
    
    //Calculate length
    length = btFabs(range.y - range.x) * r;
}

void Arc2D::FindClosestPoint(const Point2D &position, Point2D &point, Point2D &tangent)
{
    
}

void Arc2D::PointAtTime(Scalar t, Point2D &point, Point2D &tangent)
{
    Scalar angleAtTime = range.x + (range.y - range.x) * t;
    
    point.x = c.x + r * btCos(angleAtTime);
    point.y = c.y + r * btSin(angleAtTime);
    
    tangent.x = -btSin(angleAtTime);
    tangent.y = btCos(angleAtTime);
    tangent *= (range.y - range.x) > Scalar(0.) ? Scalar(r) : Scalar(-r);
}

Bezier2D::Bezier2D(const Point2D& start, const Point2D& end, const Point2D& controlOrTangent1, const Point2D& controlOrTangent2, bool useTangents)
{
    //Set end points directly
    points[0] = Point2D(start.x, start.y);
    points[1] = Point2D(controlOrTangent1.x, controlOrTangent1.y);
    points[2] = Point2D(controlOrTangent2.x, controlOrTangent2.y);
    points[3] = Point2D(end.x, end.y);
    
    if(useTangents) //Calculate control points from tangents
    {
        points[1] = points[0] + points[1];
        points[2] = points[3] - points[2];
    }
    
    //Calculate length
    //Length of piecewise linear connection of points
    Pwl2D pwl(points[0]);
    pwl.AddLineToPoint(points[1]);
    pwl.AddLineToPoint(points[2]);
    pwl.AddLineToPoint(points[3]);
    Scalar pwlLength = pwl.getLength();
    
    //Steps of integration (step length of 1 mm)
    unsigned int steps = floor(pwlLength/Scalar(0.001));
    
    //Integrate length
    Point2D lastPoint = points[0];
    length = Scalar(0.);
    
    for(unsigned int i = 1; i <= steps; i++)
    {
        Point2D point;
        Point2D tangent;
        PointAtTime(Scalar(i)/Scalar(steps), point, tangent);
        length += lastPoint.distance(point);
        lastPoint = point;
    }
}

void Bezier2D::FindClosestPoint(const Point2D &position, Point2D &point, Point2D &tangent)
{
    
}

void Bezier2D::PointAtTime(Scalar t, Point2D &point, Point2D &tangent)
{
    point = points[0] * bezierTerm(3, 0, t);
    point += points[1] * bezierTerm(3, 1, t);
    point += points[2] * bezierTerm(3, 2, t);
    point += points[3] * bezierTerm(3, 3, t);
    
    tangent = (points[1] - points[0]) * Scalar(3.) * bezierTerm(2, 0, t);
    tangent += (points[2] - points[1]) * Scalar(3.) * bezierTerm(2, 1, t);
    tangent += (points[3] - points[2]) * Scalar(3.) * bezierTerm(2, 2, t);
}

unsigned int Bezier2D::factorial(unsigned int n)
{
    return (n == 0 || n == 1) ? 1 : factorial(n - 1) * n;
}

Scalar Bezier2D::bezierTerm(unsigned int n, unsigned int i, Scalar t)
{
    Scalar factorialPart = Scalar(factorial(n))/Scalar(factorial(i) * factorial(n - i));
    Scalar exponentPart = btPow(t, Scalar(i)) * btPow(Scalar(1.) - t, Scalar(n - i));
    return factorialPart * exponentPart;
}



