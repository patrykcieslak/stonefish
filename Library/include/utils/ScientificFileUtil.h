//
//  ScientificFileUtil.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ScientificFileUtil__
#define __Stonefish_ScientificFileUtil__

#include <LinearMath/btMatrixX.h>
#include "common.h"

namespace sf
{

typedef enum {DATA_SCALAR, DATA_VECTOR, DATA_MATRIX} ScientificDataType;

struct ScientificDataItem
{
    std::string name;
    ScientificDataType type;
    void* value;
    
    ScientificDataItem() : value(NULL) {}
    
    ~ScientificDataItem()
    {
        if(value != NULL)
        {
            switch(type)
            {
                case DATA_SCALAR:
                    delete ((btScalar*)value);
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

class ScientificData
{
public:
    ScientificData(std::string filepath);
    virtual ~ScientificData();
    
    void addItem(ScientificDataItem* it);
    const ScientificDataItem* getItem(std::string name) const;
    const ScientificDataItem* getItem(unsigned int index) const;
    unsigned int getItemsCount() const;
    btScalar getScalar(std::string name);
    btVectorXu getVector(std::string name);
    btMatrixXu getMatrix(std::string name);

private:
    std::string path;
    std::vector<ScientificDataItem*> items;
};

ScientificData* LoadOctaveData(const char* path);
bool LoadOctaveScalar(std::ifstream& file, ScientificDataItem* it, bool isFloat = false);
bool LoadOctaveMatrix(std::ifstream& file, ScientificDataItem* it, bool isFloat = false);

bool SaveOctaveData(const char* path, const ScientificData& data);
void SaveOctaveScalar(std::ofstream& file, const ScientificDataItem& it);
void SaveOctaveMatrix(std::ofstream& file, const ScientificDataItem& it);
    
}

#endif
