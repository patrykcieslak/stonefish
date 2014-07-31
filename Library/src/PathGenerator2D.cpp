//
//  PathGenerator2D.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "PathGenerator2D.h"

#pragma mark Path generator
#pragma mark -Constructors
PathGenerator2D::PathGenerator2D(PlaneType pathOnPlane)
{
    plane = pathOnPlane;
}

#pragma mark -Destructor
PathGenerator2D::~PathGenerator2D()
{
    for(int i = 0; i < subPaths.size(); i++)
        delete subPaths[i];
}

#pragma mark -Accessors
PlaneType PathGenerator2D::getPlane()
{
    return plane;
}

bool PathGenerator2D::is3D()
{
    return false;
}

#pragma mark -Methods
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
            subPaths.back()->PointAtTime(btScalar(1.), p0, d0);
            sp->PointAtTime(btScalar(0.), p1, d1);
            
            if(p0.distance(p1) >= SIMD_EPSILON) //Ends of subpaths do not coincide - interpath needed
            {
                //Convert back to external system -> needed for interpath constructors
                p0.x = UnitSystem::GetLength(p0.x);
                p0.y = UnitSystem::GetLength(p0.y);
                p1.x = UnitSystem::GetLength(p1.x);
                p1.y = UnitSystem::GetLength(p1.y);
                d0.x = UnitSystem::GetLength(d0.x);
                d0.y = UnitSystem::GetLength(d0.y);
                d1.x = UnitSystem::GetLength(d1.x);
                d1.y = UnitSystem::GetLength(d1.y);
                
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

void PathGenerator2D::FindClosestPoint(const btVector3& position, btVector3& point, btVector3& tangent)
{
    
}

void PathGenerator2D::PointAtTime(btScalar t, btVector3& point, btVector3& tangent)
{
    if(subPaths.size() == 0)
        return;
    
    //Check and correct time
    t = t > btScalar(1.) ? btScalar(1.) : (t < btScalar(0.) ? btScalar(0.) : t);
    
    //Find point on subpath
    btScalar distance = t * length;
    btScalar lengthSum = btScalar(0.);
    Point2D pointOnSubpath;
    Point2D derivOnSubpath;
    
    for(int i = 0; i < subPaths.size(); i++) //Go from the beginning of path
    {
        btScalar subLength = subPaths[i]->getLength();
        
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
    unsigned int steps = floor(length/btScalar(0.01));
    
    glContactColor();
    glBegin(GL_LINE_STRIP);
    for(unsigned int i = 0; i <= steps; i++)
    {
        btVector3 point;
        btVector3 tangent;
        PointAtTime(btScalar(i)/btScalar(steps), point, tangent);
        glBulletVertex(point);
    }
    glEnd();
}

#pragma mark - Piecewise linear subpath
#pragma mark -Constructors
Pwl2D::Pwl2D(const Point2D& start)
{
    Point2D s(UnitSystem::SetLength(start.x), UnitSystem::SetLength(start.y));
    points.push_back(s);
    length = btScalar(0.);
}

#pragma mark -Methods
void Pwl2D::AddLineToPoint(const Point2D& p)
{
    //Push point
    Point2D pn(UnitSystem::SetLength(p.x), UnitSystem::SetLength(p.y));
    points.push_back(pn);
    
    //Update length
    length += points[points.size() - 2].distance(points.back());
}

void Pwl2D::FindClosestPoint(const Point2D &position, Point2D &point, Point2D &tangent)
{
    
}

void Pwl2D::PointAtTime(btScalar t, Point2D &point, Point2D &tangent)
{
    if(length == btScalar(0.))
        return;
        
    btScalar distance = t * length;
    btScalar lengthSum = btScalar(0.);
    
    for(unsigned int i = 0; i < points.size(); i++)
    {
        btScalar lineLength = points[i].distance(points[i+1]);
        
        if(distance <= lengthSum + lineLength)
        {
            tangent = (points[i+1] - points[i])/lineLength;
            point = points[i] + tangent * (distance - lengthSum);
            
            //TODO: Think about how to calculate good tangents (maybe a param?)
            tangent *= btScalar(0.2) * lineLength;
            break;
        }
        
        lengthSum += lineLength;
    }
}

#pragma mark - Arc subpath
#pragma mark -Constructors
Arc2D::Arc2D(const Point2D& center, btScalar radius, btScalar startAngle, btScalar endAngle)
{
    //Set params
    c.x = UnitSystem::SetLength(center.x);
    c.y = UnitSystem::SetLength(center.y);
    r = UnitSystem::SetLength(radius);
    range = Point2D(UnitSystem::SetAngle(startAngle), UnitSystem::SetAngle(endAngle));
    
    //Calculate length
    length = btFabs(range.y - range.x) * r;
}

#pragma mark -Methods
void Arc2D::FindClosestPoint(const Point2D &position, Point2D &point, Point2D &tangent)
{
    
}

void Arc2D::PointAtTime(btScalar t, Point2D &point, Point2D &tangent)
{
    btScalar angleAtTime = range.x + (range.y - range.x) * t;
    
    point.x = c.x + r * btCos(angleAtTime);
    point.y = c.y + r * btSin(angleAtTime);
    
    tangent.x = -btSin(angleAtTime);
    tangent.y = btCos(angleAtTime);
    tangent *= (range.y - range.x) > btScalar(0.) ? btScalar(r) : btScalar(-r);
}

#pragma mark - Cubic Bezier subpath
#pragma mark -Constructors
Bezier2D::Bezier2D(const Point2D& start, const Point2D& end, const Point2D& controlOrTangent1, const Point2D& controlOrTangent2, bool useTangents)
{
    //Set end points directly
    points[0] = Point2D(UnitSystem::SetLength(start.x), UnitSystem::SetLength(start.y));
    points[1] = Point2D(UnitSystem::SetLength(controlOrTangent1.x), UnitSystem::SetLength(controlOrTangent1.y));
    points[2] = Point2D(UnitSystem::SetLength(controlOrTangent2.x), UnitSystem::SetLength(controlOrTangent2.y));
    points[3] = Point2D(UnitSystem::SetLength(end.x), UnitSystem::SetLength(end.y));;
    
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
    btScalar pwlLength = UnitSystem::GetLength(pwl.getLength());
    
    //Steps of integration (step length of 1 mm)
    unsigned int steps = floor(pwlLength/btScalar(0.001));
    
    //Integrate length
    Point2D lastPoint = points[0];
    length = btScalar(0.);
    
    for(unsigned int i = 1; i <= steps; i++)
    {
        Point2D point;
        Point2D tangent;
        PointAtTime(btScalar(i)/btScalar(steps), point, tangent);
        length += lastPoint.distance(point);
        lastPoint = point;
    }
}

#pragma mark -Methods
void Bezier2D::FindClosestPoint(const Point2D &position, Point2D &point, Point2D &tangent)
{
    
}

void Bezier2D::PointAtTime(btScalar t, Point2D &point, Point2D &tangent)
{
    point = points[0] * bezierTerm(3, 0, t);
    point += points[1] * bezierTerm(3, 1, t);
    point += points[2] * bezierTerm(3, 2, t);
    point += points[3] * bezierTerm(3, 3, t);
    
    tangent = (points[1] - points[0]) * btScalar(3.) * bezierTerm(2, 0, t);
    tangent += (points[2] - points[1]) * btScalar(3.) * bezierTerm(2, 1, t);
    tangent += (points[3] - points[2]) * btScalar(3.) * bezierTerm(2, 2, t);
}

unsigned int Bezier2D::factorial(unsigned int n)
{
    return (n == 0 || n == 1) ? 1 : factorial(n - 1) * n;
}

btScalar Bezier2D::bezierTerm(unsigned int n, unsigned int i, btScalar t)
{
    btScalar factorialPart = btScalar(factorial(n))/btScalar(factorial(i) * factorial(n - i));
    btScalar exponentPart = btPow(t, btScalar(i)) * btPow(btScalar(1.) - t, btScalar(n - i));
    return factorialPart * exponentPart;
}



