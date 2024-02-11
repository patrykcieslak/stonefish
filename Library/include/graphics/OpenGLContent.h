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
//  OpenGLContent.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/06/2017.
//  Copyright (c) 2017-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLContent__
#define __Stonefish_OpenGLContent__

#include "graphics/OpenGLDataStructs.h"
#include "graphics/GLSLShader.h"
#include "core/NameManager.h"
#include "graphics/OpenGLPointLight.h"
#include "graphics/OpenGLSpotLight.h"
#include <map>

namespace sf
{
    class OpenGLView;

    //! An enum specifiying supported texture filtration modes.
    enum class FilteringMode {NEAREST, BILINEAR, BILINEAR_MIPMAP, TRILINEAR};

    //! Framebuffer texture
    struct FBOTexture
    {
        GLenum attach;
        GLenum target;
        GLuint tex;
        GLint lvl;
        GLint z;

        FBOTexture(GLenum attachment, GLenum target, GLuint texture, GLint level = 0, GLint zoffset = 0)
        : attach(attachment), target(target), tex(texture), lvl(level), z(zoffset) {}
    };
    
    #pragma pack(1)
    //! A structure representing data of the Lights UBO (std140 aligned)
    struct LightsUBO
    {
        PointLightUBO pointLights[MAX_POINT_LIGHTS];
        SpotLightUBO spotLights[MAX_SPOT_LIGHTS];
        GLint numPointLights;
        GLint numSpotLights;
    };
    //! A structure representing a velocity field UBO (std140 aligned).
    struct VelocityFieldUBO
    {
        glm::vec4 posR;   //Position and orifice radius
        glm::vec4 dirV;   //Direction and velocity value
        glm::vec3 params; //Additional params
        GLuint type;      //Type of velocity field
    };
    #pragma pack(0)

    //! A structure representing a material shader collection.
    struct MaterialShader
    {
        std::string shadingAlgorithm;
        GLSLShader* shaders[6];

        MaterialShader()
        {
            shadingAlgorithm = "";
        }

        MaterialShader(const MaterialShader &obj)
        {
            shadingAlgorithm = obj.shadingAlgorithm;
            for(size_t i=0; i<6; ++i)
                shaders[i] = obj.shaders[i];
        }
    };

    //! A class implementing OpenGL content management and core rendering funtions.
    class OpenGLContent
    {
    public:
        //! A constructor.
        OpenGLContent();
        
        //! A destructor.
        ~OpenGLContent();
        
        //! A method implementing necessary initializations.
        void Finalize();
        
        //! A method that destroys all created OpenGL content.
        void DestroyContent();
        
        //! A method to set the current rendering viewport size.
        /*!
         \param width the width of the viewport
         \param height the height of the viewport
         */
        void SetViewportSize(unsigned int width, unsigned int height);
        
        //! A method to set current projection matrix.
        /*!
         \param P projection matrix
         */
        void SetProjectionMatrix(glm::mat4 P);
        
        //! A method to set current view matrix.
        /*!
         \param V view matrix
         */
        void SetViewMatrix(glm::mat4 V);
        
        //! A method returning the view matrix.
        glm::mat4 GetViewMatrix();
        
        //! A method to set the current drawing mode.
        /*!
         \param m drawing mode
         */
        void SetDrawingMode(DrawingMode m);
        
        //! A method to set current view.
        /*!
         \param v a pointer to a view object
         */
        void SetCurrentView(OpenGLView* v);
        
        //! A method that binds the base vertex array.
        void BindBaseVertexArray();
        
        //! A method to draw a screen-aligned quad.
        void DrawSAQ();
        
        //! A method to draw a textured screen-aligned quad.
        /*! 
         \param texture the id of the texture
         \param color a color that is multiplied with the texture color
         */
        void DrawTexturedSAQ(GLuint texture, glm::vec4 color = glm::vec4(1.f));
        
        //! A method to draw a textured quad (2D texture).
        /*!
         \param x the x coordinate of the quad origin
         \param y the y coordinate of the quad origin
         \param width the width of the quad
         \param height the height of the quad
         \param texture the id of the texture
         \param color a color that is multiplied with the texture color
         */
        void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, glm::vec4 color = glm::vec4(1.f));
        
        //! A method to draw a textured quad (array or 3D texture).
        /*!
         \param x the x coordinate of the quad origin
         \param y the y coordinate of the quad origin
         \param width the width of the quad
         \param height the height of the quad
         \param texture the id of the texture
         \param z the layer of the texture
         \param array has to be set to true if texture is an array texture
         */
        void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, GLint z, bool array = true);
         
        //! A method to display a cubemap texture.
        /*!
         \param texture the id of the texture
         */
        void DrawCubemapCross(GLuint texture);
        
        //! A method to draw a Cartesian coord system with color-coded axes.
        /*!
         \param M the model matrix
         \param size the length of the axes [m]
         */
        void DrawCoordSystem(glm::mat4 M, GLfloat size);
        
        //! A method to draw an ellipsoid.
        /*!
         \param M the model matrix
         \param radii a vector of 3 radiuses of the ellipsoid
         \param color the color to be used when rendering
         */
        void DrawEllipsoid(glm::mat4 M, glm::vec3 radii, glm::vec4 color);
        
        //! A method to draw a cylinder.
        /*!
         \param M the model matrix
         \param dims the dimensions of the cylinder [m]
         \param color the color to be used when rendering
         */
        void DrawCylinder(glm::mat4 M, glm::vec3 dims, glm::vec4 color);
        
        //! A method to draw primitives.
        /*!
         \param type the type of the primitive
         \param vertices a list of vertices of the primitives
         \param color the color to be used when drawing
         \param M the model matrix
         */
        void DrawPrimitives(PrimitiveType type, std::vector<glm::vec3>& vertices, glm::vec4 color, glm::mat4 M = glm::mat4(1.f));
        
        //! A method to draw an object.
        /*!
         \param objectId the id of the graphical object
         \param lookId the id of the graphical material
         \param M the model matrix
         */
        void DrawObject(int objectId, int lookId, const glm::mat4& M);

        //! A method to draw the light source.
        /*!
         \param lightId the id of the light
         */
        void DrawLightSource(unsigned int lightId);

        //! A method to add a view to the list of views.
        /*!
         \param view a pointer to a view object
         */
        void AddView(OpenGLView* view);
        
        //! A method to add light to the list of lights.
        /*!
         \param light a pointer to a light object
         */
        void AddLight(OpenGLLight* light);
        
        //! A method to build a graphical object from a mesh structure.
        /*!
         \param mesh a pointer to the mesh structure
         \return an id of the built object
         */
        unsigned int BuildObject(Mesh* mesh);
        
        //! A method to create a new simple look.
        /*!
         \param name the name of the look
         \param rgbColor the diffuse color
         \param specular the specular strength
         \param shininess the shininess factor
         \param reflectivity the amount of reflection
         \param albedoTexturePath a path to the texture file specifying albedo color
         \return the actual name of the created look
         */
        std::string CreateSimpleLook(const std::string& name, glm::vec3 rgbColor, GLfloat specular, GLfloat shininess, 
                                     GLfloat reflectivity = 0.f, const std::string& albedoTexturePath = "");
        
        //! A method to create a new physical look.
        /*!
         \param name the name of the look
         \param rgbColor the diffuse color
         \param roughness the roughness of the surface
         \param metalness the amount of metal look
         \param relfectivity the amount of reflection
         \param albedoTexturePath a path to the texture file specifying albedo color
         \param normalTexturePath a path to the texture file specifying surface normal (bump mapping)
         \return the actual name of the created look
         */
        std::string CreatePhysicalLook(const std::string& name, glm::vec3 rgbColor, GLfloat roughness, GLfloat metalness = 0.f, 
                                       GLfloat reflectivity = 0.f, const std::string& albedoTexturePath = "", const std::string& normalTexturePath = "");
        
        //! A method to use a look.
        /*!
         \param lookId an id of the look to use
         \param texturable a flag determining if the object rendered is texturable
         \param M the model matrix
         */
        void UseLook(unsigned int lookId, bool texturable, const glm::mat4& M);
        
        //! A method returning a pointer to a view.
        /*!
         \param id the index of the view
         \return a pointer to the view object
         */
        OpenGLView* getView(size_t id);
        
        //! A method returning the number of views.
        size_t getViewsCount();
        
        //! A method returning a pointer to a light.
        /*!
         \param id the index of the light
         \return a pointer to the light object
         */
        OpenGLLight* getLight(size_t id);
        
        //! A method returning the number of lights.
        size_t getLightsCount();
        
        //! A method that updates lights UBO.
        void SetupLights();
        
        //! A method returing the id of a look.
        /*!
         \param name the name of the look
         \return the id of the corresponding look structure
         */
        int getLookId(std::string name);

        //! A method returning a reference to the object structure.
        /*!
         \param id the id of the object
         */
        const Object& getObject(size_t id);

        //! A method returning a reference to the look structure.
        /*!
         \param id the id of the look
         */
        const Look& getLook(size_t id);
        
        //! A static method to load a texture.
        /*!
         \param filename the path to the texture file
         \param hasAlphaChannel a flag to indicate if the texture has transparency
         \param anisotropy defines maximum anisotropic filtering
         \param internal a flag to indicate if the texture is an internal resource
         \return the id of the loaded texture
         */
        static GLuint LoadTexture(std::string filename, bool hasAlphaChannel = false, GLfloat anisotropy = 0.f, bool internal = false);
        
        //! A static method to load an internal texture.
        /*!
         \param filename the name of the texture file
         \param hasAlphaChannel a flag to indicate if the texture has transparency
         \param anisotropy defines maximum anisotropic filtering
         \return the id of the loaded texture
         */
        static GLuint LoadInternalTexture(std::string filename, bool hasAlphaChannel = false, GLfloat anisotropy = 0.f);
        
        //! A static method to generate a new texture.
        /*!
         \param target the OpenGL texture target enum
         \param dimensions the dimensions of the texture [px]
         \param internalFormat the OpenGL internal texture format enum
         \param format the OpenGL texture format enum
         \param type the OpenGL data type enum
         \param data a pointer to the data of the texture or NULL
         \param fm the filtering mode
         \param repeat a flag indicating if texture repeat mode should be used
         \param anisotropy a flag indicating if anisotropic filtering should be enabled
         \return a texture handle
        */
        static GLuint GenerateTexture(GLenum target, glm::uvec3 dimensions, GLenum internalFormat, GLenum format, 
                                      GLenum type, const void* data, FilteringMode fm, bool repeat, bool anisotropy = false);

        //! A static method to create a framebuffer object.
        /*!
         \param textures a vector of framebuffer textures 
         \return a framebuffer object handle
         */
        static GLuint GenerateFramebuffer(const std::vector<FBOTexture>& textures);

        //! A static method to load a mesh from a file.
        /*!
         \param filename a path to the model file
         \param scale the scale of the model
         \param smooth a flag to decide if model normals should be smoothed after loading
         \return a pointer to the allocated mesh structure
         */
        static Mesh* LoadMesh(std::string filename, GLfloat scale, bool smooth);
        
        //! A static method to build a graphical plane object.
        /*!
         \param halfExtents the size of the plane [m]
         \param uvScale scaling of the texture coordinates
         \return a pointer to the allocated mesh structure
         */
        static Mesh* BuildPlane(GLfloat halfExtents, GLfloat uvScale = 1.f);
        
        //! A static method to build a graphical box object.
        /*!
         \param halfExtents the dimensions of the box (half of side lengths) [m]
         \param subdivisions number of subdivisions used when generating the mesh (needed for hydrodynamics)
         \param uvMode texture coordinates generation mode (0 - texture cross, 1 - same texture on all faces, 2 - coordinates synced with dimensions (for grid))
         \return a pointer to the allocated mesh structure
         */
        static Mesh* BuildBox(glm::vec3 halfExtents, unsigned int subdivisions = 3, unsigned int uvMode = 0);
        
        //! A static method to build a graphical sphere object. The volume of the discretized sphere is equal to the volume of an analytical sphere.
        /*!
         \param radius the radius of the sphere [m]
         \param subdivisions number of subdivisions used when generating the mesh (needed for hydrodynamics)
         \return a pointer to the allocated mesh structure
         */
        static Mesh* BuildSphere(GLfloat radius, unsigned int subdivisions = 3);
        
        //! A static method to build a graphical cylinder object. The volume of the discretized cylinder is equal to the volume of an analytical cylinder.
        /*!
         \param radius the radius of the cylinder [m]
         \param height the height of the cylinder [m]
         \param slices number of slices used when computing the cylinder
         \return a pointer to the allocated mesh structure
         */
        static Mesh* BuildCylinder(GLfloat radius, GLfloat height, unsigned int slices = 24);
        
        //! A static method to build a graphical torus object. The volume of the discretized torus is equal to the volume of an analytical torus.
        /*!
         \param majorRadius the radius of the torus [m]
         \param minorRadius the radius of the tube [m]
         \param majorSlices number of slices used when computing the torus
         \param minorSlices number of slices used when computing the torus
         \return a pointer to the allocated mesh structure
         */
        static Mesh* BuildTorus(GLfloat majorRadius, GLfloat minorRadius, unsigned int majorSlices = 48, unsigned int minorSlices = 24);
        
        //! A static method to build a graphical wing object.
        /*!
         \param baseChordLength the length of the chord at the base of the wing [m]
         \param tipChordLength the length of the chord at the tip of the wing [m]
         \param maxCamber the camber of the wing
         \param maxCamper pos the position of the max camber point
         \param profileThickness the thickness of the profile [m]
         \param wingLength the length of the wing [m]
         \return a pointer to the allocated mesh structure
         */
        static Mesh* BuildWing(GLfloat baseChordLength, GLfloat tipChordLength, GLfloat maxCamber, GLfloat maxCamberPos, GLfloat profileThickness, GLfloat wingLength);
        
        //! A static method to build a graphical terrain object.
        /*!
         \param heightfield a pointer to the heightfield data
         \param sizeX the size of the data in X
         \param sizeY the size of the data in Y
         \param scaleX the scale of the terrain in X
         \param scaleY the scale of the terrain in Y
         \param maxHeight the height of the terrain [m]
         \param uvScale scaling of the texture coordinates
         \return a pointer to the allocated mesh structure
         */
        static Mesh* BuildTerrain(GLfloat* heightfield, int sizeX, int sizeY, GLfloat scaleX, GLfloat scaleY, GLfloat maxHeight, GLfloat uvScale = 1.f);
        
        //! A static method to transform mesh vertices.
        /*!
         \param mesh a pointer to a mesh structure
         \param T the transformation to be applied
         */
        static void TransformMesh(Mesh* mesh, const Transform& T);
        
        //! A static method to check if normals correspond to the vertex order and fix the errors.
        /*!
         \param mesh a pointer to a mesh structure
         */
        static void CheckAndRepairFaceVertexOrder(Mesh* mesh);

        //! A static method to smooth the mesh normals.
        /*!
         \param mesh a pointer to a mesh structure
         */
        static void SmoothNormals(Mesh* mesh);

        //! A static method to compute tangent and bitangent vectors.
        /*!
         \param mesh a pointer to a mesh structure
         */
        static void ComputeTangents(TexturableMesh* mesh);
        
        //! A method to compute average face size.
        /*!
         \param mesh a pointer to a mesh structure
         */
        static GLfloat ComputeAverageFaceArea(Mesh* mesh);
        
        //! A method to subdivide the mesh.
        /*!
         \param mesh a pointer to a mesh structure
         \param icoMode the icosahedron mode flag
         */
        static void Subdivide(Mesh* mesh, bool icoMode = false);
        
        //! A method to uniformize mesh face sizes.
        /*!
         \param mesh a pointer to a mesh structure
         \param sizeThreshold faces with an area larger than this parameter multiplied by average face size are subdivided
         */
        static void Refine(Mesh* mesh, GLfloat sizeThreshold);
        
        //! A method to compute the axis-aligned bounding box of a mesh.
        /*!
         \param mesh a pointer to a mesh structure
         \param min a reference to a variable that will store the min corner
         \param max a reference to a variable that will store the max corner
         */
        static void AABB(Mesh* mesh, glm::vec3& min, glm::vec3& max);
        
        //! A method to compute the axis-aligned bounding sphere of a mesh.
        /*!
         \param mesh a pointer to a mesh structure
         \param bsRadius a reference to a variable that will store the sphere radius
         \param bsCenterOffset a reference to a variable that will store the sphere center position
         */
        static void AABS(Mesh* mesh, GLfloat& bsRadius, glm::vec3& bsCenterOffset);
        
    private:
        //Modes
        DrawingMode mode;
        GLfloat maxAnisotropy;
        
        //Data
        std::vector<OpenGLView*> views;
        std::vector<OpenGLLight*> lights;
        std::vector<Object> objects; //VBAs
        std::vector<Look> looks; //OpenGL materials
        NameManager lookNameManager;
        int currentLookId;
        bool currentTexturable;
        int currentShaderMode;
        
        glm::vec3 eyePos;
        glm::vec3 viewDir;
        glm::mat4 view; //Current view matrix;
        glm::mat4 projection; //Current projection matrix
        glm::mat4 viewProjection; //Current view-projection matrix
        glm::vec2 viewportSize; //Current view-port size
        GLfloat FC; //Current logarithmic depth buffer constant
        
        //Standard objects
        GLuint baseVertexArray; //base VAO
        GLuint cubeBuf; //cubemap cross VBO
        GLuint csBuf[2]; //vertex data for drawing coord systems
        Object ellipsoid; //used for approximating fluid dynamics coeffs
        Object cylinder; //used for approximating fluid dynamics coeffs
        GLuint lightsUBO;
        LightsUBO lightsUBOData;
        GLuint viewUBO;
        
        //Shaders
        std::map<std::string, GLSLShader*> basicShaders;
        std::vector<MaterialShader> materialShaders;
        GLSLShader* lightSourceShader[2];
        
        //Methods
        void UseStandardLook(const glm::mat4& M);
    };
}

#endif
