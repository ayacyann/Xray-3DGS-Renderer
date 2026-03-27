#include "imgui_own_render_code.h"

#include "imgui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace imgui_own_render_code
{
    const unsigned int FBO_WIDTH = 512;
    const unsigned int FBO_HEIGHT = 512;

    // 帧缓冲(FBO)相关
    unsigned int fbo;          // 帧缓冲对象
    unsigned int textureColor; // FBO颜色附件（纹理）
    unsigned int rbo;          // 渲染缓冲对象（深度/模板）

    // 初始化帧缓冲 FBO
    void create_framebuffer()
    {
        // 1. 创建帧缓冲
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // 2. 创建纹理附件（颜色）
        glGenTextures(1, &textureColor);
        glBindTexture(GL_TEXTURE_2D, textureColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FBO_WIDTH, FBO_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 把纹理附加到帧缓冲
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColor, 0);

        // 3. 创建渲染缓冲（深度/模板）
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, FBO_WIDTH, FBO_HEIGHT);
        // 附加到帧缓冲
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        // 检查帧缓冲是否完整
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

        // 解绑
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

 

	void render_ui()
	{
        ImGui::SetNextWindowSize(ImVec2(FBO_WIDTH + 15, FBO_HEIGHT + 30));

        ImGui::Begin("model render", nullptr,
            ImGuiWindowFlags_NoResize |    // 禁止调整大小
            ImGuiWindowFlags_NoScrollbar); // 不需要滚动
      
        // 关键代码：在 ImGui 中显示 FBO 纹理
        // 参数：纹理ID、显示尺寸、UV(0,1) 是因为 OpenGL 纹理上下颠倒
        ImGui::Image(
            (ImTextureID)textureColor,
            ImVec2(FBO_WIDTH, FBO_HEIGHT),
            ImVec2(0, 1), ImVec2(1, 0)
        );

		ImGui::End();
	}
}