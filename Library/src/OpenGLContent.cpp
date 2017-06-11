//
//  OpenGLContent.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/06/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLContent.h"
#include "OpenGLGBuffer.h"
#include "Console.h"
#include "SystemUtil.hpp"
#include "stb_image.h"
#include "SimulationApp.h"
#include <map>
#include <algorithm>

#define clamp(x,min,max)     (x > max ? max : (x < min ? min : x))

OpenGLContent* OpenGLContent::instance = NULL;

OpenGLContent* OpenGLContent::getInstance()
{
    if(instance == NULL)
        instance = new OpenGLContent();
    
    return instance;
}

void OpenGLContent::Destroy()
{
	delete instance;
	instance = NULL;
}

OpenGLContent::OpenGLContent()
{
	saq = 0;
}

OpenGLContent::~OpenGLContent()
{
	if(saq != 0)
		glDeleteBuffers(1,&saq);
}

void OpenGLContent::DestroyContent()
{
	for(unsigned int i=0; i<looks.size();++i)
		if(looks[i].texture != 0)
			glDeleteTextures(1, &looks[i].texture);
	looks.clear();
			
	for(unsigned int i=0; i<objects.size();++i)
	{
		glDeleteBuffers(1, &objects[i].vboVertex);
		glDeleteBuffers(1, &objects[i].vboIndex);
		glDeleteVertexArrays(1, &objects[i].vao);
	}	
	objects.clear();
}

void OpenGLContent::Init()
{
	//Build screen-aligned quad VBO
	GLfloat saqData[4][4] = {{-1.f, -1.f, 0.f, 0.f},
							 { 1.f, -1.f, 1.f, 0.f},
							 {-1.f,  1.f, 0.f, 1.f},
							 { 1.f,  1.f, 1.f, 1.f}};
	
	glGenBuffers(1, &saq); 
	glBindBuffer(GL_ARRAY_BUFFER, saq); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(saqData), saqData, GL_STATIC_DRAW); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLContent::DrawSAQ()
{
	if(saq != 0)
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, saq); 
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), NULL);
 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(0);
	}
}

unsigned int OpenGLContent::BuildObject(Mesh* mesh)
{
	Object obj;
	obj.mesh = mesh;
	GLenum error = glGetError();
	
	glGenVertexArrays(1, &obj.vao);
	glGenBuffers(1, &obj.vboVertex);
	glGenBuffers(1, &obj.vboIndex);
	
	glBindVertexArray(obj.vao);	
	glBindBuffer(GL_ARRAY_BUFFER, obj.vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*mesh->vertices.size(), &mesh->vertices[0].pos.x, GL_STATIC_DRAW);
	/*glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)0);   //The starting point of the VBO, for the vertices
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)12);   //The starting point of normals, 12 bytes away
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)24);   //The starting point of texcoords, 24 bytes away
  */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)sizeof(glm::vec3));
	glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)*2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(8);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.vboIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face)*mesh->faces.size(), &mesh->faces[0].vertexID[0], GL_STATIC_DRAW);
	glBindVertexArray(0);
	
	error = glGetError();
	
	if(error != GL_NO_ERROR)
	{
		printf("OpenGL: %s\n", gluErrorString(error));
	}
	
	objects.push_back(obj);
	return objects.size()-1;
}

void OpenGLContent::DrawObject(int objectId, int lookId, btScalar* transform)
{
	if(objectId >= 0 && objectId < objects.size()) //Check if object exists
	{
		glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
		glMultMatrixd(transform);
#else
		glMultMatrixf(transform);
#endif
		if(lookId >= 0 && lookId < looks.size())
			UseLook(lookId);
		else
			UseStandardLook();
	
		glBindVertexArray(objects[objectId].vao);
		glDrawElements(GL_TRIANGLES, 3 * objects[objectId].mesh->faces.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glPopMatrix();
	}
}

unsigned int OpenGLContent::CreateOpaqueLook(glm::vec3 rgbColor, GLfloat diffuseReflectance, GLfloat roughness, GLfloat IOR, const char* textureName, GLfloat textureMixFactor)
{
    Look look;
    look.color = rgbColor;
    look.data[0] = diffuseReflectance;
    look.data[1] = roughness;
    look.data[2] = IOR;
    look.data[3] = 0.f; //No reflections
    
    if(textureName != NULL)
    {
        look.texture = LoadTexture(textureName);
        look.textureMix = textureMixFactor; //0 - 1 -> where 1 means only texture
    }
    else
    {
        look.texture = 0;
        look.textureMix = 0.f;
    }
    
    looks.push_back(look);
	return looks.size()-1;
}

void OpenGLContent::UseLook(unsigned int lookId)
{	
    //Diffuse reflectance, roughness, FO, reflection factor
    GLfloat diffuseReflectance = floor(looks[lookId].data[0] * 255.f);
    GLfloat roughness = floor(looks[lookId].data[1] * 255.f);
    GLfloat F0 = floor(powf((1.f - looks[lookId].data[2])/(1.f + looks[lookId].data[2]), 2.f) * 255.f);
    GLfloat reflection = floor(looks[lookId].data[3] * 255.f);
    GLfloat materialData = diffuseReflectance
                           + (roughness * 256)
                           + (F0 * 256 * 256)
                           + (reflection * 256 * 256 * 256);
    
    glBindTexture(GL_TEXTURE_2D, looks[lookId].texture);
    glColor4f(looks[lookId].color[0], looks[lookId].color[1], looks[lookId].color[2], looks[lookId].textureMix); //Color + Texture mix factor
    OpenGLGBuffer::SetUniformMaterialData(materialData);
}

void OpenGLContent::UseStandardLook()
{
	GLfloat diffuseReflectance = floor(0.5f * 255.f);
    GLfloat roughness = floor(0.2f * 255.f);
    GLfloat F0 = floor(powf((1.f - 0.1f)/(1.f + 0.1f), 2.f) * 255.f);
    GLfloat reflection = floor(0.f);
    GLfloat materialData = diffuseReflectance
                           + (roughness * 256)
                           + (F0 * 256 * 256)
                           + (reflection * 256 * 256 * 256);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glColor4f(0.5f, 0.5f, 0.5f, 0.f); //Color + Texture mix factor
    OpenGLGBuffer::SetUniformMaterialData(materialData);
}

void OpenGLContent::SetupOrtho()
{
    glMatrixMode(GL_PROJECTION);
    glm::mat4 proj = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    glLoadMatrixf(glm::value_ptr(proj));
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void OpenGLContent::DrawCoordSystem(GLfloat size)
{
    glBegin(GL_LINES);
    glXAxisColor();
    glVertex3f(0, 0, 0);
    glVertex3f(size, 0, 0);
    
    glYAxisColor();
    glVertex3f(0, 0, 0);
    glVertex3f(0, size, 0);
    
    glZAxisColor();
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, size);
    glEnd();
}

//Static methods
GLuint OpenGLContent::LoadTexture(const char* filename)
{
    int width, height, channels;
    GLuint texture;
    
    // Allocate image; fail out on error
    cInfo("Loading texture from: %s", filename);
    
    unsigned char* dataBuffer = stbi_load(filename, &width, &height, &channels, 3);
    if(dataBuffer == NULL)
    {
        cError("Failed to load texture!");
        return -1;
    }
    
    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    
    // Allocate an OpenGL texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Upload texture to memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);
    // Set certain properties of texture
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
    // Wrap texture around
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Release internal buffer
    stbi_image_free(dataBuffer);
    
    return texture;
}

GLuint OpenGLContent::LoadInternalTexture(const char* filename)
{
    char path[1024];
    GetDataPath(path, 1024-32);
    strcat(path, filename);
    return LoadTexture(path);
}

Mesh* OpenGLContent::BuildPlane(GLfloat halfExtents)
{
    int uDiv = (int)floor(2.f*halfExtents)/10;
    int vDiv = (int)floor(2.f*halfExtents)/10;
    if(uDiv < 1) uDiv = 1;
    if(vDiv < 1) vDiv = 1;
    bool zUp = SimulationApp::getApp()->getSimulationManager()->isZAxisUp();
    
	Mesh* mesh = new Mesh;
	mesh->hasUVs = true;
    Face f;
    Vertex vt;
	
	//Only one normal
	vt.normal = glm::vec3(0,0,(zUp ? 1.f : -1.f));
			
	for(unsigned int i=0; i<=vDiv; ++i)
    {
        vt.uv.y = (GLfloat)i/(GLfloat)vDiv*(GLfloat)(halfExtents*2.f);
		vt.pos.y = -halfExtents + vt.uv.y;
		
        for(unsigned int h=0; h<=uDiv; ++h)
        {
            vt.uv.x = (GLfloat)h/(GLfloat)uDiv*(GLfloat)(halfExtents*2.f);
            vt.pos.x = -halfExtents + vt.uv.x;
			mesh->vertices.push_back(vt);
        }
	}
	
	for(unsigned int i=0; i<vDiv; ++i)
	{
		for(unsigned int h=0; h<uDiv; ++h)
		{
			//Triangle 1
			f.vertexID[0] = i*(uDiv+1) + h;
			f.vertexID[1] = i*(uDiv+1) + h + 1;
			f.vertexID[2] = (i+1)*(uDiv+1) + h;
			mesh->faces.push_back(f);
			
			//Triangle 2
			f.vertexID[0] = i*(uDiv+1) + h + 1;
			f.vertexID[1] = (i+1)*(uDiv+1) + h + 1;
			f.vertexID[2] = (i+1)*(uDiv+1) + h;
			mesh->faces.push_back(f);
		}
	}
	
	return mesh;
}

Mesh* OpenGLContent::BuildBox(glm::vec3 halfExtents, unsigned int subdivisions)
{
    //Build mesh
    Mesh* mesh = new Mesh();
    Face f;
    Vertex vt;
	mesh->hasUVs = false;
	vt.uv = glm::vec2(0,0);
	
    /////VERTICES
	glm::vec3 v1(-halfExtents.x, -halfExtents.y, -halfExtents.z);
    glm::vec3 v2(-halfExtents.x, halfExtents.y, -halfExtents.z);
    glm::vec3 v3(halfExtents.x, halfExtents.y, -halfExtents.z);
    glm::vec3 v4(halfExtents.x, -halfExtents.y, -halfExtents.z);
    glm::vec3 v5(halfExtents.x, halfExtents.y, halfExtents.z);
    glm::vec3 v6(halfExtents.x, -halfExtents.y, halfExtents.z);
	glm::vec3 v7(-halfExtents.x, -halfExtents.y, halfExtents.z);
    glm::vec3 v8(-halfExtents.x, halfExtents.y, halfExtents.z);
    
    /////FRONT
    //normal
	vt.normal = glm::vec3(0,0,-1.f);
    //vertices
    vt.pos = v1;
	mesh->vertices.push_back(vt); //0
	vt.pos = v2;
	mesh->vertices.push_back(vt); //1
	vt.pos = v3;
	mesh->vertices.push_back(vt); //2
	vt.pos = v4;
	mesh->vertices.push_back(vt); //3
	//faces
	f.vertexID[0] = 0;
	f.vertexID[1] = 1;
	f.vertexID[2] = 2;
	mesh->faces.push_back(f);
	f.vertexID[1] = 2;
	f.vertexID[2] = 3;
    mesh->faces.push_back(f);
    
    /////LEFT
    //normal
	vt.normal =  glm::vec3(1.f,0,0);
    //vertices
	vt.pos = v4;
	mesh->vertices.push_back(vt); //4
	vt.pos = v3;
	mesh->vertices.push_back(vt); //5
	vt.pos = v5;
	mesh->vertices.push_back(vt); //6
	vt.pos = v6;
	mesh->vertices.push_back(vt); //7
	//faces
	f.vertexID[0] = 4;
	f.vertexID[1] = 5;
	f.vertexID[2] = 6;
	mesh->faces.push_back(f);
	f.vertexID[1] = 6;
	f.vertexID[2] = 7;
	mesh->faces.push_back(f);
	
    /////RIGHT
    //normal
	vt.normal = glm::vec3(-1.f,0,0);
	//vertices
	vt.pos = v7;
	mesh->vertices.push_back(vt); //8
	vt.pos = v8;
	mesh->vertices.push_back(vt); //9
	vt.pos = v2;
	mesh->vertices.push_back(vt); //10
	vt.pos = v1;
	mesh->vertices.push_back(vt); //11
	//faces
	f.vertexID[0] = 8;
	f.vertexID[1] = 9;
	f.vertexID[2] = 10;
	mesh->faces.push_back(f);
	f.vertexID[1] = 10;
	f.vertexID[2] = 11;
	mesh->faces.push_back(f);
	
    /////BACK
    //normal
    vt.normal = glm::vec3(0,0,1.f);
	//vertices
	vt.pos = v6;
	mesh->vertices.push_back(vt); //12
	vt.pos = v5;
	mesh->vertices.push_back(vt); //13
	vt.pos = v8;
	mesh->vertices.push_back(vt); //14
	vt.pos = v7;
	mesh->vertices.push_back(vt); //15
	//faces
	f.vertexID[0] = 12;
	f.vertexID[1] = 13;
	f.vertexID[2] = 14;
	mesh->faces.push_back(f);
	f.vertexID[1] = 14;
	f.vertexID[2] = 15;
	mesh->faces.push_back(f);
	
    //////TOP
    //normal
    vt.normal = glm::vec3(0,1.f,0);
	//vertices
	vt.pos = v5;
	mesh->vertices.push_back(vt); //16
	vt.pos = v3;
	mesh->vertices.push_back(vt); //17
	vt.pos = v2;
	mesh->vertices.push_back(vt); //18
	vt.pos = v8;
	mesh->vertices.push_back(vt); //19
	//faces
	f.vertexID[0] = 16;
	f.vertexID[1] = 17;
	f.vertexID[2] = 18;
	mesh->faces.push_back(f);
	f.vertexID[1] = 18;
	f.vertexID[2] = 19;
	mesh->faces.push_back(f);
	
    /////BOTTOM
    //normal
    vt.normal = glm::vec3(0,-1.f,0);
    //vertices
	vt.pos = v4;
	mesh->vertices.push_back(vt); //20
	vt.pos = v6;
	mesh->vertices.push_back(vt); //21
	vt.pos = v7;
	mesh->vertices.push_back(vt); //22
	vt.pos = v1;
	mesh->vertices.push_back(vt); //23
	//faces
	f.vertexID[0] = 20;
	f.vertexID[1] = 21;
	f.vertexID[2] = 22;
	mesh->faces.push_back(f);
	f.vertexID[1] = 22;
	f.vertexID[2] = 23;
	mesh->faces.push_back(f);
	
	for(unsigned int i=0; i<subdivisions; ++i)
		Subdivide(mesh);
		
	SmoothNormals(mesh);
	
	return mesh;
}

Mesh* OpenGLContent::BuildSphere(GLfloat radius, unsigned int subdivisions) 
{
	Mesh* mesh = new Mesh;
    Face f;
    Vertex vt;
	vt.normal = glm::vec3(1,0,0);
	mesh->hasUVs = false;
	vt.uv = glm::vec2(0,0);
    
    //Basuc icosahedron
	const GLfloat X = -.525731112119133606f;
	const GLfloat Z = -.850650808352039932f;
	const GLfloat N = 0.f;
	
	vt.pos = glm::vec3(-X,N,Z);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(X,N,Z);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(-X,N,-Z);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(X,N,-Z);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
	
    vt.pos = glm::vec3(N,Z,X);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(N,Z,-X);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(N,-Z,X);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(N,-Z,-X);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
	
	vt.pos = glm::vec3(Z,X,N);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(-Z,X,N);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(Z,-X,N);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(-Z,-X,N);
	vt.normal = vt.pos;
	mesh->vertices.push_back(vt);
    
	f.vertexID[0] = 0;
	f.vertexID[1] = 4;
	f.vertexID[2] = 1;
	mesh->faces.push_back(f);
	f.vertexID[0] = 0;
	f.vertexID[1] = 9;
	f.vertexID[2] = 4;
	mesh->faces.push_back(f);
	f.vertexID[0] = 9;
	f.vertexID[1] = 5;
	f.vertexID[2] = 4;
	mesh->faces.push_back(f);
	f.vertexID[0] = 4;
	f.vertexID[1] = 5;
	f.vertexID[2] = 8;
	mesh->faces.push_back(f);
	f.vertexID[0] = 4;
	f.vertexID[1] = 8;
	f.vertexID[2] = 1;
	mesh->faces.push_back(f);
	f.vertexID[0] = 8;
	f.vertexID[1] = 10;
	f.vertexID[2] = 1;
	mesh->faces.push_back(f);
	f.vertexID[0] = 8;
	f.vertexID[1] = 3;
	f.vertexID[2] = 10;
	mesh->faces.push_back(f);
	f.vertexID[0] = 5;
	f.vertexID[1] = 3;
	f.vertexID[2] = 8;
	mesh->faces.push_back(f);
	f.vertexID[0] = 5;
	f.vertexID[1] = 2;
	f.vertexID[2] = 3;
	mesh->faces.push_back(f);
	f.vertexID[0] = 2;
	f.vertexID[1] = 7;
	f.vertexID[2] = 3;
	mesh->faces.push_back(f);
	f.vertexID[0] = 7;
	f.vertexID[1] = 10;
	f.vertexID[2] = 3;
	mesh->faces.push_back(f);
	f.vertexID[0] = 7;
	f.vertexID[1] = 6;
	f.vertexID[2] = 10;
	mesh->faces.push_back(f);
	f.vertexID[0] = 7;
	f.vertexID[1] = 11;
	f.vertexID[2] = 6;
	mesh->faces.push_back(f);
	f.vertexID[0] = 11;
	f.vertexID[1] = 0;
	f.vertexID[2] = 6;
	mesh->faces.push_back(f);
	f.vertexID[0] = 0;
	f.vertexID[1] = 1;
	f.vertexID[2] = 6;
	mesh->faces.push_back(f);
	f.vertexID[0] = 6;
	f.vertexID[1] = 1;
	f.vertexID[2] = 10;
	mesh->faces.push_back(f);
	f.vertexID[0] = 9;
	f.vertexID[1] = 0;
	f.vertexID[2] = 11;
	mesh->faces.push_back(f);
	f.vertexID[0] = 9;
	f.vertexID[1] = 11;
	f.vertexID[2] = 2;
	mesh->faces.push_back(f);
	f.vertexID[0] = 9;
	f.vertexID[1] = 2;
	f.vertexID[2] = 5;
	mesh->faces.push_back(f);
	f.vertexID[0] = 7;
	f.vertexID[1] = 2;
	f.vertexID[2] = 11;
	mesh->faces.push_back(f);
	
	//Subdivide to get smooth sphere
	for(unsigned int i=0; i<subdivisions; ++i)
		Subdivide(mesh, true);
		
	//Scale by radius
	for(unsigned int i=0; i<mesh->vertices.size(); ++i)
		mesh->vertices[i].pos *= radius;
	
	return mesh;
}

Mesh* OpenGLContent::BuildCylinder(GLfloat radius, GLfloat height, unsigned int slices)
{
    Mesh* mesh = new Mesh;
    GLfloat halfHeight = height/2.f;
	Vertex vt;
	Face f;
	
	mesh->hasUVs = false;
	vt.uv = glm::vec2(0,0);
	
	//SIDE
    //Side vertices
    for(unsigned int i=0; i<slices; ++i)
    {
		vt.normal = glm::vec3(sin(i/(GLfloat)slices*M_PI*2.0), 0.0, cos(i/(GLfloat)slices*M_PI*2.0));
		vt.pos = glm::vec3(sin(i/(GLfloat)slices*M_PI*2.0)*radius, halfHeight, cos(i/(GLfloat)slices*M_PI*2.0)*radius);
        mesh->vertices.push_back(vt);
		vt.pos.y = -halfHeight;
        mesh->vertices.push_back(vt);
    }
    
    //Side faces
    for(unsigned int i=0; i<slices-1; ++i)
    {
        f.vertexID[0] = i*2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+2;
        mesh->faces.push_back(f);
        f.vertexID[0] = i*2+1;
        f.vertexID[1] = i*2+3;
        f.vertexID[2] = i*2+2;
        mesh->faces.push_back(f);
    }
    
	//Last side
    int i = slices-1;
	vt.normal = glm::vec3(sin((GLfloat)(slices-1)/(GLfloat)slices*M_PI*2.0), 0.0, cos((GLfloat)(slices-1)/(GLfloat)slices*M_PI*2.0));
    f.vertexID[0] = i*2;
    f.vertexID[1] = i*2+1;
    f.vertexID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = i*2+1;
    f.vertexID[1] = 1;
    f.vertexID[2] = 0;
    mesh->faces.push_back(f);
    
    //TOP CAP
    vt.normal = glm::vec3(0,1.f,0);
	vt.pos = glm::vec3(0,halfHeight,0);
	mesh->vertices.push_back(vt);
	unsigned int centerIndex = mesh->vertices.size()-1;
	//vertices
	for(unsigned int i=0; i<slices; ++i)
    {
		vt.pos = glm::vec3(sin(i/(GLfloat)slices*M_PI*2.0)*radius, halfHeight, cos(i/(GLfloat)slices*M_PI*2.0)*radius);
        mesh->vertices.push_back(vt);
    }
	//faces
	for(unsigned int i=0; i<slices-1; ++i)
	{
		f.vertexID[0] = centerIndex;
		f.vertexID[1] = centerIndex + i + 1;
		f.vertexID[2] = centerIndex + i + 2;
		mesh->faces.push_back(f);
	}
	//last face
	f.vertexID[0] = centerIndex;
	f.vertexID[1] = centerIndex + slices-1 + 1;
	f.vertexID[2] = centerIndex + 1;
	mesh->faces.push_back(f);
	
	//BOTTOM CAP
    vt.normal = glm::vec3(0,-1.f,0);
	vt.pos = glm::vec3(0,-halfHeight,0);
	mesh->vertices.push_back(vt);
	centerIndex = mesh->vertices.size()-1;
	//vertices
	for(unsigned int i=0; i<slices; ++i)
    {
		vt.pos = glm::vec3(sin(i/(GLfloat)slices*M_PI*2.0)*radius, -halfHeight, cos(i/(GLfloat)slices*M_PI*2.0)*radius);
        mesh->vertices.push_back(vt);
    }
	//faces
	for(unsigned int i=0; i<slices-1; ++i)
	{
		f.vertexID[0] = centerIndex;
		f.vertexID[1] = centerIndex+ i + 2;
		f.vertexID[2] = centerIndex+ i + 1;
		mesh->faces.push_back(f);
	}
	//last face
	f.vertexID[0] = centerIndex;
	f.vertexID[1] = centerIndex + 1;
	f.vertexID[2] = centerIndex + slices-1 + 1;
	mesh->faces.push_back(f);
	
	Subdivide(mesh);
	Subdivide(mesh);
	return mesh;
}

Mesh* OpenGLContent::BuildTorus(GLfloat majorRadius, GLfloat minorRadius, unsigned int majorSlices, unsigned int minorSlices)
{
	Mesh* mesh = new Mesh;
	mesh->hasUVs = true;
	Face f;
	Vertex vt;

	for(unsigned int i=0; i<majorSlices; ++i)
    {
        GLfloat alpha = i/(GLfloat)majorSlices*M_PI*2.f;
        
        for(unsigned int h=0; h<minorSlices; ++h)
        {
            GLfloat ry = cosf(h/(GLfloat)minorSlices*M_PI*2.f) * minorRadius;
            GLfloat rx = sinf(h/(GLfloat)minorSlices*M_PI*2.f) * minorRadius;
            
			vt.pos = glm::vec3((rx + majorRadius)*cosf(alpha), ry, (rx + majorRadius)*sinf(alpha));
			vt.normal = glm::vec3(sinf(h/(GLfloat)minorSlices*M_PI*2.f)*cosf(alpha), cosf(h/(GLfloat)minorSlices*M_PI*2.f), sinf(h/(GLfloat)minorSlices*M_PI*2.f)*sinf(alpha));
			vt.uv = glm::vec2(4.f * h/(GLfloat)minorSlices, (GLfloat)minorSlices * i/(GLfloat)majorSlices);
			mesh->vertices.push_back(vt);
			
			if(h<minorSlices-1 && i<majorSlices-1)
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + h;
				f.vertexID[2] = i*minorSlices + (h+1);
				mesh->faces.push_back(f);
				f.vertexID[0] = (i+1)*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + (h+1);
				f.vertexID[2] = i*minorSlices + (h+1);
				mesh->faces.push_back(f);
			}
			else if(i<majorSlices-1) //Last patch in the middle slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + h;
				f.vertexID[2] = i*minorSlices;
				mesh->faces.push_back(f);
				f.vertexID[0] = (i+1)*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices;
				f.vertexID[2] = i*minorSlices;
				mesh->faces.push_back(f);
			}
			else if(h<minorSlices-1) //Middle patch in the last slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = h;
				f.vertexID[2] = i*minorSlices + (h+1);
				mesh->faces.push_back(f);
				f.vertexID[0] = h;
				f.vertexID[1] = h+1;
				f.vertexID[2] = i*minorSlices + (h+1);
				mesh->faces.push_back(f);
			}
			else //Last patch in the last slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = h;
				f.vertexID[2] = i*minorSlices;
				mesh->faces.push_back(f);
				f.vertexID[0] = h;
				f.vertexID[1] = 0;
				f.vertexID[2] = i*minorSlices;
				mesh->faces.push_back(f);
			}
	    }
    }
	
	return mesh;
}

Mesh* OpenGLContent::LoadMesh(const char* filename, btScalar scale, bool smooth)
{
    std::string extension = std::string(filename).substr(strlen(filename)-3,3);
    
    if(extension == "stl" || extension == "STL")
    {
        return LoadSTL(filename, scale, smooth);
    }
    else if(extension == "obj" || extension == "OBJ")
    {
        return LoadOBJ(filename, scale, smooth);
    }
    else
    {
        cError("Unsupported model file type: %s!", extension.c_str());
        return NULL;
    }
}

Mesh* OpenGLContent::LoadOBJ(const char *filename, btScalar scale, bool smooth)
{
    //Read OBJ data
    cInfo("Loading model from: %s", filename);
    
	FILE* file = fopen(filename, "rb");
    char line[256];
    char c1, c2;
    Mesh* mesh = new Mesh();
    bool hasNormals = false, hasUvs = false;
	
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
    
    //Read file
    while(fgets(line, 256, file))
    {
        sscanf(line, "%c%c", &c1, &c2);
        if(c1 == 'v')
        {
            if(c2 == ' ')
            {
                glm::vec3 v;
                sscanf(line, "v %f %f %f\n", &v.x, &v.y, &v.z);
                v *= scale;
                positions.push_back(v);
            }
            else if(c2 == 'n')
            {
                glm::vec3 n;
                sscanf(line, "vn %f %f %f\n", &n.x, &n.y, &n.z);
                normals.push_back(n);
                hasNormals = true;
            }
            else if(c2 == 't')
            {
                glm::vec2 uv;
                sscanf(line, "vt %f %f\n", &uv.x, &uv.y);
                uvs.push_back(uv);
                hasUvs = true;
            }
        }
        else if(c1 == 'f')
        {
            Face face;
			Vertex v;
				
            if(hasNormals && hasUvs)
            {
				unsigned int vID[3];
				unsigned int uvID[3];
				unsigned int nID[3];
				sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", &vID[0], &uvID[0], &nID[0], &vID[1], &uvID[1], &nID[1], &vID[2], &uvID[2], &nID[2]);
                
				for(short unsigned int i=0; i<3; ++i)
				{
					v.pos = positions[vID[i]-1];
					v.normal = normals[nID[i]-1];
					v.uv = uvs[uvID[i]-1];
					
					std::vector<Vertex>::iterator it;
					if((it = std::find(mesh->vertices.begin(), mesh->vertices.end(), v)) != mesh->vertices.end()) //If vertex exists
					{
						face.vertexID[i] = it-mesh->vertices.begin();
					}
					else //Create new vertex
					{
						mesh->vertices.push_back(v);
						face.vertexID[i] = mesh->vertices.size()-1;
					}
				}
            }
            else if(hasNormals)
            {
				unsigned int vID[3];
				unsigned int nID[3];
				v.uv = glm::vec2(0,0);
                sscanf(line, "f %u//%u %u//%u %u//%u\n", &vID[0], &nID[0], &vID[1], &nID[1], &vID[2], &nID[2]);
                
				for(short unsigned int i=0; i<3; ++i)
				{
					v.pos = positions[vID[i]-1];
					v.normal = normals[nID[i]-1];
					
					std::vector<Vertex>::iterator it;
					if((it = std::find(mesh->vertices.begin(), mesh->vertices.end(), v)) != mesh->vertices.end()) //If vertex exists
					{
						face.vertexID[i] = it-mesh->vertices.begin();
					}
					else //Create new vertex
					{
						mesh->vertices.push_back(v);
						face.vertexID[i] = mesh->vertices.size()-1;
					}
				}
            }
            else
            {
				unsigned int vID[3];
				v.normal = glm::vec3(1,0,0);
				v.uv = glm::vec2(0,0);
                sscanf(line, "f %u %u %u\n", &vID[0], &vID[1], &vID[2]);
                
				for(short unsigned int i=0; i<3; ++i)
				{
					v.pos = positions[vID[i]-1];
					
					std::vector<Vertex>::iterator it;
					if((it = std::find(mesh->vertices.begin(), mesh->vertices.end(), v)) != mesh->vertices.end()) //If vertex exists
					{
						face.vertexID[i] = it-mesh->vertices.begin();
					}
					else //Create new vertex
					{
						mesh->vertices.push_back(v);
						face.vertexID[i] = mesh->vertices.size()-1;
					}
				}
            }
            
            mesh->faces.push_back(face);
        }
    }
    
    fclose(file);
	
	mesh->hasUVs = hasUvs;
   
    if(smooth)
		SmoothNormals(mesh);
    
    return mesh;
}

Mesh* OpenGLContent::LoadSTL(const char *filename, btScalar scale, bool smooth)
{
    //Read STL data
    cInfo("Loading model from: %s", filename);
    
	FILE* file = fopen(filename, "rb");
    char line[256];
    char keyword[10];
    Mesh *mesh = new Mesh();
	mesh->hasUVs = false;
    
	Vertex vt;
	vt.uv = glm::vec2(0,0);
	
    while(fgets(line, 256, file))
    {
        sscanf(line, "%s", keyword);
        
        if(strcmp(keyword, "facet")==0)
        {
            sscanf(line, " facet normal %f %f %f\n", &vt.normal.x, &vt.normal.y, &vt.normal.z);
        }
        else if(strcmp(keyword, "vertex")==0)
        {
			sscanf(line, " vertex %f %f %f\n", &vt.pos.x, &vt.pos.y, &vt.pos.z);
			vt.pos *= scale;
            mesh->vertices.push_back(vt);
        }
        else if(strcmp(keyword, "endfacet")==0)
        {
            unsigned int lastVertexID = mesh->vertices.size()-1;
            
			Face f;
            f.vertexID[0] = lastVertexID-2;
            f.vertexID[1] = lastVertexID-1;
            f.vertexID[2] = lastVertexID;
            mesh->faces.push_back(f);
        }
    }
	
    fclose(file);
    
    //Remove duplicates (so that it becomes equivalent to OBJ file representation)
	
	if(smooth)
		SmoothNormals(mesh);
    
    return mesh;
}

void OpenGLContent::SmoothNormals(Mesh* mesh)
{
	//For all faces
	for(unsigned int i=0; i<mesh->faces.size(); ++i)
	{
		Face thisFace = mesh->faces[i];
		glm::vec3 thisN = mesh->computeFaceNormal(i);
						
		//For every vertex
		for(unsigned short int h=0; h<3; ++h)
		{
			glm::vec3 n = thisN;
			unsigned int contrib = 1;
                
			//Loop through all faces
			for(unsigned int j=0; j<mesh->faces.size(); ++j)
			{
				if(j != i) //Reject same face
				{
					Face thatFace = mesh->faces[j];
					glm::vec3 thatN = mesh->computeFaceNormal(j);
                        
					for(unsigned short int k=0; k<3; k++)
					{	
						if((mesh->vertices[thatFace.vertexID[k]].pos == mesh->vertices[thisFace.vertexID[h]].pos)&&(glm::dot(thatN,thisN) > 0.707f))
						{
							n += thatN;
							contrib++;
							break;
						}
					}
				}
			}
                
			n /= (GLfloat)contrib;
			mesh->vertices[mesh->faces[i].vertexID[h]].normal = n;
		}
	}
}

GLuint vertex4EdgeIco(std::map<std::pair<GLuint, GLuint>, GLuint>& lookup, Mesh* mesh, GLuint firstID, GLuint secondID)
{
	std::map<std::pair<GLuint, GLuint>, GLuint>::key_type key(firstID, secondID);
	if(key.first > key.second)
		std::swap(key.first, key.second);
		
	auto inserted=lookup.insert({key, mesh->vertices.size()});
	if(inserted.second)
	{
		glm::vec3 edge0 = mesh->vertices[firstID].pos;
		glm::vec3 edge1 = mesh->vertices[secondID].pos;
		Vertex vt;
		vt.pos =  glm::normalize(edge0 + edge1);
		vt.normal = vt.pos;
		vt.uv = glm::vec2(0,0);
		mesh->vertices.push_back(vt);
	}
	
	return inserted.first->second;
}

GLuint vertex4Edge(std::map<std::pair<GLuint, GLuint>, GLuint>& lookup, Mesh* mesh, GLuint firstID, GLuint secondID)
{
	std::map<std::pair<GLuint, GLuint>, GLuint>::key_type key(firstID, secondID);
	if(key.first > key.second)
		std::swap(key.first, key.second);
		
	auto inserted=lookup.insert({key, mesh->vertices.size()});
	if(inserted.second)
	{
		glm::vec3 edge0 = mesh->vertices[firstID].pos;
		glm::vec3 edge1 = mesh->vertices[secondID].pos;
		glm::vec3 edge0n = mesh->vertices[firstID].normal;
		glm::vec3 edge1n = mesh->vertices[secondID].normal;
		glm::vec2 edge0uv = mesh->vertices[firstID].uv;
		glm::vec2 edge1uv = mesh->vertices[secondID].uv;
		
		Vertex vt;
		vt.pos = (edge0 + edge1)/(GLfloat)2;
		vt.normal = glm::normalize(edge0n + edge1n);
		vt.uv = (edge0uv + edge1uv)/(GLfloat)2;
		mesh->vertices.push_back(vt);
	}
	
	return inserted.first->second;
}

void OpenGLContent::Subdivide(Mesh* mesh, bool icoMode)
{
	std::map<std::pair<GLuint, GLuint>, GLuint> lookup;
	std::vector<Face> newFaces;
	
	for(GLuint i=0; i<mesh->faces.size(); ++i)
	{
		GLuint mid[3];
		
		if(icoMode)
		{
			for(int edge = 0; edge<3; ++edge)
				mid[edge] = vertex4EdgeIco(lookup, mesh, mesh->faces[i].vertexID[edge], mesh->faces[i].vertexID[(edge+1)%3]);
		}
		else
		{
			for(int edge = 0; edge<3; ++edge)
				mid[edge] = vertex4Edge(lookup, mesh, mesh->faces[i].vertexID[edge], mesh->faces[i].vertexID[(edge+1)%3]);
		}
			
		Face f;
		f.vertexID[0] = mesh->faces[i].vertexID[0];
		f.vertexID[1] = mid[0];
		f.vertexID[2] = mid[2];
		newFaces.push_back(f);
		
		f.vertexID[0] = mesh->faces[i].vertexID[1];
		f.vertexID[1] = mid[1];
		f.vertexID[2] = mid[0];
		newFaces.push_back(f);
		
		f.vertexID[0] = mesh->faces[i].vertexID[2];
		f.vertexID[1] = mid[2];
		f.vertexID[2] = mid[1];
		newFaces.push_back(f);
		
		f.vertexID[0] = mid[0];
		f.vertexID[1] = mid[1];
		f.vertexID[2] = mid[2];
		newFaces.push_back(f);
	}
	
	mesh->faces = newFaces;
}

void OpenGLContent::AABB(Mesh* mesh, btVector3& min, btVector3& max)
{
    btScalar minX=BT_LARGE_FLOAT, maxX=-BT_LARGE_FLOAT;
    btScalar minY=BT_LARGE_FLOAT, maxY=-BT_LARGE_FLOAT;
    btScalar minZ=BT_LARGE_FLOAT, maxZ=-BT_LARGE_FLOAT;
    
    for(int i=0; i<mesh->vertices.size(); i++)
    {
        glm::vec3 vertex = mesh->vertices[i].pos;
        
        if(vertex.x > maxX)
            maxX = vertex.x;
        
        if(vertex.x < minX)
            minX = vertex.x;
        
        if(vertex.y > maxY)
            maxY = vertex.y;
        
        if(vertex.y < minY)
            minY = vertex.y;
        
        if(vertex.z >maxZ)
            maxZ = vertex.z;
        
        if(vertex.z < minZ)
            minZ = vertex.z;
    }
    
    min = btVector3(minX, minY, minZ);
    max = btVector3(maxX, maxY, maxZ);
}

void OpenGLContent::AABS(Mesh* mesh, btScalar& bsRadius, btVector3& bsCenterOffset)
{
    glm::vec3 tempCenter(0,0,0);
    
    for(unsigned int i=0; i<mesh->vertices.size(); ++i)
        tempCenter += mesh->vertices[i].pos;
    
    tempCenter /= (GLfloat)mesh->vertices.size();
    
    GLfloat radius = 0;
    
    for(unsigned int i=0; i<mesh->vertices.size(); ++i)
    {
        glm::vec3 v = mesh->vertices[i].pos;
        GLfloat r = glm::length(v-tempCenter);
        if(r > radius)
            radius = r;
    }
    
    bsRadius = (btScalar)radius;
    bsCenterOffset.setX((btScalar)tempCenter.x);
    bsCenterOffset.setY((btScalar)tempCenter.y);
    bsCenterOffset.setZ((btScalar)tempCenter.z);
}