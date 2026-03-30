#include "imgui_own_render_code.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "shader.h"
#include "camera.h"
#include "model.h"
#include "ownplymodel.h"

#include <iostream>

namespace imgui_own_render_code
{
    int m_renderWidth = 512;
    int m_renderHeight = 512;

    unsigned int m_fbo;          // 帧缓冲对象
    unsigned int m_fboTexture; // FBO颜色附件（纹理）


    // 初始化帧缓冲 FBO
    void create_framebuffer()
    {
        // 1. 释放旧资源
        if (m_fboTexture) glDeleteTextures(1, &m_fboTexture);
        if (m_fbo) glDeleteFramebuffers(1, &m_fbo);

        // 2. 创建纹理
        glGenTextures(1, &m_fboTexture);
        glBindTexture(GL_TEXTURE_2D, m_fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_renderWidth, m_renderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // 纹理参数（ImGui显示必须设置）
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 3. 创建FBO
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTexture, 0);

        // 检查FBO是否完整
        bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }




    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    //Camera camera(0.0f, 1.0f, 0.001f, 1.5f, 256, 1);


    Shader shader;
    Ownplymodel ourModel;
    void create_shader_and_model()
    {
        // 创建shader程序
        // ---------------
        shader = Shader("../shader/shader.vert", "../shader/shader.frag");

        // 加载模型
        // ----------------
        ourModel = Ownplymodel("../resources/model/.ply/Lingo/foot.ply");
    }



    extern const unsigned int SCR_WIDTH = 1280;
    extern const unsigned int SCR_HEIGHT = 720;
    float deltaTime = 0.0f; // 当前帧与上一帧的时间差
    float lastFrame = 0.0f; // 上一帧的时间
    float exposure = 2.0f;

    void render_scene()
    {
        // 帧时间差
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("MVP", projection * view * model);
        shader.setVec3("cameraPos", camera.Position);
        shader.setFloat("exposure", exposure);

        ourModel.Draw(shader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }




	void render_ui()
	{
        ImGui::Begin("model render");
        
        if (m_fboTexture != 0)
        {
            ImGui::Image((ImTextureID)(intptr_t)m_fboTexture, 
                ImVec2(m_renderWidth, m_renderHeight), 
                ImVec2(0, 1), ImVec2(1, 0));
        }

		ImGui::End();
	}



    float oriMovementSpeed = 3.0f;
    float accMovementSpeed = 5.0f;

    bool firstMouse = true;
    bool ifmouse = 0;
    bool key_pressed[GLFW_KEY_LAST] = { false };

    // 键盘输入
    // --------------
    void process_input(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !key_pressed[GLFW_KEY_TAB])
        {
            key_pressed[GLFW_KEY_TAB] = true;
            ifmouse = !ifmouse;
            if (ifmouse)
            {
                firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // 鼠标
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 鼠标停留在窗口中                                     
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
        {
            key_pressed[GLFW_KEY_TAB] = false;
        }


        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.MovementSpeed = accMovementSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
            camera.MovementSpeed = oriMovementSpeed;
    }



    // 鼠标滚轮回调
    // --------------
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        //camera.ProcessMouseScroll(static_cast<float>(yoffset));
        if (yoffset > 0)
            exposure += 0.25f;
        else
            exposure -= 0.25f;
    }



    float lastX = SCR_WIDTH / 2.0f;
    float lastY = SCR_HEIGHT / 2.0f;
  
    // 鼠标移动回调
    // --------------
    void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
    {
        float xpos = 0;
        float ypos = 0;

        if (!ifmouse)
        {
            xpos = static_cast<float>(xposIn);
            ypos = static_cast<float>(yposIn);
        }
        else
        {
            return;
        }

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }


    // 调整窗口大小回调函数
    // --------------
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }
}