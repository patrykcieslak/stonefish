//
//  PathFollowingController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "controllers/PathFollowingController.h"

#include "sensors/scalar/Trajectory.h"
#include "controllers/PathGenerator2D.h"
#include "sensors/Sample.h"

namespace sf
{

PathFollowingController::PathFollowingController(std::string uniqueName, PathGenerator* pathGenerator, Trajectory* positionSensor, Scalar frequency) : Controller(uniqueName, frequency)
{
    inputPath = pathGenerator;
    measuredTraj = positionSensor;
    
    if(pathGenerator->is3D())
        error.resize(7);
    else
        error.resize(4);
}

PathFollowingController::~PathFollowingController()
{
    delete inputPath;
    measuredTraj = NULL; //will be deleted with other sensors
    outputControllers.clear(); //will be deleted with other controllers
    error.clear();
}

PathGenerator* PathFollowingController::getPath()
{
    return inputPath;
}

ControllerType PathFollowingController::getType()
{
    return CONTROLLER_PATHFOLLOWING;
}

void PathFollowingController::RenderPath()
{
    /*if(inputPath->isRenderable())
    {
        inputPath->Render();
        
        Vector3 point;
        Vector3 tangent;
        inputPath->PointAtTime(inputPath->getTime(), point, tangent);
        std::vector<glm::vec3> vertices;
		vertices.push_back(glm::vec3(point.getX(), point.getY(), point.getZ()));
		OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::POINTS, vertices, CONTACT_COLOR);
	}*/
}

void PathFollowingController::Reset()
{
}

void PathFollowingController::Tick(Scalar dt)
{
    //Update position on path
    Vector3 desiredPoint;
    Vector3 desiredTangent;
    inputPath->MoveOnPath(VelocityOnPath() * dt, desiredPoint, desiredTangent);
    //printf("%1.5f\t%1.5f\n", desiredPoint.x(), desiredPoint.y());
    
    
    if(inputPath->getTime() >= Scalar(1.0)) //End of path reached
        PathEnd();
    else
    {
        if(inputPath->is3D()) //3D path following
        {
            
        }
        else //2D path following
        {
            PathGenerator2D* inputPath2D = (PathGenerator2D*)inputPath;
            
            //Get last measured position and attitude
            std::vector<Scalar> measured = measuredTraj->getLastSample().getData();
            Vector3 actualPoint = Vector3(measured[0], measured[1], measured[2]);
            
            //Calculate errors
            switch(inputPath2D->getPlane())
            {
                case PLANE_XY:
                {
                    Scalar desiredOrientation = btAtan2(desiredTangent.y(), desiredTangent.x());
                    Scalar actualOrientation = measured[5]; //Yaw
                    Vector3 positionDifference = desiredPoint - actualPoint;
                    Vector3 normalToPath = Vector3(-desiredTangent.y(), desiredTangent.x(), 0.0);
                    
                    error[0] = positionDifference.x(); //difference in x
                    error[1] = positionDifference.y(); //difference in y
                    error[2] = positionDifference.dot(normalToPath); //distance to path
                    
                    error[3] = desiredOrientation - actualOrientation;
                    error[3] = error[3] >= M_PI ? -(2*M_PI-error[3]) : (error[3] < -M_PI ? 2*M_PI + error[3] : error[3]); // error in [-pi,pi)
                }
                    break;
                    
                case PLANE_XZ:
                {
                    Scalar desiredOrientation = btAtan2(desiredTangent.z(), desiredTangent.x());
                    Scalar actualOrientation = measured[4]; //Pitch
                    Vector3 positionDifference = desiredPoint - actualPoint;
                    Vector3 normalToPath = Vector3(-desiredTangent.z(), 0.0, desiredTangent.x());
                    
                    error[0] = positionDifference.x(); //difference in x
                    error[1] = positionDifference.z(); //difference in z
                    error[2] = positionDifference.dot(normalToPath); //distance to path
                    error[3] = desiredOrientation - actualOrientation;
                }
                    break;
                    
                case PLANE_YZ:
                {
                    Scalar desiredOrientation = btAtan2(desiredTangent.z(), desiredTangent.y());
                    Scalar actualOrientation = measured[3]; //Roll
                    Vector3 positionDifference = desiredPoint - actualPoint;
                    Vector3 normalToPath = Vector3(0.0, -desiredTangent.z(), desiredTangent.y());
                    
                    error[0] = positionDifference.y(); //difference in y
                    error[1] = positionDifference.z(); //difference in z
                    error[2] = positionDifference.dot(normalToPath); //distance to path
                    error[3] = desiredOrientation - actualOrientation;
                }
                    break;
            }
        }
        
        //Run specific path following algorithm
        ControlTick(dt);
    }
}

}
