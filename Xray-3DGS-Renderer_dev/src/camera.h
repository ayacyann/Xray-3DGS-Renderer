#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 3.0f;
const float SENSITIVITY = 0.5f;
const float ZOOM = 45.0f;

class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	glm::vec2 Focal;
	glm::vec2 Fov;
	glm::vec2 ScreenSize;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Focal = glm::vec2(ZOOM, ZOOM);
		ScreenSize = glm::vec2(512.0f, 512.0f);
		Fov = Focal2Fov(Focal, ScreenSize);
		updateCameraVectors();
	}
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		Focal = glm::vec2(ZOOM, ZOOM);
		ScreenSize = glm::vec2(512.0f, 512.0f);
		Fov = Focal2Fov(Focal, ScreenSize);
		updateCameraVectors();
	}

	// TODO: 通过相机参数构造相机，当前函数仍有bug，后续需要进行调整更改
	Camera(glm::vec2 shape, glm::vec2 nDetector, glm::vec2 dDetector, float DSD, glm::vec3 nVoxel, glm::vec3 dVoxel, float scale = 1000.0) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Position(glm::vec3(0.0f, 0.0f, 8.0f)), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)), Yaw(YAW), Pitch(PITCH)
	{
		UpdateCameraParameters(shape, nDetector, dDetector, DSD, nVoxel, dVoxel, scale);
		updateCameraVectors();
	}

	// TODO: 通过相机参数构造相机，当前函数仍有bug，后续需要进行调整更改
	Camera(const std::string& path, float scale = 1000.0) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Position(glm::vec3(0.0f, 0.0f, 8.0f)), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)), Yaw(YAW), Pitch(PITCH)
	{
		UpdateCameraParameters(path,scale);
		updateCameraVectors();
	}

	void UpdateCameraParameters(glm::vec2 shape, glm::vec2 nDetector, glm::vec2 dDetector, float DSD, glm::vec3 nVoxel, glm::vec3 dVoxel, float scale = 1000.0)
	{
		glm::vec3 sVoxel = nVoxel * dVoxel / scale;
		glm::vec2 sDetector = nDetector * dDetector / scale;

		float sceneScale = 2 / glm::max(sVoxel.x, glm::max(sVoxel.y, sVoxel.z));

		sVoxel *= sceneScale;
		DSD = DSD * sceneScale / scale;
		sDetector *= sceneScale;

		float fovX = std::atan2(sDetector.y / 2.0f, DSD) * 2.0f;
		float fovY = std::atan2(sDetector.x / 2.0f, DSD) * 2.0f;
		Fov = glm::vec2(glm::degrees(fovX), glm::degrees(fovY));
		Zoom = Fov.y;
		Focal = Fov2Focal(Fov, shape);
		ScreenSize = shape;
	}

	void UpdateCameraParameters(const std::string& path, float scale=1000.0)
	{
		std::ifstream f(path);
		json js;
		f >> js;
		glm::vec2 shape = glm::vec2(js["shape"][0], js["shape"][1]);
		glm::vec2 nDetector = glm::vec2(js["nDetector"][0], js["nDetector"][1]);
		glm::vec2 dDetector = glm::vec2(js["dDetector"][0], js["dDetector"][1]);
		float DSD = js["DSD"];
		glm::vec3 nVoxel = glm::vec3(js["nVoxel"][0], js["nVoxel"][1], js["nVoxel"][2]);
		glm::vec3 dVoxel = glm::vec3(js["dVoxel"][0], js["dVoxel"][1], js["dVoxel"][2]);
		printf("Camera Parameters:\nshape: [%f, %f]\nnDetector: [%f, %f]\ndDetector: [%f, %f]\nDSD: %f\nnVoxel: [%f, %f, %f]\ndVoxel: [%f, %f, %f]\n", shape.x, shape.y, nDetector.x, nDetector.y, dDetector.x, dDetector.y, DSD, nVoxel.x, nVoxel.y, nVoxel.z, dVoxel.x, dVoxel.y, dVoxel.z);
		UpdateCameraParameters(shape, nDetector, dDetector, DSD, nVoxel, dVoxel, scale);
	}

	void SetScreenSize(glm::vec2 screenSize)
	{
		ScreenSize = screenSize;
		Focal = Fov2Focal(Fov, ScreenSize);
	}

	glm::vec2 Fov2Focal(glm::vec2 fov, glm::vec2 sensorSize)
	{
		float focalX = (sensorSize.x / 2.0f) / tan(glm::radians(fov.x) / 2.0f);
		float focalY = (sensorSize.y / 2.0f) / tan(glm::radians(fov.y) / 2.0f);
		return glm::vec2(focalX, focalY);
	}

	glm::vec2 Focal2Fov(glm::vec2 focal, glm::vec2 sensorSize)
	{
		float fovX = 2.0f * glm::degrees(atan((sensorSize.x / 2.0f) / focal.x));
		float fovY = 2.0f * glm::degrees(atan((sensorSize.y / 2.0f) / focal.y));
		return glm::vec2(fovX, fovY);
	}

	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (direction == UP)
			Position.y += velocity;
		if (direction == DOWN)
			Position.y -= velocity;
	}

	glm::mat4 angle2pose(float DSO, float angle)
	{
		float phi1 = -glm::pi<float>() / 2.0f;
		float phi2 = glm::pi<float>() / 2.0f;
		float angle_rad = glm::radians(angle);

		// ===== 构造旋转矩阵 =====
		glm::mat4 R1 = glm::rotate(glm::mat4(1.0f), phi1, glm::vec3(1, 0, 0));
		glm::mat4 R2 = glm::rotate(glm::mat4(1.0f), angle_rad, glm::vec3(0, 1, 0));
		glm::mat4 R3 = glm::rotate(glm::mat4(1.0f), phi2, glm::vec3(0, 0, 1));

		glm::mat4 R = R3 * R2 * R1;

		// ===== 平移 =====
		glm::vec3 t(
			DSO * std::cos(angle_rad),
			0.0f,
			DSO * std::sin(angle_rad)
		);

		// ===== 组合成 transform =====
		glm::mat4 transform = R;
		transform[3] = glm::vec4(t.x, t.y, t.z, 1.0f);

		return transform;
	}

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		updateCameraVectors();
	}

	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 15.0f)
			Zoom = 15.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

private:
	void updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
		front.y = sin(glm::radians(Pitch));
		front.z = cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));
		Front = glm::normalize(front);
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
		Front = glm::normalize(glm::cross(Up, Right));
	}
};

#endif