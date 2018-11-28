//
//  PathGenerator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PathGenerator__
#define __Stonefish_PathGenerator__

#include "graphics/OpenGLPipeline.h"

namespace sf
{

/*! Abstract path generator base class */
class PathGenerator
{
public:
    PathGenerator();
    virtual ~PathGenerator();
    
    virtual void FindClosestPoint(const Vector3& position, Vector3& point, Vector3& tangent) = 0;
    virtual void PointAtTime(Scalar t, Vector3& point, Vector3& tangent) = 0;
    virtual void Render() = 0;
    
    void MoveOnPath(Scalar distance, Vector3& point, Vector3& tangent);
    void SavePathToTextFile(const char* path, unsigned int numOfPoints, unsigned int fixedPrecision = 6);
    
    void setRenderable(bool render);
    Scalar getLength();
    Scalar getTime();
    bool isRenderable();
    virtual bool is3D() = 0;
    
protected:
    Scalar time;
    Scalar length;
    bool renderable;
};
    
}

#endif
