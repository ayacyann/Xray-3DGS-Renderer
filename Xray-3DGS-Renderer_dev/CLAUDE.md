# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Xray-3DGS-Renderer 是一个基于 3D Gaussian Splatting 的 X 射线渲染器，使用 OpenGL 和 ImGui 构建交互式可视化界面。项目从 PLY 格式文件加载 3DGS 数据（包含位置、法线、密度、缩放、旋转等属性），通过几何着色器实例化渲染高斯椭球体，模拟 X 射线投影成像。

## Build Commands

项目使用 Visual Studio 2022 (v143 toolset) 和 Windows SDK 10.0 构建：

- **打开项目**: 在 Visual Studio 中打开 `Xray-3DGS-Renderer_dev.sln`
- **构建配置**: 主要使用 `Debug|x64` 配置进行开发
- **构建**: 在 Visual Studio 中按 `Ctrl+Shift+B` 或使用菜单 Build → Build Solution
- **运行**: 按 `F5` 启动调试，或 `Ctrl+F5` 无调试运行

项目依赖以下库（已包含在 `includes/` 和 `lib/` 目录）：
- GLFW 3.x (窗口管理)
- GLAD (OpenGL 加载器)
- GLM (数学库)
- Assimp (模型加载，链接 `assimp-vc142-mt.lib`)
- ImGui 1.92.6-docking (UI 框架)
- ImGuiFileDialog 0.6.8 (文件对话框)
- stb_image / stb_image_write (图像读写)

## Architecture

### 核心渲染流程

1. **数据加载** (`Ownplymodel` 类)
   - `plyformat_parser.h`: 解析自定义 PLY 格式头部，验证属性字段（x/y/z, nx/ny/nz, density, scale_0/1/2, rot_0/1/2/3）
   - 读取二进制顶点数据，每个顶点包含 16 个 float（位置、法线、密度、3D 缩放、四元数旋转）
   - 使用实例化渲染：绑定立方体几何体（36 个索引），实例数量等于 PLY 顶点数

2. **着色器管线** (`shader/`)
   - `shader.vert`: 顶点着色器，传递实例 ID 到几何着色器
   - `shader.geom`: **核心**几何着色器，根据实例 ID 从纹理缓冲区读取高斯参数（位置、旋转、缩放），生成椭球体的 billboard 四边形
   - `shader.frag`: 片段着色器，计算高斯权重并输出颜色/密度
   - `cube.vert/frag`: 用于渲染辅助立方体（如包围盒）

3. **UI 与交互** (`imgui_own_render_code.cpp/h`)
   - 使用 ImGui docking 分支，支持多视口和窗口停靠
   - 主要功能模块：
     - 文件加载对话框（ImGuiFileDialog）
     - 相机控制（`camera.h`）：鼠标拖拽旋转、滚轮缩放
     - 渲染参数调节（密度阈值、投影模式等）
     - 帧缓冲渲染到纹理，在 ImGui 窗口中显示

4. **配置文件** (`resources/model/.ply/*.json`)
   - 每个 PLY 模型对应一个 JSON 配置文件
   - 包含 CT 扫描参数：探测器尺寸 (`nDetector`, `dDetector`)、源到探测器距离 (`DSD`)、体素网格 (`nVoxel`, `dVoxel`)
   - 用于正确设置投影矩阵和物理单位

### 关键类与文件

- `src/main.cpp`: 程序入口，初始化 GLFW/GLAD/ImGui，主循环调用 `imgui_own_render_code` 命名空间函数
- `src/ownplymodel.h`: 自定义 PLY 加载器，专门处理 3DGS 格式（与 Assimp 的 `Model` 类并存但用途不同）
- `src/model.h`: 通用 Assimp 模型加载器（用于加载其他格式如 OBJ）
- `src/shader.h`: 着色器编译、链接、uniform 设置的封装类
- `src/camera.h`: 相机类，处理视图矩阵和用户输入
- `src/cube.h`: 立方体几何体生成（用于实例化渲染的基础网格）
- `src/plyexporter.h`: PLY 文件导出功能

### 数据流

```
PLY 文件 → Plyformat_parser（验证头部）→ 读取二进制顶点数据 → 上传到 GPU 纹理缓冲区
                                                                    ↓
用户输入 → Camera → 视图/投影矩阵 → Shader uniforms → 几何着色器实例化 → 片段着色器 → 帧缓冲 → ImGui 纹理显示
```

## Development Notes

- **预处理器定义**: Debug 配置使用 `_CRT_SECURE_NO_WARNINGS` 禁用 MSVC 安全警告
- **字符集**: 项目使用 Unicode 字符集
- **OpenGL 版本**: 使用 OpenGL 3.3 Core Profile
- **着色器版本**: GLSL `#version 330`
- **DPI 缩放**: 代码中已处理高 DPI 显示器缩放（`main_scale` 变量）

### 修改渲染逻辑时注意

- 修改 PLY 格式时需同步更新 `plyformat_parser.h` 中的 `line_type_List` 和 `Line_Type` 枚举
- 几何着色器中的实例化渲染依赖纹理缓冲区布局，修改顶点属性需同步更新着色器中的纹理采样偏移
- 相机回调函数在 `imgui_own_render_code` 命名空间中，修改交互逻辑需注意 ImGui 的输入捕获状态（`io.WantCaptureMouse`）

### 资源路径

- 模型文件: `resources/model/.ply/Base/` 和 `resources/model/.ply/Lingo/`（包含 abdomen, aneurism, chest, foot, head, jaw, leg, pancreas, pelvis）
- 着色器: `shader/` 目录（相对于可执行文件）
- 纹理: `Xray-3DGS-Renderer_dev/CT.png`, `XRay.png`
