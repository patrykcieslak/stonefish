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

struct QuadTree;

struct QuadTreeNode
{
	unsigned int level;
	glm::vec3 origin;
	glm::vec2 size;
	glm::vec4 edgeFactors;
	
	bool leaf;
	QuadTreeNode* parent;
	QuadTreeNode* child[4];
	QuadTree* tree;
	
	QuadTreeNode(glm::vec3 _origin, glm::vec2 _size, QuadTreeNode* _parent, QuadTree* _tree)
	{
		if(_parent != NULL)
			level = _parent->level+1;
		else
			level = 0;
			
		origin = _origin;
		size = _size;
		edgeFactors = glm::vec4(1);
	
		leaf = true;
		parent = _parent;
		child[0] = NULL;
		child[1] = NULL;
		child[2] = NULL;
		child[3] = NULL;
		tree = _tree;
	}
	
	~QuadTreeNode()
	{
		if(!leaf)
		{
			delete child[0];
			delete child[1];
			delete child[2];
			delete child[3];
		}
	}
		
	void Grow(glm::vec3 p, glm::mat4 VP);
};

struct QuadTree
{
	QuadTreeNode root;
	std::vector<QuadTreeNode*> leafs;
	unsigned int maxLvl;
	
	GLuint vao;
	GLuint vboVertex;
	GLuint vboEdgeDiv;
	
	QuadTree() : root(glm::vec3(0), glm::vec2(1.f), NULL, this)
	{
		maxLvl = 16;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}
	
	~QuadTree()
	{
		glDeleteVertexArrays(1, &vao);
	}
	
	void Grow(glm::vec3 eye, glm::mat4 VP)
	{
		root.Grow(eye, VP);
	}
};

//Rendering styles
typedef enum {SIMPLE, PHYSICAL, MIRROR, TRANSPARENT} LookType;

struct Look
{
    LookType type;
    glm::vec3 color;
    std::vector<GLfloat> params;
    std::vector<GLuint> textures;
};

//Class
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
	void BindBaseVertexArray();
	
	void DrawSAQ();
	void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, glm::vec4 color = glm::vec4(1.f));
	void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint textureArray, GLuint layer);
	void DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint textureMS, glm::ivec2 texSize);
	
	void DrawCubemapCross(GLuint texture);
	void DrawCoordSystem(glm::mat4 M, GLfloat size);
	void DrawPrimitives(PrimitiveType type, std::vector<glm::vec3>& vertices, glm::vec4 color, glm::mat4 M = glm::mat4());
	void DrawObject(int modelId, int lookId, const glm::mat4& M);
	
	//Allocate and build content
	void AddView(OpenGLView* view);
    void AddLight(OpenGLLight* light);
	unsigned int BuildObject(Mesh* mesh);
	unsigned int CreateSimpleLook(glm::vec3 rgbColor, GLfloat specular, GLfloat shininess, std::string textureName = "");
	unsigned int CreatePhysicalLook(glm::vec3 rgbColor, GLfloat roughness, GLfloat metallic = 0.f, std::string textureName = "");
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
	glm::vec3 eyePos;
	glm::vec3 viewDir;
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
	GLSLShader* texQuadMSShader;
	GLSLShader* texLayerQuadShader;
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
	static Mesh* LoadSTL(std::string filename, GLfloat scale, bool smooth);
	static Mesh* LoadOBJ(std::string filename, GLfloat scale, bool smooth);
	//TODO: Implement support for PLY
};

#endif