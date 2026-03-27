#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>

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

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{	
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// TODO: 通过相机参数构造相机，当前函数仍有bug，后续需要进行调整更改
	Camera(float angle, float DSO, float sDetector, float DSD, float nVoxel, float dVoxel)
	{
		float scale = 2 / (nVoxel / dVoxel / 1000);
		glm::mat4 c2w = angle2pose(DSO * scale, glm::radians(angle));
		glm::mat4 w2c = glm::inverse(c2w);
		glm::mat3 R = glm::mat3(w2c);
		Position = glm::vec3(w2c[3]);

		Pitch = glm::degrees(asin(Front.y));
		Yaw = glm::degrees(atan2(Front.z, Front.x));

		WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		updateCameraVectors();

		Zoom = glm::degrees(2.0f * atan2f(sDetector / 2.0f, DSD));
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
	}
};

#endif