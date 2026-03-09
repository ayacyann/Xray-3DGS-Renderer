#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

struct Ply_Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // density
    glm::vec1 Density;
    // mark
    glm::vec1 Mark;
    // scale
    glm::vec3 Scale;
    // rot
    glm::vec4 Rot;
};

class Plyexporter
{
public:
    int success;
    int vertex_count;
	vector<Ply_Vertex> vertices;
    unsigned int VAO;

	Plyexporter(string const& path)
	{
		load_to_opgl(path);
	}
private:
	int load_to_opgl(string const& path)
	{
        load_vertices(path);
        setup_opgl_vertices(vertices);
        return this->success = 1;
	}

    int load_vertices(string const& path)
    {
        ifstream binfile(path, ios::binary);
        if (!binfile.is_open()) {
            cerr << "文件打开失败！" << endl;
            return this->success = 0;
        }
        string line;
        int line_num = 0;
        while (line != "end_header")
        {
            getline(binfile, line);
            if (line.find("element vertex") != std::string::npos) 
            {
                sscanf_s(line.c_str(), "element vertex %d", &vertex_count);
                cout << "vertex_count = " << vertex_count << endl;
            }
            line_num++;
        }

        vertices.reserve(vertex_count * sizeof(Ply_Vertex));
        Ply_Vertex temp_vertex;
        for (int i = 0; i < vertex_count; ++i)
        {
            // 读取二进制数据（直接按结构体大小读取）
            binfile.read(reinterpret_cast<char*>(&temp_vertex), sizeof(Ply_Vertex));
            if (!binfile) {
                std::cerr << "错误：读取顶点数据时文件异常！" << std::endl;
                binfile.close();
                return this->success = 0;
            }
            // 存入vertices数组
            vertices.push_back(temp_vertex);
        }

        binfile.close();
        std::cout << "成功读取PLY文件：" << vertex_count << "个顶点" << std::endl;
        return true;
    }

    void setup_opgl_vertices(const vector<Ply_Vertex>& vertices)
    {
        unsigned int VBO;

        // 生成VAO和VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // 绑定VAO
        glBindVertexArray(VAO);

        // 绑定VBO并传入数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // 配置顶点属性（位置属性，索引0，3个float，步长15*sizeof(float)）
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // 配置顶点属性（位置属性，索引1，3个float，步长15*sizeof(float)）
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // 配置顶点属性（位置属性，索引2，1个float，步长15*sizeof(float)）
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        // 配置顶点属性（位置属性，索引3，1个float，步长15*sizeof(float)）
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(7 * sizeof(float)));
        glEnableVertexAttribArray(3);
        // 配置顶点属性（位置属性，索引4，3个float，步长15*sizeof(float)）
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        // 配置顶点属性（位置属性，索引5，4个float，步长15*sizeof(float)）
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(11 * sizeof(float)));
        glEnableVertexAttribArray(5);

    }

};