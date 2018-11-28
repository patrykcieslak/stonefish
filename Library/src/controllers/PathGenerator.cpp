//
//  PathGenerator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "controllers/PathGenerator.h"

#include "graphics/Console.h"

using namespace sf;

PathGenerator::PathGenerator()
{
    time = Scalar(0.);
    length = Scalar(0.);
    renderable = false;
}

PathGenerator::~PathGenerator()
{
}

Scalar PathGenerator::getTime()
{
    return time;
}

Scalar PathGenerator::getLength()
{
    return length;
}

void PathGenerator::MoveOnPath(Scalar distance, Vector3& point, Vector3& tangent)
{
    if(length > Scalar(0.))
    {
        Scalar dt = distance/length;
        time += dt;
        time = time > Scalar(1.) ? Scalar(1.) : time;
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
    fprintf(fp, "#Unit system: SI\n\n");
    
    //Write data header
    fprintf(fp, "#X\tY\tZ\n");
    
    //Write data
    std::string format = "%1." + std::to_string(fixedPrecision) + "lf";
    
    for(unsigned int i = 0; i < numOfPoints; i++)
    {
        Vector3 point;
        Vector3 tangent;
        
        PointAtTime(Scalar(i)/Scalar(numOfPoints-1), point, tangent);
        
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
