//
//  OpenGLContent.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/06/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLContent__
#define __Stonefish_OpenGLContent__

#include <vector>
#include "OpenGLPipeline.h"
#include "GLSLShader.h"

//Geometry
typedef enum {POINTS, LINES, LINE_STRIP} PrimitiveType;

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
typedef enum {SIMPLE, PHYSICAL, METAL, MIRROR, TRANSPARENT} LookType;

struct Look
{
    LookType type;
    glm::vec3 color;
    std::vector<GLfloat> params;
    std::vector<GLuint> textures;
    
    Look()
    {
        type = LookType::SIMPLE;
        color = glm::vec3(0.5f,0.5f,0.5f);
        params.push_back(0);
		params.push_back(0);
    }
};

class OpenGLView;
class OpenGLLight;

class OpenGLContent
{
public:
	static OpenGLContent* getInstance(); //Singleton
	static void Destroy();
	void Init();
	void DestroyContent();
	
	//Draw
	void SetViewportSize(unsigned int width, unsigned int height);
	void SetProjectionMatrix(glm::mat4 P);
	void SetViewMatrix(glm::mat4 V);
	void SetDrawFlatObjects(bool enable);
	void SetCurrentView(OpenGLView* v);
	
	void DrawSAQ();
	void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, glm::vec4 color = glm::vec4(1.f));
	void DrawCubemapCross(GLuint texture);
	void DrawCoordSystem(glm::mat4 M, GLfloat size);
	void DrawPrimitives(PrimitiveType type, std::vector<glm::vec3>& vertices, glm::vec4 color, glm::mat4 M = glm::mat4());
	void DrawObject(int modelId, int lookId, const glm::mat4& M);
	
	//Allocate and build content
	void AddView(OpenGLView* view);
    void AddLight(OpenGLLight* light);
	unsigned int BuildObject(Mesh* mesh);
	unsigned int CreateSimpleLook(glm::vec3 rgbColor, GLfloat specular, GLfloat shininess, const char* textureName = NULL);
	unsigned int CreatePhysicalLook(glm::vec3 rgbColor, GLfloat diffuseReflectance, GLfloat roughness, GLfloat IOR, const char* textureName = NULL);
	void UseLook(unsigned int lookId, const glm::mat4& M);
	
	OpenGLView* getView(unsigned int id);
	unsigned int getViewsCount();
	OpenGLLight* getLight(unsigned int id);
	unsigned int getLightsCount();
	
	//Static
	static GLuint LoadTexture(const char* filename);
	static GLuint LoadInternalTexture(const char* filename);
	static Mesh* LoadMesh(const char* filename, GLfloat scale, bool smooth);
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
	//Modes
	bool drawFlatObjects; //For shadow casting

	//Data
	std::vector<OpenGLView*> views;
    std::vector<OpenGLLight*> lights;
	std::vector<Object> objects; //VBAs
	std::vector<Look> looks; //OpenGL materials
	glm::vec3 eye;
	glm::mat4 view; //Current view matrix;
	glm::mat4 projection; //Current projection matrix
	glm::mat4 viewProjection; //Current view-projection matrix
	glm::vec2 viewportSize; //Current view-port size
	
	
	//Standard objects
	GLuint baseVertexArray; //base VAO
	GLuint saqBuf; //screen-aligned quad VBO
	GLuint cubeBuf; //cubemap cross VBO
	GLuint csBuf[2]; //vertex data for drawing coord systems 
	GLSLShader* helperShader;
	GLSLShader* texQuadShader;
	GLSLShader* texCubeShader;
	GLSLShader* flatShader;
	std::vector<GLSLShader*> materialShaders;

	//Singleton
	OpenGLContent();
	~OpenGLContent();
	static OpenGLContent* instance;
	
	//Methods
	void UseStandardLook(const glm::mat4& M);
	void SetupLights(GLSLShader* shader);
	
	//Static
	static Mesh* LoadSTL(const char *filename, GLfloat scale, bool smooth);
	static Mesh* LoadOBJ(const char *filename, GLfloat scale, bool smooth);
	
};

#endif