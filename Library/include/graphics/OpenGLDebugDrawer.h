//
//  OpenGLDebugDrawer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 28/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLDebugDrawer__
#define __Stonefish_OpenGLDebugDrawer__

#include <LinearMath/btIDebugDraw.h>
#include "graphics/OpenGLPipeline.h"

namespace sf
{

class OpenGLDebugDrawer : public btIDebugDraw
{
public:
    OpenGLDebugDrawer(int debugMode, bool zUp);
    
    void drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
    void drawLine(const btVector3& from,const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void draw3dText(const btVector3& location, const char* textString);
    void reportErrorWarning(const char* warningString);
	
	void setDebugMode(int debugMode);
	int	getDebugMode() const;
    
private:
    int mode;
	bool zAxisUp;
};

}

#endif
