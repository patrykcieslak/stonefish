//
//  OpenGLContent.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/06/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLContent__
#define __Stonefish_OpenGLContent__

#include "OpenGLPipeline.h"
#include <vector>

//Geometry
struct Face
{
    GLuint vertexID[3];
    GLuint normalID[3];
	GLuint uvID[3];
    
    friend bool operator==(const Face& lhs, const Face& rhs)
    {
        if(lhs.vertexID[0] != rhs.vertexID[0] || lhs.vertexID[1] != rhs.vertexID[1] || lhs.vertexID[2] != rhs.vertexID[2])
            return false;
        
        return true;
    };
};

struct Mesh
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
	std::vector<Face> faces;
	
	glm::vec3 computeFaceNormal(unsigned int faceID)
	{
		glm::vec3 v12 = vertices[faces[faceID].vertexID[1]] - vertices[faces[faceID].vertexID[0]];
		glm::vec3 v13 = vertices[faces[faceID].vertexID[2]] - vertices[faces[faceID].vertexID[0]];
		return glm::normalize(glm::cross(v12,v13));
	}
};

//Rendering styles
typedef enum {OPAQUE, REFLECTIVE, TRANSPARENT} LookType;

struct Look
{
    LookType type;
    glm::vec3 color;
    glm::vec4 data;
    GLuint texture;
    GLfloat textureMix;
    
    Look()
    {
        type = OPAQUE;
        color = glm::vec3(1.f,1.f,1.f);
        data = glm::vec4(0.2f, 1.33f, 0.2f, 0.0f);
        texture = 0;
        textureMix = 0.f;
    }
};

class OpenGLContent
{
public:
	static OpenGLContent* getInstance(); //Singleton
	static void Destroy();
	
	unsigned int BuildObject(Mesh* mesh);
	void DrawObject(int modelId, int lookId, btScalar* transform);
	unsigned int CreateOpaqueLook(glm::vec3 rgbColor, GLfloat diffuseReflectance, GLfloat roughness, GLfloat IOR, const char* textureName = NULL, GLfloat textureMixFactor = GLfloat(1.0f));
	void UseLook(unsigned int lookId);
	
	//Static
	static GLuint LoadTexture(const char* filename);
	static GLuint LoadInternalTexture(const char* filename);
	static Mesh* LoadMesh(const char* filename, btScalar scale, bool smooth);
	static Mesh* BuildPlane(GLfloat halfExtents);
	static Mesh* BuildBox(glm::vec3 halfExtents, unsigned int subdivisions = 3);
	static Mesh* BuildSphere(GLfloat radius, unsigned int subdivisions = 3);
	static Mesh* BuildCylinder(GLfloat radius, GLfloat height, unsigned int slices = 24);
	static Mesh* BuildTorus(GLfloat majorRadius, GLfloat minorRadius, unsigned int majorSlices = 48, unsigned int minorSlices = 24);
	static void SmoothNormals(Mesh* mesh);
	static void Subdivide(Mesh* mesh, bool icoMode = false);
	static void AABB(Mesh* mesh, btVector3& min, btVector3& max);
	static void AABS(Mesh* mesh, btScalar& bsRadius, btVector3& bsCenterOffset);
	
private:
	//Data
	std::vector<GLuint> objects; //VBAs
	std::vector<Look> looks; //OpenGL materials

	//Singleton
	OpenGLContent();
	~OpenGLContent();
	static OpenGLContent* instance;
	
	//Methods
	void UseStandardLook();
	
	//Static
	static Mesh* LoadSTL(const char *filename, btScalar scale, bool smooth);
	static Mesh* LoadOBJ(const char *filename, btScalar scale, bool smooth);
};

#endif