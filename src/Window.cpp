#include <GL/glew.h>

#include "Window.h"

#include "utils/Logger.h"

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	// Ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 8) return;
#ifdef NDEBUG
		//return;
#endif

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		WARNING("Source: API");
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		WARNING("Source: Window System");
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		WARNING("Source: Shader Compiler");
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		WARNING("Source: Third Party");
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		WARNING("Source: Application");
		break;
	case GL_DEBUG_SOURCE_OTHER:
		WARNING("Source: Other");
		break;
	default:
		break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		WARNING("Type: Error");
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		WARNING("Type: Deprecated Behaviour");
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		WARNING("Type: Undefined Behaviour");
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		WARNING("Type: Portability");
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		WARNING("Type: Performance");
		break;
	case GL_DEBUG_TYPE_MARKER:
		WARNING("Type: Marker");
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		WARNING("Type: Push Group");
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		WARNING("Type: Pop Group");
		break;
	case GL_DEBUG_TYPE_OTHER:
		WARNING("Type: Other");
		break;
	default:
		break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		WARNING("Severity: high");
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		WARNING("Severity: medium");
		break;
	case GL_DEBUG_SEVERITY_LOW:
		WARNING("Severity: low");
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		WARNING("Severity: notification");
		break;
	default:
		break;
	}

	std::cout << "---------------" << std::endl;
}

Window::Window(int width, int height, const char *title)
{
	// First intialize GLFW
	if (glfwInit() != GLFW_TRUE)
		ERROR("Could not init GLFW.");

	// Set window context values
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glewExperimental = GL_TRUE;
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	// Initialize window
	m_Instance = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!m_Instance)
		ERROR("Could not create GLFW window.");

	// Show window and set user pointer to current window instance
	glfwMakeContextCurrent(m_Instance);
	glfwSetWindowUserPointer(m_Instance, this);

	// Set private window input callbacks
	glfwSetKeyCallback(m_Instance, Window::keyCallback);
	glfwSetCursorPosCallback(m_Instance, Window::cursorPositionCallback);
	glfwSetMouseButtonCallback(m_Instance, Window::mouseButtonCallback);
	glfwSetScrollCallback(m_Instance, Window::mouseScrollCallback);
	glfwSetWindowSizeCallback(m_Instance, Window::resizeCallback);

	// Enable v-sync
	glfwSwapInterval(1);

	// Initialize OpenGL context using GLEW
	GLenum error = glewInit();
	if (error != GLEW_NO_ERROR)
	{
		glfwDestroyWindow(m_Instance);
		glfwTerminate();
		ERROR("Could not init GLEW.");
	}

	// Set OpenGL debug message callback
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	// Resize viewport to current window dimensions
	glViewport(0, 0, width, height);
}

Window::~Window()
{
	glfwSetKeyCallback(m_Instance, nullptr);
	glfwSetCursorPosCallback(m_Instance, nullptr);
	glfwSetMouseButtonCallback(m_Instance, nullptr);
	glfwSetScrollCallback(m_Instance, nullptr);
	glfwSetWindowSizeCallback(m_Instance, nullptr);
	glfwDestroyWindow(m_Instance);
	glfwTerminate();
}

void Window::setTitle(const char *title)
{
	glfwSetWindowTitle(m_Instance, title);
}

void Window::close()
{
	// Reset callbacks
	glfwSetKeyCallback(m_Instance, nullptr);
	glfwSetCursorPosCallback(m_Instance, nullptr);
	glfwSetMouseButtonCallback(m_Instance, nullptr);
	glfwSetScrollCallback(m_Instance, nullptr);
	glfwSetWindowSizeCallback(m_Instance, nullptr);
	glfwSetWindowShouldClose(m_Instance, GLFW_TRUE);
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(m_Instance);
}

void Window::pollEvents()
{
	glfwPollEvents();
	KeysCallback(keys, mouseKeys);
}

void Window::present()
{
	glfwSwapBuffers(m_Instance);
}

bool Window::pressed(unsigned int key) const
{
	if (key < 0 || key >= keys.size())
		return false;

	return keys[key];
}

bool Window::mousePressed(unsigned int key) const
{
	if (key >= mouseKeys.size())
		return false;

	return mouseKeys[key];
}

std::pair<int, int> Window::getDimensions() const
{
	int width, height;
	glfwGetWindowSize(m_Instance, &width, &height);
	return std::make_pair(width, height);
}

void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	auto win = (Window *)glfwGetWindowUserPointer(window);
	if (static_cast<unsigned int>(key) >= win->keys.size())
		return;

	if (action == GLFW_PRESS)
		win->keys[key] = true;
	else if (action == GLFW_RELEASE)
		win->keys[key] = false;
}

void Window::cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
{
	auto win = (Window *)glfwGetWindowUserPointer(window);
	static bool first = true;
	if (first)
	{
		first = false;
		win->LastMousePos.x = xpos;
		win->LastMousePos.y = ypos;
	}

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	const double x = (xpos - win->LastMousePos.x) / double(width);
	const double y = (ypos - win->LastMousePos.y) / double(height);

	win->PosCallback(x, y);

	win->LastMousePos.x = xpos;
	win->LastMousePos.y = ypos;
}

void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	auto win = (Window *)glfwGetWindowUserPointer(window);
	if (static_cast<unsigned int>(button) >= win->mouseKeys.size())
		return;

	if (action == GLFW_PRESS)
		win->mouseKeys[button] = true;
	else if (action == GLFW_RELEASE)
		win->mouseKeys[button] = false;
}

void Window::mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
	auto win = (Window *)glfwGetWindowUserPointer(window);
	win->ScrollCallback(xoffset, yoffset);
}

void Window::resizeCallback(GLFWwindow *window, int width, int height)
{
	auto win = (Window *)glfwGetWindowUserPointer(window);
	win->ResizeCallback(width, height);
}
