#include "imgui_own_render_code.h"

#include <imgui_internal.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "ImGuiFileDialogConfig.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "shader.h"
#include "camera.h"
#include "model.h"
#include "ownplymodel.h"
#include "cube.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION 
#include "stb_image_write.h"

#include <iostream>

namespace imgui_own_render_code
{
    enum Axis { X, Y, Z };

    int m_renderWidth = 512;
    int m_renderHeight = 512;

    unsigned int m_fbo;          // 帧缓冲对象
    unsigned int m_fboTexture; // FBO颜色附件（纹理）

    unsigned int m_ct_fbo;
	unsigned int m_ct_fboTexture;

    extern bool g_isModelRenderWindowActiveForMouse = false;
	extern bool g_isCTRenderWindowActiveForMouse = false;
    extern bool g_isFirstMouseInModelRenderWindow = true;
    extern double g_lastX = 0.0;
    extern double g_lastY = 0.0;
    extern bool isFocusMoving = false;

    float deltaTime = 0.0f; // 当前帧与上一帧的时间差
    float lastFrame = 0.0f; // 上一帧的时间
    static float exposure = 2.0f;
	Axis currentAxis = Y;
	static bool isShowCameraPlane = true;
    static float positionCT = 0.0;
	static int voxelResulution = 64;
    static float exposureCT = 5.0;
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

    struct CTCameraData {
        glm::vec3 position;
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    CTCameraData GetCTCameraData(Axis axis, float position, float thickness) {
        CTCameraData data;

        float lim = 1.3;
        float times = 1.5;
        glm::vec3 eye, center, up, dir, cubePos;
        center = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::mat4 model = glm::mat4(1.0f);
        // 1. 构建视图矩阵 (View Matrix)
        // 逻辑：相机在轴向上位于 'position'，看向原点 (0,0,0)
        switch (axis) {
        case Axis::X:
            eye = glm::vec3(position * lim * times, 0.0f, 0.0f);
			cubePos = glm::vec3(position / 2 / thickness * lim, 0.0f, 0.0f);
			dir = glm::vec3(-5.0f, 0.0f, 0.0f); 
            up = glm::vec3(0.0f, 1.0f, 0.0f); // X轴视角，Y轴朝上
			model = glm::scale(model, glm::vec3(thickness * times, lim * times, lim * times));
            break;
        case Axis::Y:
            eye = glm::vec3(0.0f, position * lim * times, 0.0f);
			cubePos = glm::vec3(0.0f, position / 2 / thickness * lim, 0.0f);
			dir = glm::vec3(0.0f, -5.0f, 0.0f); 
            up = glm::vec3(0.0f, 0.0f, 1.0f); // Y轴视角，Z轴朝上 (或者Y轴朝上，看你的模型朝向)
			model = glm::scale(model, glm::vec3(lim * times, thickness * times, lim * times));
            break;
        case Axis::Z:
            eye = glm::vec3(0.0f, 0.0f, position * lim * times);
			cubePos = glm::vec3(0.0f, 0.0f, position / 2 / thickness * lim);
			dir = glm::vec3(0.0f, 0.0f, -5.0f);
            up = glm::vec3(0.0f, 1.0f, 0.0f); // Z轴视角，Y轴朝上
			model = glm::scale(model, glm::vec3(lim * times, lim * times, thickness * times));
            break;
        }
        model = glm::translate(model, cubePos);
        data.model = model;

        // 使用 lookAt 生成视图矩阵
        // 注意：GLM 默认右手坐标系，看向 -Z。
        // 如果是 X 轴视角，相机在 +X，看向 0，方向是 -X，符合逻辑。
        data.view = glm::lookAt(eye, dir, up);
        data.position = cubePos;

        // 2. 构建投影矩阵 (Projection Matrix)
        // 场景范围是 -1 到 1，所以左右、上下边界通常固定为 -1 和 1 (或根据宽高比调整)
        float left = -lim;
        float right = lim;
        float bottom = -lim;
        float top = lim;

        // 计算近平面和远平面
        // 逻辑：position 是切片中心，thickness 是总厚度
        float half_thickness = thickness * 0.5f;
        float nearPlane = position - half_thickness;
        float farPlane = position + half_thickness;

        data.projection = glm::ortho(left, right, bottom, top, nearPlane, farPlane);

        return data;
    }

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

        // 1. 释放旧资源
        if (m_ct_fboTexture) glDeleteTextures(1, &m_ct_fboTexture);
        if (m_ct_fbo) glDeleteFramebuffers(1, &m_ct_fbo);

        // 2. 创建纹理
        glGenTextures(1, &m_ct_fboTexture);
        glBindTexture(GL_TEXTURE_2D, m_ct_fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_renderWidth, m_renderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // 纹理参数（ImGui显示必须设置）
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 3. 创建FBO
        glGenFramebuffers(1, &m_ct_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_ct_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ct_fboTexture, 0);

        // 检查FBO是否完整
        success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    // aneurism
    //Camera camera(glm::vec2(SCR_WIDTH, SCR_HEIGHT), glm::vec2(512), glm::vec2(1.0), 1500.0, glm::vec3(256.0), glm::vec3(1.0));
    // chest

    Shader shader;
    Shader cameraShader;
    Ownplymodel ourModel;
    Cube cameraPlane;
    string vert_shader_path = "../shader/shader.vert";
    string frag_shader_path = "../shader/shader.frag";
    string model_path = "../resources/model/.ply/Lingo/foot.ply";
	string params_path = "../resources/model/.ply/foot.json";
    Camera camera(params_path);

    void update_shader()
    {
        shader = Shader(vert_shader_path.c_str(), frag_shader_path.c_str());
    }

	void update_model()
	{
		ourModel = Ownplymodel(model_path);
	}

    void create_shader_and_model()
    {
        cout << vert_shader_path << endl;
        cout << frag_shader_path << endl;
        cout << model_path << endl;

        // 创建shader程序
        // ---------------
        shader = Shader(vert_shader_path.c_str(), frag_shader_path.c_str());

        // 加载模型
        // ----------------
        ourModel = Ownplymodel(model_path);

        cout << "create_shader_and_model" << endl;

        cameraShader = Shader("../shader/cube.vert", "../shader/cube.frag");

        cameraPlane = Cube(cameraShader);
    }

    void render_scene()
    {
        CTCameraData data = GetCTCameraData(currentAxis, positionCT, 1.0 / float(voxelResulution));

		glViewport(0, 0, m_renderWidth, m_renderHeight);
        // 帧时间差
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

# pragma region 绘制XRay 窗口
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(clear_color.x,clear_color.y,clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)m_renderWidth / (float)m_renderHeight, 0.1f, 100.0f);

        if (isShowCameraPlane) {
            cameraShader.use();

            cameraShader.setMat4("model", data.model);
            cameraShader.setMat4("view", view);
            cameraShader.setMat4("projection", projection);
            cameraShader.setVec3("outColor", glm::vec3(0.05, 0.01, 0.01));

            cameraPlane.Draw();
        }

        shader.use();
        shader.setMat4("MVP", projection * view);
        shader.setVec2("tanFov", glm::tan(camera.Fov / 2.0f));
        shader.setVec2("focal", camera.Focal);
        shader.setMat4("viewMatrix", view);
        shader.setVec3("cameraPos", camera.Position);
        shader.setFloat("exposure", exposure);
        shader.setVec2("screenSize", glm::vec2(m_renderWidth, m_renderHeight));

        ourModel.Draw(shader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
# pragma endregion

# pragma region 绘制CT 窗口
        glBindFramebuffer(GL_FRAMEBUFFER, m_ct_fbo);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        shader.setMat4("MVP", data.projection * data.view);
        shader.setVec2("tanFov", glm::vec2(0));
        shader.setVec2("focal", camera.Focal);
        shader.setMat4("viewMatrix", data.view);
        shader.setVec3("cameraPos", data.position);
        shader.setFloat("exposure", exposureCT);
        shader.setVec2("screenSize", glm::vec2(m_renderWidth, m_renderHeight));

        ourModel.Draw(shader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
# pragma endregion
    }

    bool show_vert_dialog = false;
    bool show_frag_dialog = false;
    bool show_model_dialog = false;
	bool show_save_dialog = false;
	bool is_save_xray = true;
	bool show_params_dialog = false;
    std::vector<unsigned char> pixels(m_renderWidth * m_renderHeight * 3);
    static void choose_path(string& buttonLabel)
    {
        
    }

	void render_ui(GLFWwindow* window)
	{
        ImGuiIO& io = ImGui::GetIO();

        ImGui::Begin("XRay Imaging");

        if (m_fboTexture != 0)
        {
            // 1. 获取当前窗口可用的最大绘图区域 (减去 padding 和滚动条后的区域)
            ImVec2 availSize = ImGui::GetContentRegionAvail();

            // 2. 计算正方形边长：取宽和高的最小值
            float dim = ImMin(availSize.x, availSize.y);

            // 3. 计算居中偏移量
            // 如果窗口很宽，availSize.x > dim，需要向右偏移
            // 如果窗口很高，availSize.y > dim，需要向下偏移
            ImVec2 cursorPos = ImGui::GetCursorPos();
            cursorPos.x += (availSize.x - dim) * 0.5f;
            cursorPos.y += (availSize.y - dim) * 0.5f;

            // 4. 设置光标位置到计算好的居中点
            ImGui::SetCursorPos(cursorPos);

            ImGui::Image((ImTextureID)(intptr_t)m_fboTexture, 
                ImVec2(dim, dim),
                ImVec2(0, 1), ImVec2(1, 0));

            bool img_hovered = ImGui::IsItemHovered();
            bool mouse_clicked = ImGui::IsAnyMouseDown();

            double currentX, currentY;
            glfwGetCursorPos(window, &currentX, &currentY);

            if (g_isFirstMouseInModelRenderWindow)
            {
                g_lastX = currentX;
                g_lastY = currentY;
                // 防止下一帧再次计算
                g_isFirstMouseInModelRenderWindow = false;
            }
            float xOffset = currentX - g_lastX;
            float yOffset = g_lastY - currentY;

            g_lastX = currentX;
            g_lastY = currentY;

            if (img_hovered && mouse_clicked)
            {
                // 当鼠标在Image上点击时，激活当前窗口状态
                g_isModelRenderWindowActiveForMouse = true;
                g_isFirstMouseInModelRenderWindow = true;
            }
            else if (g_isModelRenderWindowActiveForMouse && !isFocusMoving)
            {
                // 取消当前的激活状态
                bool clicked_outside = ImGui::IsAnyMouseDown() && !img_hovered;
                if (io.WantCaptureMouse && clicked_outside)
                {
                    g_isModelRenderWindowActiveForMouse = false;
                    g_isFirstMouseInModelRenderWindow = true;
                }
            }
        }

        // 获取当前窗口的内容区域大小（用来计算下一个窗口的位置）
        ImVec2 window1Size = ImGui::GetWindowSize();
        ImVec2 window1Pos = ImGui::GetWindowPos();

        ImGui::End();

        // 设置下一个窗口的位置 = 窗口1的位置 + 窗口1的宽度 + 一点间距(10像素)
        ImVec2 window2Pos = ImVec2(window1Pos.x + window1Size.x + 10.0f, window1Pos.y);
        ImGui::SetNextWindowPos(window2Pos, ImGuiCond_FirstUseEver); // 仅在首次使用时设置位置

        ImGui::Begin("CT Imaging");

		if (m_ct_fboTexture != 0)
		{
            // 1. 获取当前窗口可用的最大绘图区域 (减去 padding 和滚动条后的区域)
            ImVec2 availSize = ImGui::GetContentRegionAvail();

            // 2. 计算正方形边长：取宽和高的最小值
            float dim = ImMin(availSize.x, availSize.y);

            // 3. 计算居中偏移量
            // 如果窗口很宽，availSize.x > dim，需要向右偏移
            // 如果窗口很高，availSize.y > dim，需要向下偏移
            ImVec2 cursorPos = ImGui::GetCursorPos();
            cursorPos.x += (availSize.x - dim) * 0.5f;
            cursorPos.y += (availSize.y - dim) * 0.5f;

            // 4. 设置光标位置到计算好的居中点
            ImGui::SetCursorPos(cursorPos);

			ImGui::Image((ImTextureID)(intptr_t)m_ct_fboTexture,
				ImVec2(dim, dim),
				ImVec2(0, 1), ImVec2(1, 0));

            bool img_hovered = ImGui::IsItemHovered();
            bool mouse_clicked = ImGui::IsAnyMouseDown();

            if (img_hovered && mouse_clicked)
            {
                // 当鼠标在Image上点击时，激活当前窗口状态
                g_isCTRenderWindowActiveForMouse = true;
            }
            else if (g_isCTRenderWindowActiveForMouse)
            {
                // 取消当前的激活状态
                bool clicked_outside = ImGui::IsAnyMouseDown() && !img_hovered;
                if (io.WantCaptureMouse && clicked_outside)
                {
                    g_isCTRenderWindowActiveForMouse = false;
                }
            }
		}

		ImGui::End();

        {
            ImGui::Begin("Settings");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Imaging Setting");               // Display some text (you can use a format strings too)
            ImGui::Text("CT Axis Selection");
            ImGui::SameLine();
			if (ImGui::RadioButton("X Axis", currentAxis == X)) {
				currentAxis = X;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Y Axis", currentAxis == Y)) {
				currentAxis = Y;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Z Axis", currentAxis == Z)) {
				currentAxis = Z;
			}
            ImGui::SameLine(0.0,100.0);
            ImGui::Text("Save Image");
            ImGui::SameLine();
            // --- 按钮 1: 保存主视口 (m_fboTexture) ---
            if (ImGui::Button("Save Xray")) {
                is_save_xray = true;
                // 1. 绑定 FBO 读取数据
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

                // 3. 读取像素 (从显存读回 CPU 内存)
                for (int y = 0; y < m_renderHeight; ++y) {
                    glPixelStorei(GL_PACK_SKIP_ROWS, m_renderHeight - 1 - y);
                    glReadPixels(0, y, m_renderWidth, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
                }
                // 4. 恢复默认设置
                glPixelStorei(GL_PACK_ROW_LENGTH, 0);
                glPixelStorei(GL_PACK_SKIP_ROWS, 0);

                // 5. 解绑
                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                show_save_dialog = true;
            }
            ImGui::SameLine(); 
            // --- 按钮 2: 保存 CT 视口 (m_ct_fboTexture) ---
            if (ImGui::Button("Save CT")) {
                is_save_xray = false;
                // 1. 绑定 CT 的 FBO
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_ct_fbo);

                // 3. 读取像素 (从显存读回 CPU 内存)
                for (int y = 0; y < m_renderHeight; ++y) {
                    glPixelStorei(GL_PACK_SKIP_ROWS, m_renderHeight - 1 - y);
                    glReadPixels(0, y, m_renderWidth, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
                }
                // 4. 恢复默认设置
                glPixelStorei(GL_PACK_ROW_LENGTH, 0);
                glPixelStorei(GL_PACK_SKIP_ROWS, 0);

                // 5. 解绑
                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                show_save_dialog = true;
            }
            ImGui::Checkbox("Show Camera Cube", &isShowCameraPlane);
			ImGui::SameLine();
            ImGui::SliderFloat("Position Slice", &positionCT, -1.0f, 1.0f);
			ImGui::SliderFloat("Exposure Xray", &exposure, 1.0f, 10.0f);            
			ImGui::SliderFloat("Exposure CT", &exposureCT, 1.0f, 30.0f);            
			ImGui::SliderInt("Voxel Resolution", &voxelResulution, 16, 512);            
            ImGui::ColorEdit3("Background Color", (float*)&clear_color); 

            if (show_save_dialog)
            {
                IGFD::FileDialogConfig config;
                config.path = "."; // 初始路径
                if (is_save_xray)
                    config.fileName = "XRay.png";
                else
                    config.fileName = "CT.png";
                ImGuiFileDialog::Instance()->OpenDialog(
                    "ChooseFileDlgKey",         // 对话框key
                    "Choose File",              // 标题
                    ".png",         // 过滤器
                    config                      // 用config结构体传参
                );
                // 显示对话框
                if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
                {
                    // 用户点击OK
                    if (ImGuiFileDialog::Instance()->IsOk())
                    {
                        // 获取选中的文件路径
                        std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                        // 在这里处理文件
                        vert_shader_path = file_path;
                        printf("选择文件: %s\n", file_path.c_str());
                        // 写入文件
                        // 参数：文件名, 宽, 高, 通道数(3), 数据指针, 行步长(0表示紧密排列)
                        
                        if (stbi_write_jpg(file_path.c_str(), m_renderWidth, m_renderHeight, 3, pixels.data(), 0)) {
                            std::cout << "Image Saved!" << std::endl;
                        }
                    }
                    // 关闭对话框
                    ImGuiFileDialog::Instance()->Close();
                    show_save_dialog = false;
                }
            }

            //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //    counter++;
            //ImGui::SameLine();
            //ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        ImGui::Begin("Model Settings");

        if (ImGui::Button("vert_shader_path"))
        {
            show_vert_dialog = true;
            IGFD::FileDialogConfig config;
            config.path = "."; // 初始路径
			config.filePathName = vert_shader_path;

            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseFileDlgKey",         // 对话框key
                "Choose File",              // 标题
                ".*,.vert",         // 过滤器
                config                      // 用config结构体传参
            );           
        }
        if (show_vert_dialog)
        {
            // 显示对话框
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
            {
                // 用户点击OK
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    // 获取选中的文件路径
                    std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                    // 在这里处理文件
                    vert_shader_path = file_path;
                    printf("选择文件: %s\n", file_path.c_str());
                }
                update_shader();
                // 关闭对话框
                ImGuiFileDialog::Instance()->Close();
                show_vert_dialog = false;
            }
        }
        ImGui::Text(vert_shader_path.c_str());

        if (ImGui::Button("frag_shader_path"))
        {
            show_frag_dialog = true;
            IGFD::FileDialogConfig config;
            config.path = "."; // 初始路径
			config.filePathName = frag_shader_path;

            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseFileDlgKey",         // 对话框key
                "Choose File",              // 标题
                ".*,.frag",         // 过滤器
                config                      // 用config结构体传参
            );           
        }
        if (show_frag_dialog)
        {
            // 显示对话框
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
            {
                // 用户点击OK
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    // 获取选中的文件路径
                    std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                    // 在这里处理文件
                    frag_shader_path = file_path;
                    printf("选择文件: %s\n", file_path.c_str());
                }
                update_shader();
                // 关闭对话框
                ImGuiFileDialog::Instance()->Close();
                show_frag_dialog = false;
            }
        }
        ImGui::Text(frag_shader_path.c_str());

        if (ImGui::Button("model_path"))
        {
            show_model_dialog = true;
            IGFD::FileDialogConfig config;
            config.path = "../resources/model/.ply"; // 初始路径
            config.filePathName = model_path;

            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseFileDlgKey",         // 对话框key
                "Choose File",              // 标题
                ".ply",         // 过滤器
                config                      // 用config结构体传参
            );           
        }
        if (show_model_dialog)
        {
            // 显示对话框
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
            {
                // 用户点击OK
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    // 获取选中的文件路径
                    std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                    // 在这里处理文件
                    model_path = file_path;
                    printf("选择文件: %s\n", file_path.c_str());
                }
				update_model();
                // 关闭对话框
                ImGuiFileDialog::Instance()->Close();
                show_model_dialog = false;
            }
        }
        ImGui::Text(model_path.c_str());

        if (ImGui::Button("params_path"))
        {
            show_params_dialog = true;
            IGFD::FileDialogConfig config;
            config.path = "../resources/model/.ply"; // 初始路径
            config.filePathName = params_path;

            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseFileDlgKey",         // 对话框key
                "Choose File",              // 标题
                ".json",         // 过滤器
                config                      // 用config结构体传参
            );
        }
        if (show_params_dialog)
        {
            // 显示对话框
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
            {
                // 用户点击OK
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    // 获取选中的文件路径
                    std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                    // 在这里处理文件
                    params_path = file_path;
                    printf("选择文件: %s\n", file_path.c_str());
                }
				camera.UpdateCameraParameters(params_path);
                // 关闭对话框
                ImGuiFileDialog::Instance()->Close();
                show_params_dialog = false;
            }
        }
        ImGui::Text(params_path.c_str());

        ImGui::End();     
	}

    float oriMovementSpeed = 3.0f;
    float accMovementSpeed = 5.0f;

    bool key_pressed[GLFW_KEY_LAST] = { false };

    // 键盘输入
    // --------------
    void process_input(GLFWwindow* window)
    {
        if (g_isModelRenderWindowActiveForMouse)
        {
            if (isFocusMoving)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.WantCaptureKeyboard = false;
                io.WantCaptureMouse = false;
                io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange; // 确保没有禁用光标变化
                io.MouseDrawCursor = false; // ImGui 不绘制光标
            }
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

            if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !key_pressed[GLFW_KEY_TAB])
            {
                key_pressed[GLFW_KEY_TAB] = true;
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
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            {
                isFocusMoving = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
            {
                isFocusMoving = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    // 鼠标滚轮回调
    // --------------
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        if (g_isModelRenderWindowActiveForMouse)
        {        
            //camera.ProcessMouseScroll(static_cast<float>(yoffset));
            if (yoffset > 0)
                exposure += 0.1f;
            else
                exposure -= 0.1f;
        }
        if (g_isCTRenderWindowActiveForMouse)
        {
            //camera.ProcessMouseScroll(static_cast<float>(yoffset));
            if (yoffset > 0)
                exposureCT += 1.0f;
            else
                exposureCT -= 1.0f;
        }
    }
  
    // 鼠标移动回调
    // --------------
    void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
    {
        if (!g_isModelRenderWindowActiveForMouse || !isFocusMoving)
        {
            return;
        }

        float xoffset = xposIn - g_lastX;
        float yoffset = g_lastY - yposIn;
        g_lastX = xposIn;
        g_lastY = yposIn;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }

    // 调整窗口大小回调函数
    // --------------
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        //glViewport(0, 0, width, height);
    }
}