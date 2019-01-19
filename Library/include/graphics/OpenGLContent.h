//
//  OpenGLContent.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/06/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLContent__
#define __Stonefish_OpenGLContent__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class GLSLShader;
    class OpenGLView;
    class OpenGLLight;
    
    //!
    class OpenGLContent
    {
    public:
        OpenGLContent();
        ~OpenGLContent();
        
        void Finalize();
        void DestroyContent();
        
        //Draw
        void SetViewportSize(unsigned int width, unsigned int height);
        void SetProjectionMatrix(glm::mat4 P);
        void SetViewMatrix(glm::mat4 V);
        glm::mat4 GetViewMatrix();
        void SetDrawingMode(DrawingMode m);
        void SetCurrentView(OpenGLView* v, bool mirror = false);
        void BindBaseVertexArray();
        void EnableClipPlane(glm::vec4 clipPlaneCoeff);
        void DisableClipPlane();
        
        void DrawSAQ();
        void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, glm::vec4 color = glm::vec4(1.f));
        void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, GLint z, bool array = true);
        void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint textureMS, glm::ivec2 texSize);
        
        void DrawCubemapCross(GLuint texture);
        void DrawCoordSystem(glm::mat4 M, GLfloat size);
        void DrawEllipsoid(glm::mat4 M, glm::vec3 radii, glm::vec4 color);
        void DrawCylinder(glm::mat4 M, glm::vec3 dims, glm::vec4 color);
        void DrawPrimitives(PrimitiveType type, std::vector<glm::vec3>& vertices, glm::vec4 color, glm::mat4 M = glm::mat4(1.f));
        void DrawObject(int modelId, int lookId, const glm::mat4& M);
        
        //Allocate and build content
        void AddView(OpenGLView* view);
        void AddLight(OpenGLLight* light);
        unsigned int BuildObject(Mesh* mesh);
        unsigned int CreateSimpleLook(glm::vec3 rgbColor, GLfloat specular, GLfloat shininess, GLfloat reflectivity = 0.f, std::string textureName = "");
        unsigned int CreatePhysicalLook(glm::vec3 rgbColor, GLfloat roughness, GLfloat metalness = 0.f, GLfloat reflectivity = 0.f, std::string textureName = "");
        void UseLook(unsigned int lookId, const glm::mat4& M);
        
        OpenGLView* getView(unsigned int id);
        unsigned int getViewsCount();
        OpenGLLight* getLight(unsigned int id);
        unsigned int getLightsCount();
        
        //Static
        static GLuint LoadTexture(std::string filename);
        static GLuint LoadInternalTexture(std::string filename);
        static Mesh* LoadMesh(std::string filename, GLfloat scale, bool smooth);
        static Mesh* BuildPlane(GLfloat halfExtents);
        static Mesh* BuildBox(glm::vec3 halfExtents, unsigned int subdivisions = 3);
        static Mesh* BuildSphere(GLfloat radius, unsigned int subdivisions = 3);
        static Mesh* BuildCylinder(GLfloat radius, GLfloat height, unsigned int slices = 24);
        static Mesh* BuildTorus(GLfloat majorRadius, GLfloat minorRadius, unsigned int majorSlices = 48, unsigned int minorSlices = 24);
        static Mesh* BuildWing(GLfloat baseChordLength, GLfloat tipChordLength, GLfloat maxCamber, GLfloat maxCamberPos, GLfloat profileThickness, GLfloat wingLength);
        static Mesh* BuildTerrain(GLfloat* heightfield, int sizeX, int sizeY, GLfloat scaleX, GLfloat scaleY, GLfloat maxHeight);
        static void TransformMesh(Mesh* mesh, const Transform& T);
        static void SmoothNormals(Mesh* mesh);
        static void Subdivide(Mesh* mesh, bool icoMode = false);
        static void AABB(Mesh* mesh, glm::vec3& min, glm::vec3& max);
        static void AABS(Mesh* mesh, GLfloat& bsRadius, glm::vec3& bsCenterOffset);
        
    private:
        //Modes
        DrawingMode mode;
        
        //Data
        std::vector<OpenGLView*> views;
        std::vector<OpenGLLight*> lights;
        std::vector<Object> objects; //VBAs
        std::vector<Look> looks; //OpenGL materials
        glm::vec3 eyePos;
        glm::vec3 viewDir;
        glm::mat4 view; //Current view matrix;
        glm::mat4 projection; //Current projection matrix
        glm::mat4 viewProjection; //Current view-projection matrix
        glm::vec2 viewportSize; //Current view-port size
        glm::vec4 clipPlane;
        
        //Standard objects
        GLuint baseVertexArray; //base VAO
        GLuint quadBuf; //quad for debugging textures
        GLuint cubeBuf; //cubemap cross VBO
        GLuint csBuf[2]; //vertex data for drawing coord systems
        Object ellipsoid; //used for approximating fluid dynamics coeffs
        Object cylinder; //used for approximating fluid dynamics coeffs
        
        //Shaders
        GLSLShader* helperShader;
        GLSLShader* texQuadShader;
        GLSLShader* texQuadMSShader;
        GLSLShader* texLayerQuadShader;
        GLSLShader* texLevelQuadShader;
        GLSLShader* texCubeShader;
        GLSLShader* flatShader;
        std::vector<GLSLShader*> materialShaders;
        
        //Methods
        void UseStandardLook(const glm::mat4& M);
        void SetupLights(GLSLShader* shader);
    };
}

#endif
