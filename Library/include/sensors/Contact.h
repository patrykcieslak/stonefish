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
    
    struct Renderable;
    class Entity;
    
    //! A class implementing a sensor measuring the contact between two entities.
    class Contact
    {
    public:
        //! A constructor.
        /*!
         \param entityA a pointer to the first entity
         \param entityB a pointer to the second entity
         \param historyLength defines: 0 -> unlimited history, >0 -> history with a specified length
         */
        Contact(Entity* entityA, Entity* entityB, unsigned int historyLength = 1);
        
        //! A destructor.
        ~Contact();
        
        //! A method to add a contact point to the history.
        /*!
         \param manifold a pointer to the contact manifold
         \param swapped
         */
        void AddContactPoint(const btPersistentManifold* manifold, bool swapped);
        
        //! A method to add a contact point to the history.
        /*!
         \param p a structure containing contact info
         */
        void AddContactPoint(ContactPoint p);
        
        //! A method clearing the contact history.
        void ClearHistory();
        
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
        
        //! A method returning a pointer to the first entity.
        const Entity* getEntityA();
        
        //! A method returning a pointer to the second entity.
        const Entity* getEntityB();
        
        //! A method returning the history of the contact.
        const std::deque<ContactPoint>& getHistory();
        
    private:
        Entity* A;
        Entity* B;
        std::deque<ContactPoint> points;
        unsigned int historyLen;
        int16_t displayMask;
    };
}

#endif
