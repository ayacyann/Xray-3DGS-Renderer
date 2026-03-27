#pragma once

namespace imgui_own_render_code
{
	// 帧缓冲(FBO)相关
	extern unsigned int fbo;          // 帧缓冲对象
	extern unsigned int textureColor; // FBO颜色附件（纹理）
	extern unsigned int rbo;          // 渲染缓冲对象（深度/模板）

	void create_framebuffer();

	void render_ui();
}