//
//  OpenGLPipeline.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPipeline__
#define __Stonefish_OpenGLPipeline__

#include <GL/glew.h>
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <SDL2/SDL_thread.h>
#include "StonefishCommon.h"

#define DUMMY_COLOR glm::vec4(1.f, 0.4f, 0.1f, 1.f)
#define CONTACT_COLOR glm::vec4(1.f, 0, 0, 1.f)

#define SUN_ILLUMINANCE 107527.f //Sun average illuminance Lux
#define SUN_SKY_FACTOR 10.f //Sun illuminance to sky illuminance factor (sky treated as reference)

#define TEX_BASE				((GLint)0)
#define TEX_GUI1                ((GLint)1)
#define TEX_GUI2                ((GLinr)2)

#define TEX_ATM_TRANSMITTANCE	((GLint)3)
#define TEX_ATM_SCATTERING 		((GLint)4)
#define TEX_ATM_IRRADIANCE 		((GLint)5)

#define TEX_POSTPROCESS1		((GLint)6)
#define TEX_POSTPROCESS2		((GLint)7)
#define TEX_POSTPROCESS3		((GLint)8)
#define TEX_POSTPROCESS4		((GLint)9)
#define TEX_POSTPROCESS5		((GLint)10)

#define TEX_SUN_SHADOW			((GLint)11)
#define TEX_SUN_DEPTH			((GLint)12)
#define TEX_SPOT_SHADOW			((GLint)13)
#define TEX_SPOT_DEPTH			((GLint)14)
#define TEX_POINT_SHADOW        ((GLint)15) //Not used
#define TEX_POINT_DEPTH         ((GLint)16) //Not used

namespace sf
{
    //! An enum used to designate rendering quality.
    typedef enum {QUALITY_DISABLED = 0, QUALITY_LOW, QUALITY_MEDIUM, QUALITY_HIGH} RenderQuality;
    
    //! A structure containing rendering settings.
    struct RenderSettings
    {
        GLint windowW;
        GLint windowH;
        RenderQuality shadows;
        RenderQuality ao;
        RenderQuality atmosphere;
        RenderQuality ocean;
        bool msaa;
        
        RenderSettings()
        {
            windowW = 800;
            windowH = 600;
            shadows = RenderQuality::QUALITY_MEDIUM;
            ao = RenderQuality::QUALITY_MEDIUM;
            atmosphere = RenderQuality::QUALITY_MEDIUM;
            ocean = RenderQuality::QUALITY_MEDIUM;
            msaa = false;
        }
    };
    
    //! An enum used to designate type of helper objecy to be drawn.
    typedef enum {SOLID = 0, SOLID_CS, MULTIBODY_AXIS, HYDRO_CYLINDER, HYDRO_ELLIPSOID, HYDRO_CS, HYDRO_POINTS, HYDRO_LINES, HYDRO_LINE_STRIP, SENSOR_CS, SENSOR_LINES, SENSOR_LINE_STRIP, SENSOR_POINTS, ACTUATOR_LINES} RenderableType;
    
    //! A structure containing settings for drawing helper objects.
    struct HelperSettings
    {
        bool showCoordSys;
        bool showJoints;
        bool showActuators;
        bool showSensors;
        bool showLightMeshes;
        bool showCameraFrustums;
        bool showFluidDynamics;
        bool showBulletDebugInfo;
        
        HelperSettings()
        {
            showCoordSys = false;
            showJoints = false;
            showActuators = false;
            showSensors = false;
            showLightMeshes = false;
            showCameraFrustums = false;
            showFluidDynamics = false;
            showBulletDebugInfo = false;
        }
    };
    
    //! A structure that represents a renderable object.
    struct Renderable
    {
        RenderableType type;
        int lookId;
        int objectId;
        glm::mat4 model;
        std::vector<glm::vec3> points;
    };
    
    //! A structure representing a color.
    struct Color
    {
        glm::vec3 rgb;
        
        Color(GLfloat R, GLfloat G, GLfloat B)
        {
            rgb = glm::vec3(R,G,B);
        }
        
        static Color RGB(GLfloat R, GLfloat G, GLfloat B)
        {
            return Color(R,G,B);
        }
        
        static Color HSV(GLfloat H, GLfloat S, GLfloat V)
        {
            return Color(0,0,0);
        }
    };
    
    //! A structure representing a color system.
    struct ColorSystem
    {
        GLfloat xRed, yRed,        /* Red x, y */
        xGreen, yGreen,          /* Green x, y */
        xBlue, yBlue,            /* Blue x, y */
        xWhite, yWhite,          /* White point x, y */
        gamma;                   /* Gamma correction for system */
    };

    class SimulationManager;
    class OpenGLContent;
    class OpenGLCamera;
    
    //! A class implementing the OpenGL rendering pipeline.
    class OpenGLPipeline
    {
    public:
        //! A constructor.
        /*!
         \param s a structure containing render settings
         */
        OpenGLPipeline(RenderSettings s, HelperSettings h);
        
        //! A destructor.
        ~OpenGLPipeline();
        
        //! A method that constitutes the main rendering pipeline.
        /*!
         \param sim a pointer to the simulation manager
         */
        void Render(SimulationManager* sim);
        
        //! A method to add renderable objects to the rendering queue.
        /*!
         \param r a renderable object
         */
        void AddToDrawingQueue(Renderable r);
        
        //! A method that draws all normal objects.
        void DrawObjects();
        
        //! A method that blits the screen FBO to the main framebuffer.
        void DrawDisplay();
        
        //! A method that informs if the drawing queue is empty.
        bool isDrawingQueueEmpty();
        
        //! A method to get mutex of the drawing queue for thread safeness.
        SDL_mutex* getDrawingQueueMutex();
        
        //! A method returning a copy of the render settings.
        RenderSettings getRenderSettings() const;
        
        //! A method returning a reference to the helper object settings.
        HelperSettings& getHelperSettings();

        //! A method returning the screen texture, used for generating GUI background.
        GLuint getScreenTexture();
        
        //! A method returning a pointer to the OpenGL content manager.
        OpenGLContent* getContent();
        
    private:
        void PerformDrawingQueueCopy();
        void DrawHelpers();
        
        RenderSettings rSettings;
        HelperSettings hSettings;
        std::vector<Renderable> drawingQueue;
        std::vector<Renderable> drawingQueueCopy;
        SDL_mutex* drawingQueueMutex;
        GLuint screenFBO;
        GLuint screenTex;
        OpenGLContent* content;
    };
}

#endif
