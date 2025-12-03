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
//  Copyright (c) 2017-2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLOcean__
#define __Stonefish_OpenGLOcean__

#include "graphics/OpenGLContent.h"
#include <SDL2/SDL_mutex.h>
#include <map>
#include <random>

namespace sf
{
    #pragma pack(1)
    //! A structure holding parameters of ocean generation and rendering (JONSWAP model).
    struct OceanSpectrumUBO
    {
        GLfloat scale {1.f};                // Overall scale factor
        GLfloat windSpeed {1.f};            // Wind speed [m/s]
        GLfloat windDirection {0.f};        // Wind direction [rad]
        GLfloat windFetchLength {1000.f};   // Wind fetch length [m]
        GLfloat depth {10.f};               // Water depth [m]
        GLfloat peakOmega {0.f};            // Peak angular frequency [rad/s]
        GLfloat alpha {0.f};                // Phillips spectrum constant
        GLfloat gamma {3.3f};               // Peak enhancement factor
        GLfloat swell {1.f};                // Swell factor
        GLfloat spreadBlend {0.5f};         // Spreading blend factor
        GLfloat shortWavesFade {0.2f};      // Short waves fade factor
        GLfloat g {9.81f};                  // Gravitational acceleration [m/s^2]

        bool operator==(const OceanSpectrumUBO& other) const = default;
    };

    //! A structure representing the ocean currents UBO.
    struct OceanCurrentsUBO
    {
        std::array<VelocityFieldUBO, MAX_OCEAN_CURRENTS> currents; //REMARK: type -> 0=uniform,1=jet,2=pipe,10=thruster
        glm::vec3 gravity;
        GLuint numCurrents;

        bool operator==(const OceanCurrentsUBO& other) const = default;
    };
    #pragma pack(0)

    struct OceanParams
    {
        OceanSpectrumUBO spectrumParams;
        bool propagateWaves;
    };

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

        //! A method to set ocean parameters.
        /*!
        \param p the ocean parameters
        */
        void UpdateOceanParams(const OceanParams& p);
        
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
         \param origin the origin point of the spectrum display
         \param size the size of the spectrum display
         */
        void ShowSpectrum(glm::vec2 origin, GLfloat size);
        
        //! A method used to display a specified texture.
        /*!
         \param layer the layer of the texture to display
         \param rect a rectangle in which to draw the texture on screen
         */
        void ShowFFTLayer(GLuint layer, glm::vec4 rect);
       
        //! A method that updates ocean currents information.
        void UpdateOceanCurrentsData(const OceanCurrentsUBO& data);

        //! A method to allocate a particle system for a view.
        /*!
         \param view a pointer to the view.
         \return a pointer to the allocated particle system
         */
        OpenGLOceanParticles* AllocateParticles(OpenGLView* view);

        //! A method to assign particle system to a view.
        /*!
         \param view a pointer to the view.
         \param particles a pointer to the particle system
         */
        void AssignParticles(OpenGLView* view, OpenGLOceanParticles* particles);

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

        //! A method return the ocean parameters.
        OceanParams getOceanParams() const;
        
    protected:
        void InitializeSimulation();
        
        std::map<OpenGLView*, OpenGLOceanParticles*> oceanParticles;
        glm::vec3 absorption[64];
        glm::vec3 scattering[64];
        OceanCurrentsUBO oceanCurrentsUBOData;
        GLfloat oceanSize;
        glm::vec4 gridSizes_;
        int fftSize_;
        GLuint fftLayers_;
        OceanParams params_;
        GLfloat propagationTime_;
        glm::vec3 lightAbsorption;
        glm::vec3 lightScattering;
        GLfloat waterTemperature;

        std::map<std::string, GLSLShader*> oceanShaders;
        std::map<std::string, GLuint> oceanTextures_;
        GLuint oceanFBOs[3];
        GLuint oceanSpectrumUBO_;
        GLuint oceanCurrentsUBO;
        
    private:
        int bitReverse(int i, int N);
        void computeWeight(int N, int k, float &Wr, float &Wi);    
        GLfloat* ComputeButterflyLookupTable(unsigned int size, unsigned int passes);
        
        float GetSlopeVariance(float kx, float ky, float *spectrumSample);
        float ComputeSlopeVariance();

        int oceanBoxObj;
        bool particlesEnabled;

        // FFT ocean waves generation
        GLuint fftPasses_;
        GLuint slopeVarianceSize_;
        GLfloat* spectrum12_;
        GLfloat* spectrum34_;
        std::random_device rd_;
        std::mt19937 gen_;
        std::normal_distribution<GLfloat> randomGaussian_;
        std::uniform_real_distribution<GLfloat> randomUniform_;
    };
}

#endif
