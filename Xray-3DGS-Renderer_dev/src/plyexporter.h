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
        // 1. 立方体顶点和索引
        float cubeVertices[] = {
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
        };
        unsigned int cubeIndices[] = {
            0,1,2, 2,3,0,
            4,5,6, 6,7,4,
            0,1,5, 5,4,0,
            2,3,7, 7,6,2,
            0,3,7, 7,4,0,
            1,2,6, 6,5,1
        };

        // 1. 创建VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // 2. 立方体顶点VBO
        unsigned int cubeVBO;
        glGenBuffers(1, &cubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        // 顶点属性0：立方体顶点
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // 3. 立方体索引EBO
        unsigned int cubeEBO;
        glGenBuffers(1, &cubeEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

        // 4. 实例VBO
        unsigned int VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Ply_Vertex), vertices.data(), GL_STATIC_DRAW);

        // 配置顶点属性（位置属性，索引0，3个float，步长15*sizeof(float)）
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Ply_Vertex), (void*)(offsetof(Ply_Vertex, Position)));
        glEnableVertexAttribArray(1);
        // 每个实例更新一次属性
		glVertexAttribDivisor(1, 1); 
        // 配置顶点属性（法线属性，索引1，3个float，步长15*sizeof(float)）
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Ply_Vertex), (void*)(offsetof(Ply_Vertex, Normal)));
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);
        // 配置顶点属性（密度属性，索引2，1个float，步长15*sizeof(float)）
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Ply_Vertex), (void*)(offsetof(Ply_Vertex, Density)));
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);
        // 配置顶点属性（初始化点云蒙版属性，索引3，1个float，步长15*sizeof(float)）
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Ply_Vertex), (void*)(offsetof(Ply_Vertex, Mark)));
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4, 1);
        // 配置顶点属性（缩放属性，索引4，3个float，步长15*sizeof(float)）
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Ply_Vertex), (void*)(offsetof(Ply_Vertex, Scale)));
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1);
        // 配置顶点属性（旋转属性，索引5，4个float，步长15*sizeof(float)）
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Ply_Vertex), (void*)(offsetof(Ply_Vertex, Rot)));
        glEnableVertexAttribArray(6);
        glVertexAttribDivisor(6, 1);

        // 解绑
        glBindVertexArray(0);
    }

};