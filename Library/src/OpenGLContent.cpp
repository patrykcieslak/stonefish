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
#include "SystemUtil.h"
#include "stb_image.h"
#include "SimulationApp.h"
#include <map>

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
}

OpenGLContent::~OpenGLContent()
{
	for(int i=0; i<looks.size();++i)
		if(looks[i].texture != 0)
			glDeleteTextures(1, &looks[i].texture);
	looks.clear();
			
	for(int i=0; i<objects.size();++i)
		//glDeleteBuffers(1, &objects[i]);
		glDeleteLists(objects[i], 1);
	objects.clear();
}

unsigned int OpenGLContent::BuildObject(Mesh* mesh)
{
	GLuint list = glGenLists(1);
	glNewList(list, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(int h=0; h<mesh->faces.size(); h++)
    {
        if(mesh->uvs.size() > 0)
        {
			glNormal3fv(&(mesh->normals[mesh->faces[h].normalID[0]])[0]);
            glTexCoord2fv(&(mesh->uvs[mesh->faces[h].uvID[0]])[0]);
			glVertex3fv(&(mesh->vertices[mesh->faces[h].vertexID[0]])[0]);
            
			glNormal3fv(&(mesh->normals[mesh->faces[h].normalID[1]])[0]);
            glTexCoord2fv(&(mesh->uvs[mesh->faces[h].uvID[1]])[0]);
			glVertex3fv(&(mesh->vertices[mesh->faces[h].vertexID[1]])[0]);
          
			glNormal3fv(&(mesh->normals[mesh->faces[h].normalID[2]])[0]);
            glTexCoord2fv(&(mesh->uvs[mesh->faces[h].uvID[2]])[0]);
			glVertex3fv(&(mesh->vertices[mesh->faces[h].vertexID[2]])[0]);
        }
        else
        {
            glNormal3fv(&(mesh->normals[mesh->faces[h].normalID[0]])[0]);
			glVertex3fv(&(mesh->vertices[mesh->faces[h].vertexID[0]])[0]);
            
			glNormal3fv(&(mesh->normals[mesh->faces[h].normalID[1]])[0]);
			glVertex3fv(&(mesh->vertices[mesh->faces[h].vertexID[1]])[0]);
          
			glNormal3fv(&(mesh->normals[mesh->faces[h].normalID[2]])[0]);
			glVertex3fv(&(mesh->vertices[mesh->faces[h].vertexID[2]])[0]);
        }
    }
    glEnd();
    glEndList();
	
	objects.push_back(list);
	
	return objects.size()-1;
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
		
		glCallList(objects[objectId]);
		glPopMatrix();
	}
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
    Face f;
    glm::vec3 v;
	glm::vec2 uv;
    
	//Only one normal
	mesh->normals.push_back(glm::vec3(0,0,(zUp ? 1.f : -1.f)));
	f.normalID[0] = f.normalID[1] = f.normalID[2] = 0;
			
	for(unsigned int i=0; i<=vDiv; ++i)
    {
        uv.y = (GLfloat)i/(GLfloat)vDiv*(GLfloat)(halfExtents*2.f);
		v.y = -halfExtents + uv.y;
		
        for(unsigned int h=0; h<=uDiv; ++h)
        {
            uv.x = (GLfloat)h/(GLfloat)uDiv*(GLfloat)(halfExtents*2.f);
            v.x = -halfExtents + uv.x;
			
			mesh->vertices.push_back(v);
			mesh->uvs.push_back(uv);
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
			f.uvID[0] = f.vertexID[0];
			f.uvID[1] = f.vertexID[1];
			f.uvID[2] = f.vertexID[2];
			mesh->faces.push_back(f);
			
			//Triangle 2
			f.vertexID[0] = i*(uDiv+1) + h + 1;
			f.vertexID[1] = (i+1)*(uDiv+1) + h + 1;
			f.vertexID[2] = (i+1)*(uDiv+1) + h;
			f.uvID[0] = f.vertexID[0];
			f.uvID[1] = f.vertexID[1];
			f.uvID[2] = f.vertexID[2];
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
    glm::vec3 v;
	glm::vec3 n;
    
    /////VERTICES
	v = glm::vec3(-halfExtents.x, -halfExtents.y, -halfExtents.z);
    mesh->vertices.push_back(v);
    v = glm::vec3(-halfExtents.x, halfExtents.y, -halfExtents.z);
    mesh->vertices.push_back(v);
    v = glm::vec3(halfExtents.x, halfExtents.y, -halfExtents.z);
    mesh->vertices.push_back(v);
    v = glm::vec3(halfExtents.x, -halfExtents.y, -halfExtents.z);
    mesh->vertices.push_back(v);
    v = glm::vec3(halfExtents.x, halfExtents.y, halfExtents.z);
    mesh->vertices.push_back(v);
    v = glm::vec3(halfExtents.x, -halfExtents.y, halfExtents.z);
    mesh->vertices.push_back(v);
    v = glm::vec3(-halfExtents.x, -halfExtents.y, halfExtents.z);
    mesh->vertices.push_back(v);
    v = glm::vec3(-halfExtents.x, halfExtents.y, halfExtents.z);
    mesh->vertices.push_back(v);
    
    /////FRONT
    //normal
    mesh->normals.push_back(glm::vec3(0,0,-1.f));
    //faces
    f.vertexID[0] = 0;
    f.vertexID[1] = 1;
    f.vertexID[2] = 2;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = 0;
    f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = 0;
    f.vertexID[1] = 2;
    f.vertexID[2] = 3;
    mesh->faces.push_back(f);
    
    /////LEFT
    //normal
    mesh->normals.push_back(glm::vec3(1.f,0,0));
    //faces
    f.vertexID[0] = 3;
    f.vertexID[1] = 2;
    f.vertexID[2] = 4;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = 1;
    f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = 3;
    f.vertexID[1] = 4;
    f.vertexID[2] = 5;
    mesh->faces.push_back(f);
    
    /////RIGHT
    //normal
    mesh->normals.push_back(glm::vec3(-1.f,0,0));
    //faces
    f.vertexID[0] = 6;
    f.vertexID[1] = 7;
    f.vertexID[2] = 1;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = 2;
    f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = 6;
    f.vertexID[1] = 1;
    f.vertexID[2] = 0;
    mesh->faces.push_back(f);
    
    /////BACK
    //normal
    mesh->normals.push_back(glm::vec3(0,0,1.f));
    //faces
    f.vertexID[0] = 5;
    f.vertexID[1] = 4;
    f.vertexID[2] = 7;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = 3;
    f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = 5;
    f.vertexID[1] = 7;
    f.vertexID[2] = 6;
    mesh->faces.push_back(f);
    
    //////TOP
    //normal
    mesh->normals.push_back(glm::vec3(0,1.f,0));
    //faces
    f.vertexID[0] = 4;
    f.vertexID[1] = 2;
    f.vertexID[2] = 1;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = 4;
    f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = 4;
    f.vertexID[1] = 1;
    f.vertexID[2] = 7;
    mesh->faces.push_back(f);
    
    /////BOTTOM
    //normal
    mesh->normals.push_back(glm::vec3(0,-1.f,0));
    //faces
    f.vertexID[0] = 3;
    f.vertexID[1] = 5;
    f.vertexID[2] = 6;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = 5;
    f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = 3;
    f.vertexID[1] = 6;
    f.vertexID[2] = 0;
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
    glm::vec3 v;
    
    //Basuc icosahedron
	const GLfloat X = -.525731112119133606f;
	const GLfloat Z = -.850650808352039932f;
	const GLfloat N = 0.f;
	
	v = glm::vec3(-X,N,Z);
	mesh->vertices.push_back(v);
    v = glm::vec3(X,N,Z);
	mesh->vertices.push_back(v);
    v = glm::vec3(-X,N,-Z);
	mesh->vertices.push_back(v);
    v = glm::vec3(X,N,-Z);
	mesh->vertices.push_back(v);
	
    v = glm::vec3(N,Z,X);
	mesh->vertices.push_back(v);
    v = glm::vec3(N,Z,-X);
	mesh->vertices.push_back(v);
    v = glm::vec3(N,-Z,X);
	mesh->vertices.push_back(v);
    v = glm::vec3(N,-Z,-X);
	mesh->vertices.push_back(v);
	
	v = glm::vec3(Z,X,N);
	mesh->vertices.push_back(v);
    v = glm::vec3(-Z,X,N);
	mesh->vertices.push_back(v);
    v = glm::vec3(Z,-X,N);
	mesh->vertices.push_back(v);
    v = glm::vec3(-Z,-X,N);
	mesh->vertices.push_back(v);
    
	f.normalID[0] = f.normalID[1] = f.normalID[2] = 0;
	f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
	
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
		mesh->vertices[i] *= radius;
	
	//Generate smooth normals
	SmoothNormals(mesh);
	
	return mesh;
}

Mesh* OpenGLContent::BuildCylinder(GLfloat radius, GLfloat height, unsigned int slices)
{
    Mesh* mesh = new Mesh;
    GLfloat halfHeight = height/2.f;
	
    //vertices
    for(unsigned int i=0; i<slices; ++i)
    {
        glm::vec3 v(sin(i/(GLfloat)slices*M_PI*2.0)*radius, halfHeight, cos(i/(GLfloat)slices*M_PI*2.0)*radius);
        mesh->vertices.push_back(v);
		v.y = -halfHeight;
        mesh->vertices.push_back(v);
		
		glm::vec3 n(sin(i/(GLfloat)slices*M_PI*2.0), 0.0, cos(i/(GLfloat)slices*M_PI*2.0));
        mesh->normals.push_back(n);
    }
    
    //faces
    //SIDE
    for(unsigned int i=0; i<slices-1; ++i)
    {
        Face f;
        f.vertexID[0] = i*2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+2;
        f.normalID[0] = f.vertexID[0]/2;
		f.normalID[1] = f.vertexID[1]/2;
		f.normalID[2] = f.vertexID[2]/2;
        f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
        mesh->faces.push_back(f);
        f.vertexID[0] = i*2+1;
        f.vertexID[1] = i*2+3;
        f.vertexID[2] = i*2+2;
        f.normalID[0] = f.vertexID[0]/2;
		f.normalID[1] = f.vertexID[1]/2;
		f.normalID[2] = f.vertexID[2]/2;
        mesh->faces.push_back(f);
    }
    
    glm::vec3 n(sin((GLfloat)(slices-1)/(GLfloat)slices*M_PI*2.0), 0.0, cos((GLfloat)(slices-1)/(GLfloat)slices*M_PI*2.0));
    mesh->normals.push_back(n);
    
    Face f;
    int i = slices-1;
    f.vertexID[0] = i*2;
    f.vertexID[1] = i*2+1;
    f.vertexID[2] = 0;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = i;
    f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = i*2+1;
    f.vertexID[1] = 1;
    f.vertexID[2] = 0;
    mesh->faces.push_back(f);
    
    //CAPS
    n = glm::vec3(0,1.f,0);
    mesh->normals.push_back(n);
    n = glm::vec3(0,-1.f,0);
    mesh->normals.push_back(n);
    int n1index = mesh->normals.size()-2;
    int n2index = n1index+1;
	
    glm::vec3 v(0, halfHeight, 0);
    mesh->vertices.push_back(v);
    mesh->vertices.push_back(-v);
    int v1index = mesh->vertices.size()-2;
    int v2index = v1index+1;
    
    for(unsigned int i=0; i<slices-1; ++i)
    {
        Face f;
        f.vertexID[0] = i*2;
        f.vertexID[1] = (i+1)*2;
        f.vertexID[2] = v1index;
        f.normalID[0] = f.normalID[1] = f.normalID[2] = n1index;
        f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
        mesh->faces.push_back(f);
        f.vertexID[0] = (i+1)*2+1;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = v2index;
        f.normalID[0] = f.normalID[1] = f.normalID[2] = n2index;
        mesh->faces.push_back(f);
    }
    
    i = slices-1;
    f.vertexID[0] = i*2;
    f.vertexID[1] = 0;
    f.vertexID[2] = v1index;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = n1index;
	f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = 1;
    f.vertexID[1] = i*2+1;
    f.vertexID[2] = v2index;
    f.normalID[0] = f.normalID[1] = f.normalID[2] = n2index;
    mesh->faces.push_back(f);
	
	Subdivide(mesh);
	Subdivide(mesh);
	SmoothNormals(mesh);
    	
	return mesh;
}

Mesh* OpenGLContent::BuildTorus(GLfloat majorRadius, GLfloat minorRadius, unsigned int majorSlices, unsigned int minorSlices)
{
	Mesh* mesh = new Mesh;
	Face f;

	for(unsigned int i=0; i<majorSlices; ++i)
    {
        GLfloat alpha = i/(GLfloat)majorSlices*M_PI*2.f;
        
        for(unsigned int h=0; h<minorSlices; ++h)
        {
            GLfloat ry = cosf(h/(GLfloat)minorSlices*M_PI*2.f) * minorRadius;
            GLfloat rx = sinf(h/(GLfloat)minorSlices*M_PI*2.f) * minorRadius;
            
			glm::vec3 v((rx + majorRadius)*cosf(alpha), ry, (rx + majorRadius)*sinf(alpha));
			glm::vec3 n(sinf(h/(GLfloat)minorSlices*M_PI*2.f)*cosf(alpha), cosf(h/(GLfloat)minorSlices*M_PI*2.f), sinf(h/(GLfloat)minorSlices*M_PI*2.f)*sinf(alpha));
			glm::vec2 uv(4.f * h/(GLfloat)minorSlices, (GLfloat)minorSlices * i/(GLfloat)majorSlices);
			
			mesh->vertices.push_back(v);
			mesh->normals.push_back(n);
			mesh->uvs.push_back(uv);
			
			if(h<minorSlices-1 && i<majorSlices-1)
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + h;
				f.vertexID[2] = i*minorSlices + (h+1);
				f.normalID[0] = f.vertexID[0];
				f.normalID[1] = f.vertexID[1];
				f.normalID[2] = f.vertexID[2];
				f.uvID[0] = f.vertexID[0];
				f.uvID[1] = f.vertexID[1];
				f.uvID[2] = f.vertexID[2];
				mesh->faces.push_back(f);
				f.vertexID[0] = (i+1)*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + (h+1);
				f.vertexID[2] = i*minorSlices + (h+1);
				f.normalID[0] = f.vertexID[0];
				f.normalID[1] = f.vertexID[1];
				f.normalID[2] = f.vertexID[2];
				f.uvID[0] = f.vertexID[0];
				f.uvID[1] = f.vertexID[1];
				f.uvID[2] = f.vertexID[2];
				mesh->faces.push_back(f);
			}
			else if(i<majorSlices-1) //Last patch in the middle slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + h;
				f.vertexID[2] = i*minorSlices;
				f.normalID[0] = f.vertexID[0];
				f.normalID[1] = f.vertexID[1];
				f.normalID[2] = f.vertexID[2];
				f.uvID[0] = f.vertexID[0];
				f.uvID[1] = f.vertexID[1];
				f.uvID[2] = f.vertexID[2];
				mesh->faces.push_back(f);
				f.vertexID[0] = (i+1)*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices;
				f.vertexID[2] = i*minorSlices;
				f.normalID[0] = f.vertexID[0];
				f.normalID[1] = f.vertexID[1];
				f.normalID[2] = f.vertexID[2];
				f.uvID[0] = f.vertexID[0];
				f.uvID[1] = f.vertexID[1];
				f.uvID[2] = f.vertexID[2];
				mesh->faces.push_back(f);
			}
			else if(h<minorSlices-1) //Middle patch in the last slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = h;
				f.vertexID[2] = i*minorSlices + (h+1);
				f.normalID[0] = f.vertexID[0];
				f.normalID[1] = f.vertexID[1];
				f.normalID[2] = f.vertexID[2];
				f.uvID[0] = f.vertexID[0];
				f.uvID[1] = f.vertexID[1];
				f.uvID[2] = f.vertexID[2];
				mesh->faces.push_back(f);
				f.vertexID[0] = h;
				f.vertexID[1] = h+1;
				f.vertexID[2] = i*minorSlices + (h+1);
				f.normalID[0] = f.vertexID[0];
				f.normalID[1] = f.vertexID[1];
				f.normalID[2] = f.vertexID[2];
				f.uvID[0] = f.vertexID[0];
				f.uvID[1] = f.vertexID[1];
				f.uvID[2] = f.vertexID[2];
				mesh->faces.push_back(f);
			}
			else //Last patch in the last slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = h;
				f.vertexID[2] = i*minorSlices;
				f.normalID[0] = f.vertexID[0];
				f.normalID[1] = f.vertexID[1];
				f.normalID[2] = f.vertexID[2];
				f.uvID[0] = f.vertexID[0];
				f.uvID[1] = f.vertexID[1];
				f.uvID[2] = f.vertexID[2];
				mesh->faces.push_back(f);
				f.vertexID[0] = h;
				f.vertexID[1] = 0;
				f.vertexID[2] = i*minorSlices;
				f.normalID[0] = f.vertexID[0];
				f.normalID[1] = f.vertexID[1];
				f.normalID[2] = f.vertexID[2];
				f.uvID[0] = f.vertexID[0];
				f.uvID[1] = f.vertexID[1];
				f.uvID[2] = f.vertexID[2];
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
    bool normals = false, uvs = false;
    
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
                mesh->vertices.push_back(v);
            }
            else if(c2 == 'n')
            {
                glm::vec3 n;
                sscanf(line, "vn %f %f %f\n", &n.x, &n.y, &n.z);
                mesh->normals.push_back(n);
                normals = true;
            }
            else if(c2 == 't')
            {
                glm::vec2 uv;
                sscanf(line, "vt %f %f\n", &uv.x, &uv.y);
                mesh->uvs.push_back(uv);
                uvs = true;
            }
        }
        else if(c1 == 'f')
        {
            Face face;
       
            if(normals && uvs)
            {
                sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", &face.vertexID[0], &face.uvID[0], &face.normalID[0], &face.vertexID[1], &face.uvID[1], &face.normalID[1], &face.vertexID[2], &face.uvID[2], &face.normalID[2]);
                face.vertexID[0] -= 1;
                face.vertexID[1] -= 1;
                face.vertexID[2] -= 1;
                face.uvID[0] -= 1;
                face.uvID[1] -= 1;
                face.uvID[2] -= 1;
                face.normalID[0] -= 1;
                face.normalID[1] -= 1;
                face.normalID[2] -= 1;
            }
            else if(normals)
            {
                sscanf(line, "f %u//%u %u//%u %u//%u\n", &face.vertexID[0], &face.normalID[0], &face.vertexID[1], &face.normalID[1], &face.vertexID[2], &face.normalID[2]);
                face.vertexID[0] -= 1;
                face.vertexID[1] -= 1;
                face.vertexID[2] -= 1;
                face.uvID[0] = face.uvID[1] = face.uvID[2] = 0;
                face.normalID[0] -= 1;
                face.normalID[1] -= 1;
                face.normalID[2] -= 1;
            }
            else
            {
                sscanf(line, "f %u %u %u\n", &face.vertexID[0], &face.vertexID[1], &face.vertexID[2]);
                face.vertexID[0] -= 1;
                face.vertexID[1] -= 1;
                face.vertexID[2] -= 1;
                face.uvID[0] = face.uvID[1] = face.uvID[2] = 0;
                
                glm::vec3 v1(mesh->vertices[face.vertexID[1]].x - mesh->vertices[face.vertexID[0]].x,
							 mesh->vertices[face.vertexID[1]].y - mesh->vertices[face.vertexID[0]].y,
							 mesh->vertices[face.vertexID[1]].z - mesh->vertices[face.vertexID[0]].z);
                
                glm::vec3 v2(mesh->vertices[face.vertexID[2]].x - mesh->vertices[face.vertexID[0]].x,
							 mesh->vertices[face.vertexID[2]].y - mesh->vertices[face.vertexID[0]].y,
							 mesh->vertices[face.vertexID[2]].z - mesh->vertices[face.vertexID[0]].z);
                
				glm::vec3 n = glm::normalize(glm::cross(v1,v2));
                mesh->normals.push_back(n);
                face.normalID[0] = face.normalID[1] = face.normalID[2] = mesh->normals.size()-1;
            }
            
            mesh->faces.push_back(face);
        }
    }
    
    fclose(file);
   
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
    
    while(fgets(line, 256, file))
    {
        sscanf(line, "%s", keyword);
        
        if(strcmp(keyword, "facet")==0)
        {
            glm::vec3 n;
            sscanf(line, " facet normal %f %f %f\n", &n.x, &n.y, &n.z);
            mesh->normals.push_back(n);
        }
        else if(strcmp(keyword, "vertex")==0)
        {
            glm::vec3 v;
            sscanf(line, " vertex %f %f %f\n", &v.x, &v.y, &v.z);
			v *= scale;
            mesh->vertices.push_back(v);
        }
        else if(strcmp(keyword, "endfacet")==0)
        {
            Face f;
            uint32_t lastVertexIndex = mesh->vertices.size()-1;
            uint32_t lastNormalIndex = mesh->normals.size()-1;
            f.vertexID[0] = lastVertexIndex-2;
            f.vertexID[1] = lastVertexIndex-1;
            f.vertexID[2] = lastVertexIndex;
            f.normalID[0] = f.normalID[1] = f.normalID[2] = lastNormalIndex;
            f.uvID[0] = f.uvID[1] = f.uvID[2] = 0;
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
    mesh->normals.clear();
		
	//for all faces
	for(unsigned int i=0; i<mesh->faces.size(); ++i)
	{
		Face thisFace = mesh->faces[i];
		glm::vec3 thisN = mesh->computeFaceNormal(i);
						
		//For every vertex
		for(uint8_t h=0; h<3; ++h)
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
                        
					for(uint8_t k=0; k<3; k++)
					{	
						if((thatFace.vertexID[k] == thisFace.vertexID[h])&&(glm::dot(thatN,thisN) > 0.707f))
						{
							n += thatN;
							contrib++;
							break;
						}
					}
				}
			}
                
			n /= (GLfloat)contrib;
			mesh->normals.push_back(n);
			mesh->faces[i].normalID[h] = mesh->normals.size()-1;
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
		glm::vec3 edge0 = mesh->vertices[firstID];
		glm::vec3 edge1 = mesh->vertices[secondID];
		glm::vec3 point = glm::normalize(edge0 + edge1);
		mesh->vertices.push_back(point);
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
		glm::vec3 edge0 = mesh->vertices[firstID];
		glm::vec3 edge1 = mesh->vertices[secondID];
		glm::vec3 point = (edge0 + edge1)/(GLfloat)2;
		mesh->vertices.push_back(point);
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
        glm::vec3 vertex = mesh->vertices[i];
        
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
        tempCenter += mesh->vertices[i];
    
    tempCenter /= (GLfloat)mesh->vertices.size();
    
    GLfloat radius = 0;
    
    for(unsigned int i=0; i<mesh->vertices.size(); ++i)
    {
        glm::vec3 v = mesh->vertices[i];
        GLfloat r = glm::length(v-tempCenter);
        if(r > radius)
            radius = r;
    }
    
    bsRadius = (btScalar)radius;
    bsCenterOffset.setX((btScalar)tempCenter.x);
    bsCenterOffset.setY((btScalar)tempCenter.y);
    bsCenterOffset.setZ((btScalar)tempCenter.z);
}