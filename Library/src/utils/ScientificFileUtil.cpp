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
//  ScientificFileUtil.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "utils/ScientificFileUtil.h"

#include <iostream>
#include <fstream>
#include "core/SimulationApp.h"

namespace sf
{

ScientificData::ScientificData(const std::string& filepath) : path(filepath)
{
}

ScientificData::~ScientificData()
{
    for(unsigned int i = 0; i < items.size(); ++i)
        delete items[i];
    
    items.clear();
}

void ScientificData::addItem(ScientificDataItem *it)
{
    if(it != NULL && it->value != NULL)
        items.push_back(it);
}

const ScientificDataItem* ScientificData::getItem(std::string name) const
{
    for(unsigned int i = 0; i < items.size(); ++i)
        if(items[i]->name == name)
            return items[i];
    
    return NULL;
}

const ScientificDataItem* ScientificData::getItem(unsigned int index) const
{
    if(index < items.size())
        return items[index];
    else
        return NULL;
}

unsigned int ScientificData::getItemsCount() const
{
    return (unsigned int)items.size();
}

Scalar ScientificData::getScalar(std::string name)
{
    const ScientificDataItem* it = getItem(name);
    if(it != NULL && it->type == DATA_SCALAR)
        return *((Scalar*)it->value);
    else
        return Scalar(0.);
}

btVectorXu ScientificData::getVector(std::string name)
{
    const ScientificDataItem* it = getItem(name);
    if(it != NULL && it->type == DATA_VECTOR)
        return *((btVectorXu*)it->value);
    else
        return btVectorXu();
}

btMatrixXu ScientificData::getMatrix(std::string name)
{
    const ScientificDataItem* it = getItem(name);
    if(it != NULL && it->type == DATA_MATRIX)
        return *((btMatrixXu*)it->value);
    else
        return btMatrixXu();
}

ScientificData* LoadOctaveData(const std::string& path)
{
    //Open file
    cInfo("Loading scientific data from: %s", path.c_str());
    
    std::ifstream file;
    file.open(path.c_str());
    if(!file.is_open())
    {
        cError("File could not be opened!");
        return NULL;
    }
    
    //Check file identifier [10 bytes]
    const int idStringLen = 10;
    char idString[idStringLen + 1];
    file.read(idString, idStringLen);
    idString[idStringLen] = '\0';
    
    //Get endianness
    //bool littleEndian;
    
    if(strncmp(idString, "Octave-1-L", idStringLen) == 0)
        ; //littleEndian = true;
    else if(strncmp(idString, "Octave-1-B", idStringLen) == 0)
    {
        //littleEndian = false;
        cError("File format not supported!");
        return NULL;
    }
    else
    {
        cError("File format not recognized!");
        return NULL;
    }
    
    //Get float format [1 byte]
    char floatFormat;
    file.read(&floatFormat, 1);
    
    if(floatFormat > 1)
    {
        cError("Float format not recognized!");
        return NULL;
    }
    
    //Read data items
    ScientificData* sdata = new ScientificData(path);
    
    while(!file.eof())
    {
        //read item name [4 bytes + nameLen]
        int32_t nameLen = 0;
        file.read(reinterpret_cast<char*>(&nameLen), 4);
        
        //check if EOF reached
        if(file.eof())
            break;
        
        char cname[nameLen + 1];
        cname[nameLen] = '\0';
        file.read(cname, nameLen);
        
        //skip doc [4 bytes + docLen]
        int32_t docLen = 0;
        file.read(reinterpret_cast<char*>(&docLen), 4);
        file.seekg(docLen, std::ios_base::cur);
        
        //skip global flag [1 byte]
        file.seekg(1, std::ios_base::cur);
        
        //get data type [1 byte]
        unsigned char dataType;
        file.read(reinterpret_cast<char*>(&dataType), 1);
        
        //only scalars and real matrices supported!
        if(dataType == 1) //scalar
        {
            ScientificDataItem* item = new ScientificDataItem();
            item->name = std::string(cname);
            
            if(!LoadOctaveScalar(file, item, floatFormat != 0))
            {
                cError("Failed to load scalar \"%s\"!", cname);
                delete item;
                delete sdata;
                return NULL;
            }
            
            sdata->addItem(item);
        }
        else if(dataType == 2) //matrix
        {
            ScientificDataItem* item = new ScientificDataItem();
            item->name = std::string(cname);
            
            if(!LoadOctaveMatrix(file, item, floatFormat != 0))
            {
                cError("Failed to load matrix \"%s\"!", cname);
                delete item;
                delete sdata;
                return NULL;
            }
            
            sdata->addItem(item);
        }
        else if(dataType == 255) //new type specifier
        {
            //get type name [4 bytes + typeLen]
            int32_t typeLen = 0;
            file.read(reinterpret_cast<char*>(&typeLen), 4);
            
            char typeName[typeLen + 1];
            typeName[typeLen] = '\0';
            file.read(typeName, typeLen);
            
            if(strncmp(typeName, "scalar", typeLen) == 0)
            {
                ScientificDataItem* item = new ScientificDataItem();
                item->name = std::string(cname);
                
                if(!LoadOctaveScalar(file, item))
                {
                    cError("Failed to load scalar \"%s\"!", cname);
                    delete item;
                    delete sdata;
                    return NULL;
                }
                
                sdata->addItem(item);
            }
            else if(strncmp(typeName, "matrix", typeLen) == 0)
            {
                ScientificDataItem* item = new ScientificDataItem();
                item->name = std::string(cname);
                
                if(!LoadOctaveMatrix(file, item))
                {
                    cError("Failed to load matrix \"%s\"!", cname);
                    delete item;
                    delete sdata;
                    return NULL;
                }
                sdata->addItem(item);
            }
            else if(strncmp(typeName, "float scalar", typeLen) == 0)
            {
                ScientificDataItem* item = new ScientificDataItem();
                item->name = std::string(cname);
                
                if(!LoadOctaveScalar(file, item, true))
                {
                    cError("Failed to load scalar \"%s\"!", cname);
                    delete item;
                    delete sdata;
                    return NULL;
                }
                
                sdata->addItem(item);
            }
            else if(strncmp(typeName, "float matrix", typeLen) == 0)
            {
                ScientificDataItem* item = new ScientificDataItem();
                item->name = std::string(cname);
                
                if(!LoadOctaveMatrix(file, item, true))
                {
                    cError("Failed to load matrix \"%s\"!", cname);
                    delete item;
                    delete sdata;
                    return NULL;
                }
                
                sdata->addItem(item);
            }
            else
            {
                cError("Data type of \"%s\" not supported!", cname);
                delete sdata;
                return NULL;
            }
        }
        else
        {
            cError("Data type of \"%s\" not supported!", cname);
            delete sdata;
            return NULL;
        }
    }
    
    return sdata;
}

bool LoadOctaveScalar(std::ifstream& file, ScientificDataItem* it, bool isFloat)
{
    it->type = DATA_SCALAR;
    
    //skip 1 byte
    file.seekg(1, std::ios_base::cur);
    
    //read value
    if(isFloat)
    {
        float* scalar = new float;
        file.read(reinterpret_cast<char*>(scalar), 4);
        it->value = scalar;
    }
    else //double
    {
        double* scalar = new double;
        file.read(reinterpret_cast<char*>(scalar), 8);
        it->value = scalar;
    }
    
    return true;
}

bool LoadOctaveMatrix(std::ifstream& file, ScientificDataItem* it, bool isFloat)
{
    //get number of dimensions
    int32_t dims;
    file.read(reinterpret_cast<char*>(&dims), 4);
    dims = -dims;
    
    if(dims != 2) //something is wrong...
        return false;
    
    //get matrix dimensions
    uint32_t rows;
    uint32_t cols;
    file.read(reinterpret_cast<char*>(&rows), 4);
    file.read(reinterpret_cast<char*>(&cols), 4);
    
    //skip 1 byte
    file.seekg(1, std::ios_base::cur);
    
    if(rows == 1 || cols == 1) //vector
    {
        //setup vector with size
        it->type = DATA_VECTOR;
        uint32_t vLen = rows == 1 ? cols : rows;
        btVectorXu* v = new btVectorXu(vLen);
        
        //choose float format
        if(isFloat)
        {
            for(uint32_t i = 0; i < vLen; ++i)
            {
                float value;
                file.read(reinterpret_cast<char*>(&value), 4);
                (*v)[i] = (Scalar)value;
            }
        }
        else //double
        {
            for(uint32_t i = 0; i < vLen; ++i)
            {
                double value;
                file.read(reinterpret_cast<char*>(&value), 8);
                (*v)[i] = (Scalar)value;
            }
        }
        
        it->value = v;
    }
    else //matrix
    {
        //setup matrix with size
        it->type = DATA_MATRIX;
        btMatrixXu* m = new btMatrixXu(rows, cols);
        
        //choose float format
        if(isFloat)
        {
            for(uint32_t i = 0; i < cols; ++i)
                for (uint32_t h = 0; h < rows; ++h)
                {
                    float value;
                    file.read(reinterpret_cast<char*>(&value), 4);
                    m->setElem(h, i, (Scalar)value);
                }
        }
        else //double
        {
            for(uint32_t i = 0; i < cols; ++i)
                for (uint32_t h = 0; h < rows; ++h)
                {
                    double value;
                    file.read(reinterpret_cast<char*>(&value), 8);
                    m->setElem(h, i, (Scalar)value);
                }
        }
        
        it->value = m;
    }
    
    return true;
}

bool SaveOctaveData(const std::string& path, const ScientificData& data)
{
    //open file
    cInfo("Saving scientific data to: %s", path.c_str());
    
    std::ofstream file;
    file.open(path.c_str());
    if(!file.is_open())
    {
        cError("File could not be opened!");
        return false;
    }
    
    //write file identifier [10 bytes]
    file.write("Octave-1-L", 10);
    
    //write float format [1 byte]
    char floatFormat = 0;
    file.write(&floatFormat, 1);
    
    //write data items
    for(unsigned int i = 0; i < data.getItemsCount(); ++i)
    {
        const ScientificDataItem* it = data.getItem(i);
        
        //write item name [4 bytes + nameLen]
        int32_t nameLen = (int32_t)it->name.length();
        file.write(reinterpret_cast<char*>(&nameLen), 4);
        file.write(it->name.c_str(), nameLen);

        //write empty doc [4 bytes]
        int32_t docLen = 0;
        file.write(reinterpret_cast<char*>(&docLen), 4);
        
        //write global flag [1 byte]
        char global = 0;
        file.write(&global, 1);
        
        //write data type [1 byte]
        unsigned char dataType = 255; //new type definition
        file.write(reinterpret_cast<char*>(&dataType), 1);

        //write data
        switch (it->type)
        {
            case DATA_SCALAR:
            {
                //write type name [4 bytes + typeLen]
#ifdef BT_USE_DOUBLE_PRECISION
                std::string typeName = "scalar";
#else
                std::string typeName = "float scalar";
#endif
                int32_t typeLen = (int32_t)typeName.length();
                file.write(reinterpret_cast<char*>(&typeLen), 4);
                file.write(typeName.c_str(), typeLen);
                
                //write data
                SaveOctaveScalar(file, *it);
            }
                break;
                
            case DATA_VECTOR:
            case DATA_MATRIX:
            {
                //write type name [4 bytes + typeLen]
#ifdef BT_USE_DOUBLE_PRECISION
                std::string typeName = "matrix";
#else
                std::string typeName = "float matrix";
#endif
                int32_t typeLen = (int32_t)typeName.length();
                file.write(reinterpret_cast<char*>(&typeLen), 4);
                file.write(typeName.c_str(), typeLen);
                
                //write data
                SaveOctaveMatrix(file, *it);
            }
                break;
        }
    }
    
    return true;
}

void SaveOctaveScalar(std::ofstream& file, const ScientificDataItem& it)
{
#ifdef BT_USE_DOUBLE_PRECISION
    //write type identifier [1 byte]
    char type = 7;
    file.write(&type, 1);
    
    //write value
    Scalar* scalar = (Scalar*)it.value;
    file.write(reinterpret_cast<char*>(scalar), 8);
#else
    //write type identifier [1 byte]
    char type = 6;
    file.write(&type, 1);
    
    //write value
    Scalar* scalar = (Scalar*)it.value;
    file.write(reinterpret_cast<char*>(scalar), 4);
#endif
}

void SaveOctaveMatrix(std::ofstream& file, const ScientificDataItem& it)
{
    //write number of dimensions (always 2)
    int32_t dims = -2;
    file.write(reinterpret_cast<char*>(&dims), 4);
    
    if(it.type == DATA_VECTOR) //column vector
    {
        btVectorXu* vector = (btVectorXu*)it.value;
        
        //write size [2 x 4 bytes]
        uint32_t rows = vector->size();
        uint32_t cols = 1;
        file.write(reinterpret_cast<char*>(&rows), 4);
        file.write(reinterpret_cast<char*>(&cols), 4);
        
#ifdef BT_USE_DOUBLE_PRECISION
        //write type identifier [1 byte]
        char type = 7;
        file.write(&type, 1);
        
        //write value
        for(unsigned int i = 0; i < rows; ++i)
        {
            double value = (*vector)[i];
            file.write(reinterpret_cast<char*>(&value), 8);
        }
#else
        //write type identifier [1 byte]
        char type = 6;
        file.write(&type, 1);
        
        //write value
        for(unsigned int i = 0; i < rows; ++i)
        {
            float value = (*vector)[i];
            file.write(reinterpret_cast<char*>(&value), 4);
        }
#endif
    }
    else //matrix
    {
        btMatrixXu* matrix = (btMatrixXu*)it.value;
        
        //write size [2 x 4 bytes]
        uint32_t rows = matrix->rows();
        uint32_t cols = matrix->cols();
        file.write(reinterpret_cast<char*>(&rows), 4);
        file.write(reinterpret_cast<char*>(&cols), 4);
        
#ifdef BT_USE_DOUBLE_PRECISION
        //write type identifier [1 byte]
        char type = 7;
        file.write(&type, 1);
        
        //write value
        for(unsigned int i = 0; i < cols; ++i)
            for(unsigned int h = 0; h < rows; ++h)
            {
                double value = (*matrix)(h,i);
                file.write(reinterpret_cast<char*>(&value), 8);
            }
#else
        //write type identifier [1 byte]
        char type = 6;
        file.write(&type, 1);
        
        //write value
        for(unsigned int i = 0; i < cols; ++i)
            for(unsigned int h = 0; h < rows; ++h)
            {
                float value = (*matrix)(h, i);
                file.write(reinterpret_cast<char*>(&value), 4);
            }
#endif
    }
}

}
