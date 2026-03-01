#pragma once

#include "shader.h"
#include "plyformat_parser.h"
#include "plyexporter.h"

#include <string>
#include <iostream>

using namespace std;

class Ownplymodel
{
public:
    string loadpath;  
    bool gammaCorrection;
    Ownplymodel(string const& path, bool gamma = false) : loadpath(path), gammaCorrection(gamma)
    {
        int success = loadModel(path);
        if (success)
        {
            cout << "loadmodel successfully at ::" << loadpath << endl;
        }
        else
        {
            cout << "fail to loadmodel at ::" << loadpath << endl;
        }
    }
    void Draw(Shader& shader)
    {
        
    }
private:
    int loadModel(string const& path)
    {
        Plyformat_parser parser(path);
        if (!parser.success)
        {
            return 0;
        }
        Plyexporter exporter(path);
    }
};