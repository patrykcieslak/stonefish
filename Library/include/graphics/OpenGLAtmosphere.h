/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  OpenGLAtmosphere.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/07/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLAtmosphere__
#define __Stonefish_OpenGLAtmosphere__

#include <functional>
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
	//! A structure representing data of the SunSky UBO (std140 aligned).
    #pragma pack(1)
	struct SunSkyUBO
	{
		glm::mat4 sunClipSpace[4];      
        glm::vec4 sunFrustumNear;       
        glm::vec4 sunFrustumFar;        
        glm::vec3 sunDirection;
        GLfloat planetRadiusInUnits;  
        glm::vec3 whitePoint;
        GLfloat atmLengthUnitInMeters;
	};
    #pragma pack(0)

    //! An enum definind id's of atmosphere textures.
    enum AtmosphereTextures
    {
        TRANSMITTANCE = 0,
        SCATTERING,
        IRRADIANCE,
        TEXTURE_COUNT
    };
    
    class GLSLShader;
    class OpenGLPipeline;
    class OpenGLCamera;
    
    //! A class implementing a physically correct atmosphere in OpenGL.
    class OpenGLAtmosphere
    {
    public:
        //! A constructor.
        /*!
         \param modelFilename a path to the atmosphere model file
         \param shadow a rendering quality od the shadow
         */
        OpenGLAtmosphere(const std::string& modelFilename, RenderQuality shadow);
        
        //! A destructor.
        ~OpenGLAtmosphere();

        //! A method loading the precomputed atmosphere model.
        /*!
         \param filename a path to the model data file
         */
        void LoadAtmosphereData(const std::string& filename);
        
        //! A method that draws the sky and sun.
        /*!
         \param view a pointer to the current view
         */
        void DrawSkyAndSun(const OpenGLCamera* view);
        
        //! A method that bakes the shadow maps for the sun.
        /*!
         \param pipe a pointer to the rendering pipeline
         \param view a pointer to the current view
         */
        void BakeShadowmaps(OpenGLPipeline* pipe, OpenGLCamera* view);
        
        //! A method to setup a material shaders.
        void SetupMaterialShaders();
         
        //! A method to set position of the sun in the sky.
        /*!
         \param azimuthDeg the azimuth of the sun [deg]
         \param elevationDeg the elevation of the sun [deg]
         */
        void SetSunPosition(GLfloat azimuthDeg, GLfloat elevationDeg);
        
        //! A method to get position of the sun in the sky.
        /*!
         \param azimuthDeg a reference to a variable to store the sun azimuth [deg]
         \param elevationDeg a reference to a variable to store the sun elevation [deg]
         */
        void GetSunPosition(GLfloat& azimuthDeg, GLfloat& elevationDeg);
        
        //! A method returning the OpenGL id of a texture.
        /*!
         \param id the atmosphere texture to get id for
         \return OpenGL id of the requested texture
         */
        GLuint getAtmosphereTexture(AtmosphereTextures id);
        
        //! A method returning the sun direction.
        glm::vec3 GetSunDirection();
         
        //! A method displaying the sun shadow maps.
        /*!
         \param x the x coordinate of the origin of the shadow map display on screen
         \param y the y coordinate of the origin of the shadow map display on screen
         \param scale a scale to use when displaying the shadow map
         */
        void ShowSunShadowmaps(GLfloat x, GLfloat y, GLfloat scale);
        
        //! Load atmosphere API needed for loading shaders of other objects.
        static void Init();
        
        //! A static method returning the OpenGL id of the compiled API shader.
        static GLuint getAtmosphereAPI();
        
    private:
        //Data
        unsigned int nPrecomputedWavelengths;
        unsigned int nScatteringOrders;
        GLuint sunSkyUBO;
        SunSkyUBO sunSkyUBOData;
        
        //Shadows
        glm::mat4 BuildCropProjMatrix(ViewFrustum &f);
        void UpdateFrustumCorners(ViewFrustum &f, glm::vec3 center, glm::vec3 dir, glm::vec3 up);
        void UpdateSplitDist(GLfloat nd, GLfloat fd);
        GLuint sunShadowmapArray;
        GLuint sunDepthSampler;
        GLuint sunShadowSampler;
        GLuint sunShadowmapSplits;
        GLuint sunShadowmapSize;
        glm::vec3 sunDirection;
        glm::mat4x4* sunShadowCPM;
        glm::mat4x4 sunModelView;
        ViewFrustum* sunShadowFrustum;
        GLuint sunShadowFBO;
		GLSLShader* sunShadowmapShader; //debug draw shadowmap
        
        //Rendering
        GLfloat sunAzimuth;
        GLfloat sunElevation;
        GLSLShader* skySunShader;
        GLuint textures[AtmosphereTextures::TEXTURE_COUNT];
        static GLuint atmosphereAPI;
    };
}

#endif
