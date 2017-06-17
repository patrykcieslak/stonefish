//
//  PathGenerator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "PathGenerator.h"
#include "Console.h"

PathGenerator::PathGenerator()
{
    time = btScalar(0.);
    length = btScalar(0.);
    renderable = false;
}

PathGenerator::~PathGenerator()
{
}

btScalar PathGenerator::getTime()
{
    return time;
}

btScalar PathGenerator::getLength()
{
    return length;
}

void PathGenerator::MoveOnPath(btScalar distance, btVector3& point, btVector3& tangent)
{
    if(length > btScalar(0.))
    {
        btScalar dt = distance/length;
        time += dt;
        time = time > btScalar(1.) ? btScalar(1.) : time;
        PointAtTime(time, point, tangent);
    }
}

void PathGenerator::SavePathToTextFile(const char* path, unsigned int numOfPoints, unsigned int fixedPrecision)
{
    if(numOfPoints < 2)
        return;
    
    cInfo("Saving path to: %s", path);
    
    FILE* fp = fopen(path, "wt");
    if(fp == NULL)
    {
        cError("File could not be opened!");
        return;
    }
    
    //Write header
    fprintf(fp, "#Unit system: %s\n\n", UnitSystem::GetDescription().c_str());
    
    //Write data header
    fprintf(fp, "#X\tY\tZ\n");
    
    //Write data
    std::string format = "%1." + std::to_string(fixedPrecision) + "lf";
    
    for(unsigned int i = 0; i < numOfPoints; i++)
    {
        btVector3 point;
        btVector3 tangent;
        
        PointAtTime(btScalar(i)/btScalar(numOfPoints-1), point, tangent);
        point = UnitSystem::GetPosition(point);
        
        fprintf(fp, format.c_str(), point.x());
        fprintf(fp, "\t");
        fprintf(fp, format.c_str(), point.y());
        fprintf(fp, "\t");
        fprintf(fp, format.c_str(), point.z());
        fprintf(fp, "\n");
    }
    
    fclose(fp);
}

bool PathGenerator::isRenderable()
{
    return renderable;
}

void PathGenerator::setRenderable(bool render)
{
    renderable = render;
}