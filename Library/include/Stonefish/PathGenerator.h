//
//  PathGenerator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PathGenerator__
#define __Stonefish_PathGenerator__

#include "UnitSystem.h"
#include "OpenGLPipeline.h"

/*! Abstract path generator base class */
class PathGenerator
{
public:
    PathGenerator();
    virtual ~PathGenerator();
    
    virtual void FindClosestPoint(const btVector3& position, btVector3& point, btVector3& tangent) = 0;
    virtual void PointAtTime(btScalar t, btVector3& point, btVector3& tangent) = 0;
    virtual void Render() = 0;
    
    void MoveOnPath(btScalar distance, btVector3& point, btVector3& tangent);
    void SavePathToTextFile(const char* path, unsigned int numOfPoints, unsigned int fixedPrecision = 6);
    
    void setRenderable(bool render);
    btScalar getLength();
    btScalar getTime();
    bool isRenderable();
    virtual bool is3D() = 0;
    
protected:
    btScalar time;
    btScalar length;
    bool renderable;
};

#endif
