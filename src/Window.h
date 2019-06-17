#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <functional>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>

class Window
{
  public:
	/**
   	 * Initializes a GLFW3 Window and OpenGL context
   	 * @param width 	Width of window
   	 * @param height 	Height of window
   	 * @param title 	Initial title to use
   	 */
	Window(int width, int height, const char *title);
	~Window();

	/**
	 * Changewindow title
	 * @param title
	 */
	void setTitle(const char *title);

	/**
	 * Notify window to be closed
	 */
	void close();

	/**
	 * Check if window should close
	 * @return
	 */
	bool shouldClose();

	/**
	 * Retrieve input events
	 */
	void pollEvents();

	/**
	 * Swap OpenGL buffers of window and present content
	 */
	void present();

	/**
	 * Check if keyboard key is pressed
	 * @param key 	GLFW key code
	 * @return		Whether key is pressed
	 */
	bool pressed(unsigned int key) const;

	/**
	 * Check if mouse button is pressed
	 * @param key 	GLFW mouse code
	 * @return 		Whether mouse button is pressed
	 */
	bool mousePressed(unsigned int key) const;

	/**
	 * Get current window dimensions
	 * @return 		tuple of width and height
	 */
	std::pair<int, int> getDimensions() const;

	/**
	 * Set callback for mouse input
	 * @param callback
	 */
	inline void setMousePosCallback(std::function<void(double, double)> callback)
	{
		PosCallback = std::move(callback);
	}

	/**
	 * Set callback for keyboard input
	 * @param callback
	 */
	inline void setKeysCallback(std::function<void(const std::vector<bool> &, const std::vector<bool> &)> callback)
	{
		KeysCallback = std::move(callback);
	}

	/**
	 * Set callback for mouse scroll input
	 * @param callback
	 */
	inline void setScrollCallback(std::function<void(double, double)> callback)
	{
		ScrollCallback = std::move(callback);
	}

	/**
	 * Set callback for window resize event
	 * @param callback
	 */
	inline void setResizeCallback(std::function<void(int, int)> callback)
	{
		ResizeCallback = std::move(callback);
	}

  private:
	glm::dvec2 LastMousePos = {0.0, 0.0};

	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos);
	static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	static void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
	static void resizeCallback(GLFWwindow *window, int width, int height);

	GLFWwindow *m_Instance;

	std::vector<bool> keys = std::vector<bool>(512);
	std::vector<bool> mouseKeys = std::vector<bool>(32);

	std::function<void(int, int)> ResizeCallback = [](int, int) {};
	std::function<void(double, double)> PosCallback = [](double, double) {};
	std::function<void(const std::vector<bool> &, const std::vector<bool> &)> KeysCallback = [](auto, auto) {};
	std::function<void(double, double)> ScrollCallback = [](double, double) {};
};
