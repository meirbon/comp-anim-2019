#include "Camera.h"

using namespace glm;

Camera::Camera(glm::vec3 pos, float width, float height)
	: m_Pos(pos), m_Forward(vec3(0, 0, 1)), m_Right(vec3(1, 0, 0)), m_Up(vec3(0, 1, 0)),
	  m_Fov(45.0f), m_Pitch(0), m_Yaw(0), m_Width(width), m_Height(height), m_DrawDist({0.1f, 2e3f})
{
	update();
}

void Camera::rotate(glm::vec2 rotate)
{
	m_Yaw += rotate.x;
	// Clamp pitch value to prevent gimbal lock
	m_Pitch = glm::clamp(m_Pitch + rotate.y, -89.0f, 89.0f);
	update();
}

const glm::mat4 &Camera::getViewMatrix() const
{
	return m_View;
}

const glm::mat4 &Camera::getProjectionMatrix() const
{
	return m_Proj;
}

glm::mat4 Camera::getCombinedMatrix(glm::mat4 model) const
{
	return m_Combined * model;
}

void Camera::move(glm::vec3 offset)
{
	m_Pos += offset.x * m_Right + offset.y * m_Up + offset.z * m_Forward;
	update();
}

void Camera::resize(float width, float height)
{
	m_Width = width;
	m_Height = height;
	update();
}

void Camera::setDrawDistance(glm::vec2 distance)
{
	m_DrawDist = distance;
	update();
}

glm::vec2 Camera::getDrawDistance()
{
	return m_DrawDist;
}

void Camera::update()
{
	// Convert euler angles to radians
	const float pitch = glm::radians(m_Pitch);
	const float yaw = glm::radians(m_Yaw);

	// Calculate forward, right and up vectors
	m_Forward = normalize(vec3(cosf(yaw) * cosf(pitch), sinf(pitch), sinf(yaw) * cosf(pitch)));
	m_Right = normalize(cross(m_Forward, vec3(0.0f, 1.0f, 0.0f)));
	m_Up = normalize(cross(m_Forward, m_Right));

	// Use vectors to calculate current camera matrices
	m_Proj = glm::perspective(m_Fov, m_Width / m_Height, m_DrawDist.x, m_DrawDist.y);
	m_View = glm::lookAt(m_Pos, m_Pos + m_Forward, m_Up);
	m_Combined = m_Proj * m_View;
}

void Camera::reset()
{
	m_Pos = vec3(0);
	m_Forward = vec3(0, 0, 1);
	m_Right = vec3(1, 0, 0);
	m_Up = vec3(0, 1, 0);
	m_Fov = 45.0f;
	m_Pitch = 0;
	m_Yaw = 0;
	m_DrawDist = {0.0f, 2e3f};
	update();
}