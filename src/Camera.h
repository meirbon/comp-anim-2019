#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera
{
	/**
  	 * Initializes a camera at position pos with aspect ratio width / height
  	 * @param pos
  	 * @param width
  	 * @param height
  	 */
	explicit Camera(glm::vec3 pos, float width, float height);
	~Camera() = default;

	/**
	 * Resets camera values to original values.
	 */
	void reset();

	/**
	 * Rotates camera by x and y values in degrees
	 * @param rotate
	 */
	void rotate(glm::vec2 rotate);

	/**
	 * Moves camera with an offset of x, y and z in world-space distance
	 * @param offset
	 */
	void move(glm::vec3 offset);

	/**
	 * Set aspect ratio according to width and height
	 * @param width
	 * @param height
	 */
	void resize(float width, float height);

	/**
	 * Set near and far plane of camera
	 * @param distance
	 */
	void setDrawDistance(glm::vec2 distance);

	/**
	 * Returns near and far plane values
	 */
	glm::vec2 getDrawDistance();

	/**
	 * Returns the view matrix
	 */
	const glm::mat4 &getViewMatrix() const;

	/**
	 * Returns the projection matrix
	 * @return
	 */
	const glm::mat4 &getProjectionMatrix() const;

	/**
	 * Returns a combined matrix of view, projection and (optionally) the model matrix
	 * @param model
	 * @return
	 */
	glm::mat4 getCombinedMatrix(glm::mat4 model = glm::mat4(1.0f)) const;

	void setPosition(const glm::vec3 &pos)
	{
		m_Pos = pos;
		update();
	}

	const glm::vec3 &getPosition() const
	{
		return m_Pos;
	}

	const glm::vec3 &getForward() const
	{
		return m_Forward;
	}

	const glm::vec3 &getRight() const
	{
		return m_Right;
	}

	const glm::vec3 &getUp() const
	{
		return m_Up;
	}

  private:
	/**
  	 * Updates vectors and matrices for this camera
  	 */
	void update();

	/**
	 * Camera vectors
	 */
	glm::vec3 m_Pos, m_Forward, m_Right, m_Up;

	/**
	 * Field of view and euler angles
	 */
	float m_Fov, m_Pitch, m_Yaw;
	float m_Width, m_Height;

	/**
	 * Near and far plane distances
	 */
	glm::vec2 m_DrawDist;

	/**
	 * Camera matrices
	 */
	glm::mat4 m_Proj, m_View;
	glm::mat4 m_Combined;
};