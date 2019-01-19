//
//  OpenGLLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLLight__
#define __Stonefish_OpenGLLight__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //!
    typedef enum {POINT_LIGHT, SPOT_LIGHT} LightType;
    
    class GLSLShader;
    class OpenGLPipeline;
    class OpenGLCamera;
    class SimulationManager;
    
    //!
    class OpenGLLight
    {
    public:
        //Discrete lights
        OpenGLLight(glm::vec3 position, glm::vec3 color, GLfloat illuminance);
        virtual ~OpenGLLight();
        
        virtual void InitShadowmap(GLint shadowmapLayer) = 0;
        virtual void SetupShader(GLSLShader* shader, unsigned int lightId) = 0;
        virtual void BakeShadowmap(OpenGLPipeline* pipe) = 0;
        
        virtual void ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h) = 0;
        virtual LightType getType() = 0;
        
        void UpdatePosition(glm::vec3 p);
        virtual void UpdateTransform();
        void Activate();
        void Deactivate();
        void setLightSurfaceDistance(GLfloat dist);
        glm::vec3 getColor();
        glm::vec3 getPosition();
        glm::quat getOrientation();
        bool isActive();
        
        //Static methods
        static void Init(std::vector<OpenGLLight*>& lights);
        static void Destroy();
        static void SetupShader(GLSLShader* shader);
        static void SetCamera(OpenGLCamera* view);
        
    protected:
        GLfloat surfaceDistance;
        
        static GLuint spotShadowArrayTex; //2D array texture for storing shadowmaps of all spot lights (using only one texture unit for all spotlights!)
        static GLuint spotShadowSampler;
        static GLuint spotDepthSampler;
        static OpenGLCamera* activeView;
        
    private:
        bool active;
        glm::vec3 pos;
        glm::vec3 tempPos;
        glm::vec3 color;
    };
}

#endif
