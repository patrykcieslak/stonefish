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
//  OpenGLOcean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/2017.
//  Copyright (c) 2017-2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLOcean__
#define __Stonefish_OpenGLOcean__

#include "graphics/OpenGLContent.h"
#include <SDL2/SDL_mutex.h>
#include <map>
#include <memory>

namespace sf
{
    //! A structure holding parameters of ocean generation and rendering.
    struct OceanParams
    {
        unsigned int passes;
        unsigned int slopeVarianceSize;
        int fftSize;
        glm::vec4 gridSizes;
        float* spectrum12;
        float* spectrum34;
        bool propagate; //wave propagation
        float wind; //wind speed in meters per second (at 10m above surface)
        float omega; //sea state (inverse wave age)
        float A; //wave amplitude
        float km;
        float cm;
        float t;
    };

    #pragma pack(1)
    //! A structure representing the ocean currents UBO.
    struct OceanCurrentsUBO
    {
        VelocityFieldUBO currents[MAX_OCEAN_CURRENTS]; //REMARK: type -> 0=uniform,1=jet,2=pipe,10=thruster
        glm::vec3 gravity;
        GLuint numCurrents;
    };
    #pragma pack(0)

    class GLSLShader;
	class OpenGLView;
    class OpenGLCamera;
	class OpenGLOceanParticles;
    class VelocityField;
	
    //! A class implementing ocean simulation in OpenGL.
    class OpenGLOcean
    {
    public:
        //! A constructor.
        /*!
         \param size the size of the ocean surface mesh [m]
         */
        OpenGLOcean(GLfloat size);
        
        //! A destructor.
        virtual ~OpenGLOcean();

        //! A method that simulates wave propagation.
		/*!
		 \param dt time since last update
		 */
        virtual void Simulate(GLfloat dt);
        
        //! A method that updates the wave mesh.
        /*!
         \param view a pointer to the active view
         */
        virtual void UpdateSurface(OpenGLView* view) = 0;
        
        //! A method that draws the surface of the ocean.
        /*!
         \param view a pointer to the active view
         */
        virtual void DrawSurface(OpenGLView* view) = 0;
        
        //! A method that draws the surface of the ocean as thermal image.
        /*!
         \param view a pointer to the active view
         */
        virtual void DrawSurfaceTemperature(OpenGLView* view) = 0;

        //! A method that draws the surface of the ocean, seen from underwater.
        /*!
         \param view a pointer to the active view
         */
        virtual void DrawBacksurface(OpenGLView* view) = 0;
        
        //! A method that generates the stencil mask.
        /*!
         \param view a pointer to the active view
         */
        virtual void DrawUnderwaterMask(OpenGLView* view);

        //! A method that draws the distant background of the ocean.
        /*!
         \param view a pointer to the active view
         */
        void DrawBackground(OpenGLView* view);
        
		//! A method that draws underwater particles.
		/*!
		 \param view a pointer to the active view
		 */
		void DrawParticles(OpenGLView* view);

        //! A method that draws underwater particles id.
		/*!
		 \param view a pointer to the active view
         \param id the id of the particles
		 */
		void DrawParticlesId(OpenGLView* view, GLushort id);

        //! A method that draw water velocity field.
        /*!
         \param view a pointer to the view
         \param velocityMax velocity corresponding to the display limit
         */
        void DrawVelocityField(OpenGLView* view, GLfloat velocityMax);
		
        //! A method that draws the underwater bloom and blur effects (scattering).
        /*!
         \param cam a pointer to the active camera
         */
        void ApplySpecialEffects(OpenGLCamera* cam);
        
        //! A method that drawsd the spectrum of waves.
        /*!
         \param viewportSize size of the active viewport
         \param rect a rectangle in which to draw the spectrum on screen
         */
        void ShowSpectrum(glm::vec2 viewportSize, glm::vec4 rect);
        
        //! A method used to display a specified texture.
        /*!
         \param id the id of the texture to display
         \param rect a rectangle in which to draw the tecture on screen
         */
        void ShowTexture(int id, glm::vec4 rect);
       
        //! A method that updates ocean currents information.
        void UpdateOceanCurrentsData(const OceanCurrentsUBO& data);

        //! A method to allocate a particle system for a view.
        /*!
         \param view a pointer to the view.
         \return a pointer to the allocated particle system
         */
        std::shared_ptr<OpenGLOceanParticles> AllocateParticles(OpenGLView* view);

        //! A method to assign particle system to a view.
        /*!
         \param view a pointer to the view.
         \param particles a pointer to the particle system
         */
        void AssignParticles(OpenGLView* view, std::shared_ptr<OpenGLOceanParticles> particles);

        //! A method to set the type of ocean water.
        /*!
         \param t type of water
         */
        void setWaterType(GLfloat t);

        //! A method to set the temperature of the water.
        /*!
         \param t temperature of the water [degC]
         */
        void setWaterTemperature(GLfloat t);

        //! A method to get the temperature of the water.
        /*!
         \return temperature of the water [degC]
         */
        GLfloat getWaterTemperature();

        //! A method to set if the particles should be rendered.
        /*!
         \param enabled a flag specifying if the particles should be rendered
         */
        void setParticles(bool enabled);

        //! A method returning informing if the particles are enabled.
        bool getParticlesEnabled();

        //! A method to get wave height at a specified coordinate.
        /*!
         \param x the x coordinate in world frame [m]
         \param y the y coordinate in world frame [m]
         \return wave height [m]
         */
        virtual GLfloat ComputeWaveHeight(GLfloat x, GLfloat y);

        //! A method returning the id of the wave texture.
        GLuint getWaveTexture();

        //! A method returning the vector of wave grid sizes.
        glm::vec4 getWaveGridSizes();

        //! A method returning calculated light attenuation coefficient.
        glm::vec3 getLightAttenuation();
        	
        //! A method calculating Henyey-Greenstein scattering factor.
        glm::vec3 getLightScattering();
        
    protected:
        virtual void InitializeSimulation();
        float sqr(float x);
        
        std::map<OpenGLView*, std::shared_ptr<OpenGLOceanParticles>> oceanParticles;
        glm::vec3 absorption[64];
        glm::vec3 scattering[64];
        OceanCurrentsUBO oceanCurrentsUBOData;
        GLfloat oceanSize;
        OceanParams params;
        glm::vec3 lightAbsorption;
        glm::vec3 lightScattering;
        GLfloat waterTemperature;

        std::map<std::string, GLSLShader*> oceanShaders;
        GLuint oceanFBOs[3];
        GLuint oceanTextures[6];
        GLuint oceanCurrentsUBO;
        
    private:
        GLfloat* ComputeButterflyLookupTable(unsigned int size, unsigned int passes);
        int bitReverse(int i, int N);
        void computeWeight(int N, int k, float &Wr, float &Wi);
        float ComputeSlopeVariance();
        float GetSlopeVariance(float kx, float ky, float *spectrumSample);
        void GenerateWavesSpectrum();
        void GetSpectrumSample(int i, int j, float lengthScale, float kMin, float *result);
        float spectrum(float kx, float ky, bool omnispectrum = false);
        float omega(float k);

        int oceanBoxObj;
        bool particlesEnabled;
    };
}

#endif
