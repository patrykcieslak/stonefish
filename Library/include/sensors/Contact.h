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
//  Contact.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Contact__
#define __Stonefish_Contact__

#include <deque>
#include "StonefishCommon.h"

namespace sf
{
    //! An enum specifying the style of contact rendering.
    typedef enum {
        CONTACT_DISPLAY_NONE = 0,
        CONTACT_DISPLAY_LAST_A = 1 << 0,
        CONTACT_DISPLAY_PATH_A = 1 << 1,
        CONTACT_DISPLAY_LAST_SLIP_VELOCITY_A = 1 << 2,
        CONTACT_DISPLAY_SLIP_VELOCITY_SEQUENCE_A = 1 << 3,
        CONTACT_DISPLAY_NORMAL_FORCE_A = 1 << 4,
        CONTACT_DISPLAY_LAST_B = 1 << 5,
        CONTACT_DISPLAY_PATH_B = 1 << 6,
        CONTACT_DISPLAY_LAST_SLIP_VELOCITY_B = 1 << 7,
        CONTACT_DISPLAY_SLIP_VELOCITY_SEQUENCE_B = 1 << 8,
        CONTACT_DISPLAY_NORMAL_FORCE_B = 1 << 9,
    } ContactDisplayType;
    
    //! A structure containing the data of a contact point.
    struct ContactPoint
    {
        Scalar timeStamp;
        Vector3 locationA;
        Vector3 locationB;
        Vector3 slippingVelocityA;
        Vector3 normalForceA;
    };

    //! A structure containing the internal data attached to a contact point.
    struct ContactInfo
    {
        Scalar totalAppliedImpulse;
        Vector3 slip;
    };
    
    struct Renderable;
    class Entity;
    
    //! A class implementing a sensor measuring the contact between two entities.
    class Contact
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the contact
         \param entityA a pointer to the first entity
         \param entityB a pointer to the second entity
         \param historyLength defines: 0 -> unlimited history, >0 -> history with a specified length
         */
        Contact(std::string uniqueName, Entity* entityA, Entity* entityB, unsigned int historyLength = 1);
        
        //! A destructor.
        ~Contact();

        //! A method to add a contact point to the history.
        /*!
         \param manifold a pointer to the contact manifold
         \param swapped a flag indicating if the contact bodies are swapped
         \param dt time step of the simulation tick callback
         */
        void AddContactPoint(const btPersistentManifold* manifold, bool swapped, Scalar dt);
        
        //! A method to add a contact point to the history.
        /*!
         \param p a structure containing contact info
         */
        void AddContactPoint(ContactPoint p);
        
        //! A method clearing the contact history.
        void ClearHistory();
        
        //! A method used to mark data as old.
        void MarkDataOld();
        
        //! A method to check if new data is available.
        bool isNewDataAvailable();
        
        //! A method that saves contact data to an Octave file.
        /*!
         \param path a path to the output file
         \param includeTime a flag to specify if time should be written
         */
        void SaveContactDataToOctaveFile(const std::string& path, bool includeTime = true);
        
        //! A method that implements rendering of the contact.
        std::vector<Renderable> Render();
        
        //! A method to set the display style of the contact.
        /*!
         \param mask an integer which defines the rendering style
         */
        void setDisplayMask(int16_t mask);
        
        //! A method returning the sensor name.
        std::string getName();
        
        //! A method returning a pointer to the first entity.
        const Entity* getEntityA();
        
        //! A method returning a pointer to the second entity.
        const Entity* getEntityB();
        
        //! A method returning the history of the contact.
        const std::deque<ContactPoint>& getHistory();
        
    private:
        std::string name;
        Entity* A;
        Entity* B;
        std::deque<ContactPoint> points;
        std::deque<ContactPoint>::iterator pointsLastBeg;
        unsigned int historyLen;
        int16_t displayMask;
        bool newDataAvailable;
    };
}

#endif
