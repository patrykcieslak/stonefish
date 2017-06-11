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
struct Vertex 
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
	
	friend bool operator==(const Vertex& lhs, const Vertex& rhs)
	{
		if(lhs.pos == rhs.pos && lhs.normal == rhs.normal && lhs.uv == rhs.uv)
			return true;
		return false;
	};
};

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

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
	bool hasUVs;
	
	glm::vec3 computeFaceNormal(unsigned int faceID)
	{
		glm::vec3 v12 = vertices[faces[faceID].vertexID[1]].pos - vertices[faces[faceID].vertexID[0]].pos;
		glm::vec3 v13 = vertices[faces[faceID].vertexID[2]].pos - vertices[faces[faceID].vertexID[0]].pos;
		return glm::normalize(glm::cross(v12,v13));
	}
};

struct Object
{
	Mesh* mesh;
	GLuint vao;
	GLuint vboVertex;
	GLuint vboIndex;
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
	
	//Deprecated
	void SetupOrtho();
    
	void Init();
	void DestroyContent();
	
	//Draw
	void DrawObject(int modelId, int lookId, btScalar* transform);
	void DrawSAQ();
	void DrawCoordSystem(GLfloat size);
	
	//Allocate and build content
	unsigned int BuildObject(Mesh* mesh);
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
	std::vector<Object> objects; //VBAs
	std::vector<Look> looks; //OpenGL materials
	GLuint saq; //screen-aligned quad VBO

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