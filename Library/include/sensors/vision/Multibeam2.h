//
//  Multibeam2.h
//  Stonefish
//
//  Created by Patryk Cieślak on 21/01/2019.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Multibeam2__
#define __Stonefish_Multibeam2__

#include <functional>
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLDataStructs.h"

#define MULTIBEAM_MAX_SINGLE_FOV 30.0

namespace sf
{
    class OpenGLDepthCamera;
    
    struct CamData
    {
        OpenGLDepthCamera* cam;
        int width;
        GLfloat fovH;
        size_t dataOffset;
    };
    
    //!
    class Multibeam2 : public Camera
    {
    public:
        Multibeam2(std::string uniqueName, unsigned int horizontalRes, unsigned int verticalRes, Scalar horizontalFOVDeg, Scalar verticalFOVDeg,
                    Scalar minDepth, Scalar maxDepth, Scalar frequency = Scalar(-1));
        ~Multibeam2();
        
        void InternalUpdate(Scalar dt);
        void UpdateTransform();
        void SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up);
        void SetupCamera(size_t index, const Vector3& eye, const Vector3& dir, const Vector3& up);
        void InstallNewDataHandler(std::function<void(Multibeam2*)> callback);
        void NewDataReady(unsigned int index = 0);
        std::vector<Renderable> Render();
        
        glm::vec2 getDepthRange();
        void* getImageDataPointer(unsigned int index = 0);
        float* getRangeDataPointer();
        
    private:
        void InitGraphics();
        
        std::vector<CamData> cameras;
        GLfloat* imageData;
        GLfloat* rangeData;
        Scalar fovV;
        glm::vec2 depthRange;
        std::function<void(Multibeam2*)> newDataCallback;
        int dataCounter;
    };




}

#endif
