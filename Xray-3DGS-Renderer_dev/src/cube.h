#include <glad/glad.h>  // 确保你配置了 GLAD
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>

// --- 2. Cube 类定义 ---
class Cube {
private:
    unsigned int VAO, VBO, EBO;
    unsigned int shaderProgram;
    Shader shader;

    // 立方体顶点数据 (位置 + 颜色)
    // 8个顶点，每个顶点有 x,y,z 和 r,g,b
    float vertices[8 * 3] = {
        // 位置 (x, y, z)       
        -0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
        -0.5f,  0.5f, -0.5f,  
        -0.5f, -0.5f,  0.5f,  
         0.5f, -0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f,  0.5f,  
    };

    // 索引数据 (定义 6 个面，每个面 2 个三角形)
    unsigned int indices[36] = {
        0, 1, 2,  2, 3, 0, // 后面
        4, 5, 6,  6, 7, 4, // 前面
        0, 4, 7,  7, 3, 0, // 左面
        1, 5, 6,  6, 2, 1, // 右面
        3, 7, 6,  6, 2, 3, // 上面
        0, 1, 5,  5, 4, 0  // 下面
    };

    void setupMesh() {
        // 生成 VAO, VBO, EBO
        glGenVertexArrays(2, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // 填充顶点数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 填充索引数据
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 位置属性 (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0); // 解绑
    }

public:
    Cube(Shader sh) {
        shader = sh;
        setupMesh();
    }

    Cube() {

    }

    // --- 核心绘制函数 ---
    // 传入 model, view, projection 矩阵
    void Draw() {
        // 绘制
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // 36 个索引
        glBindVertexArray(0);
    }
};