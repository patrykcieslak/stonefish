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
//  OpenGLDataStructs.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/11/2018.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLDataStructs__
#define __Stonefish_OpenGLDataStructs__

#include "utils/glad.h"
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <vector>

#define Max(a, b)   (((a) > (b)) ? (a) : (b))

#define DUMMY_COLOR glm::vec4(1.f, 0.4f, 0.1f, 1.f)
#define CONTACT_COLOR glm::vec4(1.f, 0, 0, 1.f)

#define MIN_INTENSITY_THRESHOLD 1.0f //Minimum light intensity to be considered [cd]
#define STD_NEAR_PLANE_DISTANCE 0.02f //Standard near plane distance of the cameras
#define STD_FAR_PLANE_DISTANCE 100000.f //Standard far plane distance of the cameras

//Standard texture unit bindings (OpenGL 3.x >=48; OpenGL 4.x >=80)
#define TEX_BASE                ((GLint)0)
#define TEX_GUI1                ((GLint)1)
#define TEX_GUI2                ((GLint)2)
#define TEX_POSTPROCESS1        ((GLint)3)
#define TEX_POSTPROCESS2        ((GLint)4)
#define TEX_POSTPROCESS3        ((GLint)5)
#define TEX_POSTPROCESS4        ((GLint)6)
#define TEX_POSTPROCESS5        ((GLint)7)
#define TEX_ATM_TRANSMITTANCE   ((GLint)8)
#define TEX_ATM_SCATTERING      ((GLint)9)
#define TEX_ATM_IRRADIANCE      ((GLint)10)
#define TEX_SUN_SHADOW          ((GLint)11)
#define TEX_SUN_DEPTH           ((GLint)12)
#define TEX_SPOT_SHADOW         ((GLint)13)
#define TEX_SPOT_DEPTH          ((GLint)14)
#define TEX_MAT_DIFFUSE         ((GLint)15)
#define TEX_MAT_NORMAL          ((GLint)16)
//#define TEX_POINT_SHADOW        ((GLint)X) 
//#define TEX_POINT_DEPTH         ((GLint)X)

//Standard UBO bindings
#define UBO_SUNSKY              ((GLuint)1)
#define UBO_LIGHTS              ((GLuint)2)
#define UBO_VIEW                ((GLuint)3)

//Standard SSBO bindings
#define SSBO_HISTOGRAM          ((GLuint)1)
#define SSBO_QTREE_IN           ((GLuint)2)
#define SSBO_QTREE_OUT          ((GLuint)3)
#define SSBO_QTREE_CULL         ((GLuint)4)
#define AC_QTREE_LOD            ((GLuint)5)
#define AC_QTREE_CULL           ((GLuint)6)

//Light params
#define MAX_POINT_LIGHTS        ((GLint)32)
#define MAX_SPOT_LIGHTS         ((GLint)32)
#define SPOT_LIGHT_SHADOWMAP_SIZE   ((GLint)2048)

class btTransform;

namespace sf
{
    //! An enum defining a type of a rendered primitive.
    typedef enum {POINTS, LINES, LINE_STRIP} PrimitiveType;
    
    //! A structure containing single vertex data.
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        
        Vertex()
        {
            pos = glm::vec3(0.f);
            normal = glm::vec3(0.f);
            uv = glm::vec3(-1.f);
        }
        
        friend bool operator==(const Vertex& lhs, const Vertex& rhs)
        {
            if(lhs.pos == rhs.pos && lhs.normal == rhs.normal && lhs.uv == rhs.uv)
                return true;
            return false;
        };
    };
    
    //! A structure containing single face data.
    struct Face
    {
        GLuint vertexID[3];
        
        friend bool operator==(const Face& lhs, const Face& rhs)
        {
            if(lhs.vertexID[0] == rhs.vertexID[0] && lhs.vertexID[1] == rhs.vertexID[1] && lhs.vertexID[2] == rhs.vertexID[2])
                return true;
            return false;
        };
    };
    
    //! A structure containing mesh data.
    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<Face> faces;
        bool hasUVs = true;
        
        glm::vec3 ComputeFaceNormal(size_t faceID)
        {
            glm::vec3 v12 = vertices[faces[faceID].vertexID[1]].pos - vertices[faces[faceID].vertexID[0]].pos;
            glm::vec3 v13 = vertices[faces[faceID].vertexID[2]].pos - vertices[faces[faceID].vertexID[0]].pos;
            return glm::normalize(glm::cross(v12,v13));
        }
        
        GLfloat ComputeFaceArea(size_t faceID)
        {
            glm::vec3 v12 = vertices[faces[faceID].vertexID[1]].pos - vertices[faces[faceID].vertexID[0]].pos;
            glm::vec3 v13 = vertices[faces[faceID].vertexID[2]].pos - vertices[faces[faceID].vertexID[0]].pos;
            return glm::length(glm::cross(v12,v13))/2.f;
        }
        
        void JoinMesh(Mesh* mesh)
        {
            size_t voffset = vertices.size();
            size_t foffset = faces.size();
            hasUVs = hasUVs && mesh->hasUVs;
            vertices.insert(vertices.end(), mesh->vertices.begin(), mesh->vertices.end());
            faces.insert(faces.end(), mesh->faces.begin(), mesh->faces.end());
            
            for(size_t i=foffset; i<faces.size(); ++i)
            {
                faces[i].vertexID[0] += voffset;
                faces[i].vertexID[1] += voffset;
                faces[i].vertexID[2] += voffset;
            }
        }
    };
    
    //! A structure containing object data.
    struct Object
    {
        GLuint vao;
        GLuint vboVertex;
        GLuint vboIndex;
        GLsizei faceCount;
    };
    
    //! An enum representing the type of look of an object.
    typedef enum {SIMPLE, PHYSICAL, MIRROR, TRANSPARENT} LookType;
    
    //! An enum representing the rendering mode.
    typedef enum {RAW, SHADOW, FLAT, FULL, UNDERWATER} DrawingMode;
    
    //! A structure containing data of a graphical material.
    struct Look
    {
        std::string name;
        LookType type;
        glm::vec3 color;
        std::vector<GLfloat> params;
        std::vector<GLuint> textures;
        GLfloat reflectivity;
    };
    
    //! A structure containing data of a view frustum.
    struct ViewFrustum
    {
        GLfloat near;
        GLfloat far;
        GLfloat fov;
        GLfloat ratio;
        glm::vec3 corners[8];
    };
    
    //! An enum defining supported color maps.
    typedef enum {COLORMAP_HOT = 0, COLORMAP_JET, COLORMAP_PERULA, COLORMAP_GREENBLUE} ColorMap;

    //! A structure representing a color system.
    struct ColorSystem
    {
        GLfloat xRed, yRed;     //Red x, y
        GLfloat xGreen, yGreen; //Green x, y
        GLfloat xBlue, yBlue;   //Blue x, y
        GLfloat xWhite, yWhite; //White point x, y
        GLfloat gamma;          //Gamma correction for system
        
        //! A static method returning the sRGB color space structure.
        static ColorSystem sRGB()
        {
            ColorSystem cs;
            cs.xRed = 0.64f;
            cs.yRed = 0.33f;
            cs.xGreen = 0.3f;
            cs.yGreen = 0.6f;
            cs.xBlue = 0.15f;
            cs.yBlue = 0.06f;
            cs.xWhite = 0.3127f;
            cs.yWhite = 0.3291f;
            cs.gamma = 0.f;
            return cs;
        }
    };
    
    //! A structure representing a color.
    struct Color
    {
        glm::vec3 rgb;
        
        //! A constructor.
        /*!
         \param R red component value
         \param G green component value
         \param B blue component value
         */
        Color(GLfloat R, GLfloat G, GLfloat B)
        {
            rgb = glm::vec3(R,G,B);
        }
        
        //! A static method used to create a new grayscale color.
        /*!
         \param L luminance (value)
         \return color structure
        */
        static Color Gray(GLfloat L)
        {
            return Color(L,L,L);
        }
        
        //! A static method used to create a new color based on RGB triplet.
        /*!
         \param R red component value
         \param G green component value
         \param B blue component value
         \return color structure
         */
        static Color RGB(GLfloat R, GLfloat G, GLfloat B)
        {
            return Color(R,G,B);
        }
        
        //! A static method used to create a new color based on HSV triplet.
        /*!
         \param H hue component value
         \param S staturation component value
         \param V value component value
         \return color structure
         */
        static Color HSV(GLfloat H, GLfloat S, GLfloat V)
        {
            const glm::vec4 K(1.f, 2.f/3.f, 1.f/3.f, 3.f);
            glm::vec3 p = glm::abs(glm::fract(glm::vec3(H) + glm::vec3(K.x, K.y, K.z)) * 6.f - glm::vec3(K.w));
            glm::vec3 c = V * glm::mix(glm::vec3(K.x), glm::clamp(p - glm::vec3(K.x), 0.f, 1.f), S);
            return Color(c.r, c.g, c.b);
        }
        
        //! A static method used to create a new color based on color temperature (black body temperature).
        /*!
         \param Kelvins the color temperature [K]
         \return color structure
         */
        static Color BlackBody(GLfloat Kelvins)
        {
            GLfloat c1, c2, c3;
            bbSpectrumToXYZ(Kelvins, c1, c2, c3);
            xyzToRGB(c1, c2, c3, c1, c2, c3, ColorSystem::sRGB());
            return Color(c1, c2, c3);
        }
        
    private:
        static GLfloat bbSpectrum(GLfloat wavelength, GLfloat temperature);
        static void bbSpectrumToXYZ(GLfloat temperature, GLfloat& x, GLfloat& y, GLfloat& z);
        static void xyzToRGB(GLfloat x, GLfloat y, GLfloat z, GLfloat& r, GLfloat& g, GLfloat& b, ColorSystem cs);
    };
    
    //! An enum used to designate type of helper object to be drawn.
    typedef enum {
        SOLID = 0, SOLID_CS, MULTIBODY_AXIS,
        HYDRO_CYLINDER, HYDRO_ELLIPSOID, HYDRO_CS, HYDRO_POINTS, HYDRO_LINES, HYDRO_LINE_STRIP,
        SENSOR_CS, SENSOR_LINES, SENSOR_LINE_STRIP, SENSOR_POINTS, ACTUATOR_LINES, JOINT_LINES,
        FORCE_GRAVITY, FORCE_BUOYANCY, FORCE_LINEAR_DRAG, FORCE_QUADRATIC_DRAG
    } RenderableType;
    
    //! A structure that represents a renderable object.
    struct Renderable
    {
        RenderableType type;
        int lookId;
        int objectId;
        std::string materialName;
        glm::mat4 model;
        std::vector<glm::vec3> points;
		
		static bool SortByMaterial(const Renderable& r1, const Renderable& r2) 
		{
			return r1.lookId < r2.lookId;
		}
    };
    
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
        RenderQuality aa;
        
        //! A constructor.
        RenderSettings()
        {
            windowW = 800;
            windowH = 600;
            shadows = RenderQuality::QUALITY_MEDIUM;
            ao = RenderQuality::QUALITY_MEDIUM;
            atmosphere = RenderQuality::QUALITY_MEDIUM;
            ocean = RenderQuality::QUALITY_MEDIUM;
            aa = RenderQuality::QUALITY_MEDIUM;
        }
    };
    
    //! A structure containing settings for drawing helper objects.
    struct HelperSettings
    {
        bool showCoordSys;
        bool showJoints;
        bool showActuators;
        bool showSensors;
        bool showFluidDynamics;
        bool showForces;
        bool showBulletDebugInfo;
        
        //! A constructor.
        HelperSettings()
        {
            showCoordSys = false;
            showJoints = false;
            showActuators = false;
            showSensors = false;
            showFluidDynamics = false;
            showForces = false;
            showBulletDebugInfo = false;
        }
    };
    
    typedef btTransform Transform;
    
    //! A method converting a physics engine transform into an OpenGL matrix.
    /*!
     \param T transform
     \return OpenGL 4x4 matrix
     */
    glm::mat4 glMatrixFromTransform(const Transform& T);
}

#endif
