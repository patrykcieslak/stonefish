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
//  Comm.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/02/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Comm__
#define __Stonefish_Comm__

#include <SDL2/SDL_mutex.h>
#include <deque>
#include "StonefishCommon.h"
#include <variant>

namespace sf
{
    //! An enum defining types of comms.
    enum class CommType {RADIO, ACOUSTIC, USBL, VLC, OPTIC};
    
    struct Renderable;
    class Entity;
    class StaticEntity;
    class MovingEntity;
    
struct Point {
    float x, y, z;
    float intensity;
};

struct PointCloud {
    std::vector<Point> points;
    std::string frame_id;
    double timestamp;
};
    
    struct CommDataFrame
    {
        using DataType = std::variant<int, float, double, std::string, std::vector<uint8_t>, std::vector<float>, std::vector<double>, PointCloud>;
        
        Scalar timeStamp;
        uint64_t seq;
        uint64_t source;
        uint64_t destination;
        std::string data;
        DataType data_mission;
    };
    
    //! An abstract class representing a communication device.
    class Comm
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the comm device
         \param deviceId an identification code of the device
         */
        Comm(std::string uniqueName, uint64_t deviceId);
        
        //! A destructor.
        virtual ~Comm();
        
        //! A method to set the connected device id.
        /*!
         \param deviceId an identifier of the connected node
         */
        void Connect(uint64_t deviceId);
        
        //! A method used to send a message.
        /*!
         \param data the data to be sent
         */
        virtual void SendMessage(std::string data);
        
        //! A method to read received data frames. The data frame has to be destroyed manually. 
        /*!
         \return a pointer to the data frame
         */
        CommDataFrame* ReadMessage();
                
        //! A method used to attach the comm device to the world origin.
        /*!
         \param origin the place where the comm should be attached in the world frame
         */
        void AttachToWorld(const Transform& origin);
        
        //! A method used to attach the comm device to a static body.
        /*!
         \param body a pointer to the static body
         \param origin the place where the comm should be attached in the static body origin frame
         */
        void AttachToStatic(StaticEntity* body, const Transform& origin);
        
        //! A method used to attach the comm device to a rigid body.
        /*!
         \param body a pointer to the rigid body
         \param origin the place where the comm should be attached in the solid origin frame
         */
        void AttachToSolid(MovingEntity* body, const Transform& origin);
        
        //! A method implementing the rendering of the comm device.
        virtual std::vector<Renderable> Render();
        
        //! A method that updates the comm state.
        /*!
         \param dt a time step of the simulation [s]
         */
        void Update(Scalar dt);
        
        //! A method used to mark data as old.
        void MarkDataOld();
        
        //! A method to check if new data is available.
        bool isNewDataAvailable();
        
        //! A method informing if the comm is renderable.
        bool isRenderable();
        
        //! A method to set if the comm is renderable.
        void setRenderable(bool render);
        
        //! A method returning the current comm device frame in world.
        Transform getDeviceFrame();
        
        //! A method returning the device node id.
        uint64_t getDeviceId();
        
        //! A method returning the conneted device node id.
        uint64_t getConnectedId();
        
        //! A method returning the comm name.
        std::string getName();
        
        //! A method performing an internal update of the comm state.
        /*!
         \param dt the time step of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method returning the type of the comm.
        virtual CommType getType() const = 0;
        
    protected:
        //! A method used for data reception.
        void MessageReceived(CommDataFrame* message);
        //! A method to proccess received messages.
        virtual void ProcessMessages() = 0;
    
        bool newDataAvailable;
        std::deque<CommDataFrame*> txBuffer;
        std::deque<CommDataFrame*> rxBuffer;
        uint64_t txSeq;
        
    private:
        std::string name;
        uint64_t id;
        int64_t cId;
        SDL_mutex* updateMutex;
        Entity* attach;
        Transform o2c;
        bool renderable;
    };
}

#endif
