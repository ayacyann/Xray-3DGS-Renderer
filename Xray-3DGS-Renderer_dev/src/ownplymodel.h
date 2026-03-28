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
    bool gamma_correction;
    unsigned int VAO;
    int vertex_count;
    int success;

    Ownplymodel(){}

    Ownplymodel(string const& path, bool gamma = false) : loadpath(path), gamma_correction(gamma)
    {
        loadModel(path);
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
        glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, vertex_count);
        glBindVertexArray(0);
    }
private:
    int loadModel(string const& path)
    {
        Plyformat_parser parser(path);
        if (!parser.success)
        {
            cout << "parser解析header错误" << endl;

            return this->success = 0;
        }
        Plyexporter exporter(path);
        this->VAO = exporter.VAO;
        this->vertex_count = exporter.vertex_count;
        if (!exporter.success)
        {
            cout << "exporter导出顶点数据错误" << endl;
            return this->success = 0;
        }

        return this->success = 1;
    }
};