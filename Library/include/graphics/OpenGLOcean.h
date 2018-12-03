//
//  OpenGLOcean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLOcean__
#define __Stonefish_OpenGLOcean__

#include <SDL2/SDL_mutex.h>
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //!
    struct OceanParams
    {
        unsigned int passes;
        unsigned int slopeVarianceSize;
        int fftSize;
        glm::vec4 gridSizes;
        float* spectrum12;
        float* spectrum34;
        bool propagate;
        float wind;
        float omega;
        float km;
        float cm;
        float A;
        float t;
    };
    
    //!
    class OpenGLOcean
    {
    public:
        OpenGLOcean(bool geometricWaves, SDL_mutex* hydrodynamics);
        ~OpenGLOcean();
        
        void Simulate();
        void UpdateSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection);
        void DrawSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture, GLint* viewport);
        void DrawBacksurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture, GLint* viewport);
        void DrawUnderwaterMask(glm::mat4 view, glm::mat4 projection);
        void DrawBackground(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection);
        void DrawVolume(GLuint sceneTexture, GLuint linearDepthTex);
        
        void ShowSpectrum(glm::vec2 viewportSize, glm::vec4 rect);
        void ShowTexture(int id, glm::vec4 rect);
        
        GLfloat getWaveHeight(GLfloat x, GLfloat y);
        void setTurbidity(GLfloat t);
        GLfloat getTurbidity();
        void setLightAbsorption(glm::vec3 la);
        glm::vec3 getLightAbsorption();
        
    private:
        GLfloat* ComputeButterflyLookupTable(unsigned int size, unsigned int passes);
        GLfloat ComputeInterpolatedWaveData(GLfloat x, GLfloat y, GLuint channel);
        int bitReverse(int i, int N);
        void computeWeight(int N, int k, float &Wr, float &Wi);
        float ComputeSlopeVariance();
        float GetSlopeVariance(float kx, float ky, float *spectrumSample);
        void GenerateWavesSpectrum();
        void GetSpectrumSample(int i, int j, float lengthScale, float kMin, float *result);
        float spectrum(float kx, float ky, bool omnispectrum = false);
        float omega(float k);
        float sqr(float x);
        
        std::vector<GLSLShader*> oceanShaders;
        GLuint oceanFBOs[3];
        GLuint oceanTextures[6];
        
        bool waves;
        SDL_mutex* hydroMutex;
        GLfloat* fftData;
        OceanParams params;
        QuadTree* qt;
        int64_t lastTime;
        GLuint vao;
        GLuint vbo;
        GLuint vaoEdge;
        GLuint vboEdge;
        GLuint vaoMask;
        GLuint vboMask;
        glm::vec3 lightAbsorption;
        GLfloat turbidity;
    };
}

#endif
