#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace imgui_own_render_code
{
	void create_framebuffer();

	void create_shader_and_model();

	void render_scene();

	void render_ui();

	void process_input(GLFWwindow* window);

	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
}