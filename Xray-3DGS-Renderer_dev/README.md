# Xray-3DGS-Renderer

基于 3D Gaussian Splatting 的 X 射线渲染器，使用 OpenGL 和 ImGui 构建交互式可视化界面。

## 项目简介

本项目实现了一个实时 X 射线投影渲染系统，通过 3D Gaussian Splatting 技术表示医学影像数据。系统从 PLY 格式文件加载包含位置、法线、密度、缩放和旋转参数的高斯点云数据，使用几何着色器实例化渲染高斯椭球体，模拟 X 射线穿透物体的投影成像效果。

## 主要特性

- **3DGS 渲染**: 使用几何着色器实例化渲染高斯椭球体，支持自定义密度和形状参数
- **交互式界面**: 基于 ImGui docking 分支，支持多视口、窗口停靠和实时参数调节
- **医学影像支持**: 加载 CT 扫描参数配置，���确设置投影矩阵和物理单位
- **相机控制**: 鼠标拖拽旋转视角，滚轮缩放，支持自由视角观察
- **文件管理**: 集成 ImGuiFileDialog，方便加载不同的 PLY 模型文件

## 技术栈

- **图形 API**: OpenGL 3.3 Core Profile
- **窗口管理**: GLFW 3.x
- **UI 框架**: ImGui 1.92.6 (docking 分支)
- **数学库**: GLM
- **模型加载**: Assimp + 自定义 PLY 解析器
- **图像处理**: stb_image / stb_image_write

## 构建与运行

### 环境要求

- Windows 10/11
- Visual Studio 2022 (v143 toolset)
- Windows SDK 10.0

### 构建步骤

1. 使用 Visual Studio 2022 打开 `Xray-3DGS-Renderer_dev.sln`
2. 选择 `Debug|x64` 配置
3. 按 `Ctrl+Shift+B` 构建项目
4. 按 `F5` 运行

所有依赖库（GLFW, GLAD, GLM, Assimp, ImGui, ImGuiFileDialog）已包含在 `includes/` 和 `lib/` 目录中，无需额外安装。

## 项目结构

```
Xray-3DGS-Renderer_dev/
├── src/                          # 源代码
│   ├── main.cpp                  # 程序入口
│   ├── imgui_own_render_code.*   # 渲染与 UI 逻辑
│   ├── ownplymodel.h             # 自定义 PLY 加载器（3DGS 格式）
│   ├── plyformat_parser.h        # PLY 格式解析器
│   ├── model.h                   # 通用模型加载器（Assimp）
│   ├── shader.h                  # 着色器封装类
│   ├── camera.h                  # 相机控制
│   └── cube.h                    # 立方体几何体
├── shader/                       # GLSL 着色器
│   ├── shader.vert               # 顶点着色器
│   ├── shader.geom               # 几何着色器（核心渲染逻辑）
│   ├── shader.frag               # 片段着色器
│   └── cube.*                    # 辅助立方体着色器
├── resources/model/.ply/         # 模型数据
│   ├── Base/                     # 基础模型（PLY 文件）
│   ├── Lingo/                    # Lingo 模型
│   └── *.json                    # CT 扫描参数配置
├── includes/                     # 第三方库头文件
├── lib/                          # 第三方库静态库
└── imgui-1.92.6-docking/         # ImGui 源码
```

## PLY 数据格式

项目使用自定义的 PLY 格式存储 3DGS 数据，每个顶点包含以下属性（16 个 float）：

- **位置**: `x, y, z`
- **法线**: `nx, ny, nz`
- **密度**: `density`（用于 X 射线衰减计算）
- **缩放**: `scale_0, scale_1, scale_2`（椭球体三轴缩放）
- **旋转**: `rot_0, rot_1, rot_2, rot_3`（四元数表示）

配套的 JSON 文件定义 CT 扫描参数：
- `nDetector`, `dDetector`: 探测器分辨率和像素间距
- `DSD`: 源到探测器距离
- `nVoxel`, `dVoxel`: 体素网格尺寸和间距

## 渲染原理

1. **数据加载**: `Plyformat_parser` 验证 PLY 头部格式，读取二进制顶点数据并上传到 GPU 纹理缓冲区
2. **实例化渲染**: 使用立方体作为基础几何体，实例数量等于 PLY 顶点数
3. **几何着色器**: 根据实例 ID 从纹理缓冲区读取高斯参数，生成面向相机的 billboard 四边形
4. **片段着色器**: 计算每个像素的高斯权重，结合密度参数输出最终颜色
5. **帧缓冲**: 渲染结果输出到纹理，在 ImGui 窗口中显示

## 开发说明

- 修改 PLY 格式时需同步更新 `plyformat_parser.h` 中的属性列表
- 几何着色器中的纹理采样偏移与顶点属性布局强相关，修改时需保持一致
- 相机交互逻辑在 `imgui_own_render_code` 命名空间中，注意处理 ImGui 的输入捕获状态

## 模型数据

项目包含以下医学影像模型（位于 `resources/model/.ply/`）：
- abdomen（腹部）
- aneurism（动脉瘤）
- chest（胸部）
- foot（足部）
- head（头部）
- jaw（下颌）
- leg（腿部）
- pancreas（胰腺）
- pelvis（骨盆）

每个模型提供 Base 和 Lingo 两个版本。

## Git 信息

- **当前分支**: main
- **最近提交**: 加载 json 配置文件、初版渲染器、3sigma 区域外接长方体

## 许可证

（待补充）
