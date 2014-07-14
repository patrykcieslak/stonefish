//
//  ScientificFileUtil.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "ScientificFileUtil.h"
#include "Console.h"
#include <iostream>
#include <fstream>

#pragma mark ScientificData class
ScientificData::ScientificData(std::string filepath) : path(filepath)
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

ScientificDataItem* ScientificData::getItem(std::string name)
{
    for(unsigned int i = 0; i < items.size(); ++i)
        if(items[i]->name == name)
            return items[i];
    
    return NULL;
}

btScalar ScientificData::getScalar(std::string name)
{
    ScientificDataItem* it = getItem(name);
    if(it != NULL && it->type == DATA_SCALAR)
        return *((btScalar*)it->value);
    else
        return btScalar(0.);
}

btVectorXu ScientificData::getVector(std::string name)
{
    ScientificDataItem* it = getItem(name);
    if(it != NULL && it->type == DATA_VECTOR)
        return *((btVectorXu*)it->value);
    else
        return btVectorXu();
}

btMatrixXu ScientificData::getMatrix(std::string name)
{
    ScientificDataItem* it = getItem(name);
    if(it != NULL && it->type == DATA_MATRIX)
        return *((btMatrixXu*)it->value);
    else
        return btMatrixXu();
}

#pragma mark - Octave
ScientificData* LoadOctaveData(const char* path)
{
    //Open file
    cInfo("Loading scientific data from: %s", path);
    
    std::ifstream file;
    file.open(path);
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
    bool littleEndian;
    
    if(strncmp(idString, "Octave-1-L", idStringLen) == 0)
        littleEndian = true;
    else if(strncmp(idString, "Octave-1-B", idStringLen) == 0)
    {
        littleEndian = false;
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
                (*v)[i] = (btScalar)value;
            }
        }
        else //double
        {
            for(uint32_t i = 0; i < vLen; ++i)
            {
                double value;
                file.read(reinterpret_cast<char*>(&value), 8);
                (*v)[i] = (btScalar)value;
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
                    m->setElem(h, i, (btScalar)value);
                }
        }
        else //double
        {
            for(uint32_t i = 0; i < cols; ++i)
                for (uint32_t h = 0; h < rows; ++h)
                {
                    double value;
                    file.read(reinterpret_cast<char*>(&value), 8);
                    m->setElem(h, i, (btScalar)value);
                }
        }
        
        it->value = m;
    }
    
    return true;
}