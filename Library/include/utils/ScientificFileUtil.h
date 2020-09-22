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
//  ScientificFileUtil.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/07/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ScientificFileUtil__
#define __Stonefish_ScientificFileUtil__

#include "LinearMath/btMatrixX.h"
#include "StonefishCommon.h"

namespace sf
{
    //! An enum defining supported types of data.
    typedef enum {DATA_SCALAR, DATA_VECTOR, DATA_MATRIX} ScientificDataType;
    
    //! A structure representing a single data item.
    struct ScientificDataItem
    {
        std::string name;
        ScientificDataType type;
        void* value;
        
        //! A constructor.
        ScientificDataItem() : value(NULL) {}
        
        //! A destructor.
        ~ScientificDataItem()
        {
            if(value != NULL)
            {
                switch(type)
                {
                    case DATA_SCALAR:
                        delete ((Scalar*)value);
                        break;
                        
                    case DATA_VECTOR:
                        delete ((btVectorXu*)value);
                        break;
                        
                    case DATA_MATRIX:
                        delete ((btMatrixXu*)value);
                        break;
                }
            }
        }
    };
    
    //! A class representing scientific data structure.
    class ScientificData
    {
    public:
        //! A constructor.
        /*!
         \param filepath the path of the output file
         */
        ScientificData(const std::string& filepath);
        
        //! A destructor.
        virtual ~ScientificData();
        
        //! A method to add a data item to the list
        /*!
         \param it a pointer to a data item
         */
        void addItem(ScientificDataItem* it);
        
        //! A method returning a data item.
        /*!
         \param name the name of the item
         \return a pointer to the data item
         */
        const ScientificDataItem* getItem(std::string name) const;
        
        //! A method returning a data item.
        /*!
         \param index the id of the data item on the list
         \return a pointer to the data item
         */
        const ScientificDataItem* getItem(unsigned int index) const;
        
        //! A method returning the total number of items.
        unsigned int getItemsCount() const;
        
        //! A method to get value of a single scalar data item.
        /*!
         \param name the name of the data item
         \return value
         */
        Scalar getScalar(std::string name);
        
        //! A method to get value of a vector data item.
        /*!
         \param name the name of the data item
         \return value
         */
        btVectorXu getVector(std::string name);
        
        //! A method to get value of a matrix data item.
        /*!
         \param name the name of the data item
         \return value
         */
        btMatrixXu getMatrix(std::string name);
        
    private:
        std::string path;
        std::vector<ScientificDataItem*> items;
    };
    
    //! A function to load data from an Octave file.
    /*!
     \param path the path to the file
     \return a pointer to a data structure
     */
    ScientificData* LoadOctaveData(const std::string& path);
    
    //! A function to load scalar data from an Octave file.
    /*!
     \param file the path to the file
     \param it a pointer to the new data item
     \param isFloat defines if value type is float (single precision)
     \return success
     */
    bool LoadOctaveScalar(std::ifstream& file, ScientificDataItem* it, bool isFloat = false);
    
    //! A function to load matrix data from an Octave file.
    /*!
     \param file the path to the file
     \param it a pointer to the new data item
     \param isFloat defines if value type is float (single precision)
     \return success
     */
    bool LoadOctaveMatrix(std::ifstream& file, ScientificDataItem* it, bool isFloat = false);
    
    //! A function to save data to an Octave file.
    /*!
     \param path the path to the file
     \param data a reference to the data structure
     \return success
     */
    bool SaveOctaveData(const std::string& path, const ScientificData& data);
    
    //! A function to save scalar data to an Octave file.
    /*!
     \param file the path to the file
     \param it reference to the data item
     */
    void SaveOctaveScalar(std::ofstream& file, const ScientificDataItem& it);
    
    //! A function to save matrix data to an Octave file.
    /*!
     \param file the path to the file
     \param it a reference to the data item
     */
    void SaveOctaveMatrix(std::ofstream& file, const ScientificDataItem& it);
}

#endif
