#version 330 core

out vec4 FragColor;

uniform mat4 MVP;
uniform vec3 cameraPos;
uniform float exposure;
uniform bool isVoxelizer;
uniform vec2 screenSize;
uniform vec3 cubePos;

in float density;
//out float mark;
in vec2 center;
in mat2 cov2D;
in mat3 cov3D;
in float mu;
in vec3 centerGS;

float det2(mat2 M) {
    return M[0][0]*M[1][1] - M[0][1]*M[1][0];
}

float det3(mat3 M) {
    return
        M[0][0]*(M[1][1]*M[2][2] - M[1][2]*M[2][1]) -
        M[0][1]*(M[1][0]*M[2][2] - M[1][2]*M[2][0]) +
        M[0][2]*(M[1][0]*M[2][1] - M[1][1]*M[2][0]);
}

float gaussian3D(vec3 p)
{
    vec3 d = p - centerGS;

    // 这里用协方差矩阵的逆来计算高斯值
    mat3 invCov = inverse(cov3D);
    float exponent = -0.5 * dot(d, cov3D * d);
    float norm = 1.0 / sqrt(pow(2.0 * 3.1415926, 3.0));

    return norm * exp(exponent);
}

void main()
{
	bool isVox = false;
	if(isVox)
	{
		// 当前使用voxelizer的结果与直接渲染的结果大差不差，但需要更高的曝光，不太适用
		// 当前像素归一化到 [0,1]
		vec2 uv = gl_FragCoord.xy / screenSize;

		// 平面范围：[-range, range]
		float range = 2;

		// 根据 cubePos 中非零轴判断当前平面类型
		// 假设 cubePos.x / y / z 只有一个非零，表示固定的平面位置
		vec3 pos = cubePos;

		// 构造当前像素对应的平面坐标
		// 这里默认：
		// 若 cubePos.x 非零 -> 平面为 YZ 平面
		// 若 cubePos.y 非零 -> 平面为 XZ 平面
		// 若 cubePos.z 非零 -> 平面为 XY 平面
		if (abs(cubePos.x) > 1e-10)
		{
			float y = (uv.x * 2.0 - 1.0) * range;
			float z = (uv.y * 2.0 - 1.0) * range;
			pos = vec3(cubePos.x, y, z);
		}
		else if (abs(cubePos.y) > 1e-10)
		{
			float x = (uv.x * 2.0 - 1.0) * range;
			float z = (uv.y * 2.0 - 1.0) * range;
			pos = vec3(x, cubePos.y, z);
		}
		else
		{
			float x = (uv.x * 2.0 - 1.0) * range;
			float y = (uv.y * 2.0 - 1.0) * range;
			pos = vec3(x, y, cubePos.z);
		}

		// 计算当前像素位置对应的 3DGS 值
		float g = gaussian3D(pos);

		// 可以结合 density / mu 做最终输出
		float value = g * density * mu;

		// 体素化时输出灰度值
		FragColor = vec4(vec3(value * exposure * 10), 1.0);
	}
	else
	{
		float detCov2D = det2(cov2D);
		if (detCov2D <= 0.0) {
			discard; // Invalid covariance, skip this fragment
		}
		float detInv = 1.0 / detCov2D;
		vec3 cov = vec3(cov2D[0][0], cov2D[0][1], cov2D[1][1]);
		vec3 conic = vec3(cov.z * detInv, -cov.y * detInv, cov.x * detInv);

		vec2 sub = gl_FragCoord.xy - center;
		float power = -0.5 * (conic.x * sub.x * sub.x + conic.z * sub.y * sub.y) - conic.y * sub.x * sub.y;
		if (power > 0){
			discard;
		}
		float factor = density * mu *  exp(power);
		FragColor = vec4(vec3(density * mu *  exp(power) * exposure), 1.0);
	}
}