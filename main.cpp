#include <GL/glew.h>

#include <glm/glm.hpp>

#include "src/Camera.h"
#include "src/Model.h"
#include "src/Shader.h"
#include "src/VideoPlayer.h"
#include "src/Window.h"
#include "src/utils/Logger.h"

constexpr int WIDTH = 1600;
constexpr int HEIGHT = 900;

using namespace glm;

// Prototypes.
void keyCallback(Window &window, Camera &camera, double &elapsed, const std::vector<bool> &keys, const std::vector<bool> &mouseKeys);
void drawQuad();
void drawLine(const glm::vec3 &v1, const glm::vec3 &v2);

//Enable NVIDIA GPU usage
#ifdef WIN32
#include <Windows.h>
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

constexpr size_t FRAMEBUFFER_COUNT = 2;
enum Framebuffers
{
	WINDOW = 0,
	SKINNED_MESH = 0,
	SKELETON = 1
};

int main()
{
	DEBUG("Initializing window with dimensions: %i, %i.", WIDTH, HEIGHT);
	auto window = Window(WIDTH, HEIGHT, "Computer Animation");

	auto camera = Camera(vec3(21.5f, 23.5f, 110.0f), WIDTH / 3.0f, HEIGHT);

	auto skeletonCamera = Camera(vec3(21.5f, 23.5f, 110.0f), WIDTH / 3.0f, HEIGHT);
	skeletonCamera.rotate(vec2(180.0f + 90.0f, 0.0f));
	auto skinCamera = Camera(vec3(0.f, 0.4f, -3.0f), WIDTH / 3.0f, HEIGHT);
	skinCamera.rotate(vec2(90.0f, 15.0f));

	auto elapsed = 0.0;

	DEBUG("Setting window callbacks");
	window.setKeysCallback([&camera, &window, &elapsed](const std::vector<bool> &keys, const std::vector<bool> &mouseKey) {
		keyCallback(window, camera, elapsed, keys, mouseKey);
	});

	// Mouse callback for rotating camera.
	window.setMousePosCallback([&camera, &window](double x, double y) {
		if (!window.mousePressed(GLFW_MOUSE_BUTTON_LEFT)) return;
		const auto offset = glm::vec2(-float(x), float(y)) * 45.0f;
		camera.rotate(offset);
	});

	// Callback for when window is resized.
	window.setResizeCallback([&camera, &skinCamera](int width, int height) { glViewport(0, 0, width, height); });

	DEBUG("Loading video.");
	// Load in video.
	auto video = VideoPlayer("Data/video.mp4", 0, 701);

	DEBUG("Loading skinned mesh: Data/Capture/capture.DAE");
	Model mesh(true);
	mesh.loadMesh("Data/Capture/capture.DAE");

	// Enable depth testing.
	glEnable(GL_DEPTH_TEST);

	// Generate OpenGL framebuffer linked with textures.
	// This way we can draw the three views of skinned mesh, skeleton and video.
	GLuint FBOTexs[FRAMEBUFFER_COUNT], FBOs[FRAMEBUFFER_COUNT], RBOs[FRAMEBUFFER_COUNT];
	glGenTextures(FRAMEBUFFER_COUNT, FBOTexs);
	glGenFramebuffers(FRAMEBUFFER_COUNT, FBOs);
	glGenRenderbuffers(FRAMEBUFFER_COUNT, RBOs);
	for (int i = 0; i < FRAMEBUFFER_COUNT; i++)
	{
		// Configure texture
		glBindTexture(GL_TEXTURE_2D, FBOTexs[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Create renderbuffer for access to depth-buffer.
		glBindRenderbuffer(GL_RENDERBUFFER, RBOs[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

		// Link texture and renderbuffer to framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FBOs[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOTexs[i], 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBOs[i]);
	}

	// Reset texture and framebuffer OpenGL states
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers::WINDOW);

	// Initialize shaders.
	auto plotShader = Shader("Data/Shaders/quad.vert", "Data/Shaders/quad.frag");
	auto shader = Shader("Data/Shaders/mesh.vert", "Data/Shaders/mesh.frag");
	auto simple = Shader("Data/Shaders/simple.vert", "Data/Shaders/simple.frag");

	// Set lighting arguments for mesh.
	shader.bind();
	shader.setUniformFloat("ambient", glm::vec3(0.1f));
	shader.setUniformFloat("directionalLight.color", glm::normalize(glm::vec3(1)));
	shader.setUniformFloat("directionalLight.direction", glm::normalize(glm::vec3(0.5f, 0.5f, -0.5f)));
	shader.unbind();

	// Setup timing variables.
	double last_time = glfwGetTime();
	elapsed = glfwGetTime();
	constexpr double animationOffset = 0.9;
	double total = animationOffset;
	size_t videoFrame = 0;
	double totalElapsedVideo = 0;
	// For some reason OpenCV does not correctly detect that our video is 60 FPS...
	double timeTillNextFrame = 1.0 / video.getFPS() * 2.0;

	// Load initial frame into video texture.
	video.uploadNextFrame();

	// Main loop.
	while (!window.shouldClose())
	{
		// Calculate passed time.
		elapsed = glfwGetTime() - last_time;
		// Retrieve input events.
		window.pollEvents();

		// Check if we need to load the next video frame to the GPU.
		totalElapsedVideo += elapsed;
		if (totalElapsedVideo > timeTillNextFrame)
		{
			totalElapsedVideo = fmodf(totalElapsedVideo, timeTillNextFrame);
			video.uploadNextFrame();
		}

		last_time = glfwGetTime();
		total += elapsed;

		// Clear the screen.
		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Transform mesh with current animation keyframe, if it returns true -> animation was restarted.
		if (mesh.transformBones(static_cast<float>(total)))
		{
			video.reset();
			video.uploadNextFrame();
			total = animationOffset;
		}

		// Draw mesh to first framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FBOs[Framebuffers::SKINNED_MESH]);
		glViewport(0, 0, WIDTH, HEIGHT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 1.0f);

		shader.bind();
		mesh.render(shader, skinCamera);
		shader.unbind();

		// Draw skeleton to second framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FBOs[Framebuffers::SKELETON]);
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 1.0f);
		simple.bind();

		const auto model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(25.0f, -10.f, 0.f));
		simple.setUniformFloat("MVP", skeletonCamera.getCombinedMatrix(model));

		const auto rig = mesh.getSkeletalRig("MiaFBXASC058Hips");
		for (const auto &bone : rig)
		{
			std::string name;
			glm::vec3 start;
			glm::vec3 end;

			std::tie(name, start, end) = bone;

			drawLine(start, end);
		}
		simple.unbind();

		glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers::WINDOW);

		const auto dimensions = window.getDimensions();
		glViewport(0, 0, std::get<0>(dimensions), std::get<1>(dimensions));

		// Bind plotting shader and draw the three views.
		plotShader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOTexs[Framebuffers::SKINNED_MESH]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOTexs[Framebuffers::SKELETON]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, video.getTextureID());

		// Disable culling for quad drawing.
		glDisable(GL_CULL_FACE);

		// Set texture locations.
		plotShader.setUniformInt("t1", 0);
		plotShader.setUniformInt("t2", 1);
		plotShader.setUniformInt("t3", 2);
		drawQuad();

		// Reset sate.
		plotShader.unbind();
		glBindTexture(GL_TEXTURE_2D, 0);

		window.present();
	}

	// Delete framebuffer objects.
	glDeleteFramebuffers(3, FBOs);
	glDeleteRenderbuffers(3, RBOs);
	glDeleteTextures(3, FBOTexs);

	return 0;
}

/*
 * Callback for keyboard input
 */
void keyCallback(Window &window, Camera &camera, double &elapsed, const std::vector<bool> &keys, const std::vector<bool> &mouseKeys)
{
	if (keys[GLFW_KEY_ESCAPE])
	{
		window.close();
		return;
	}

	const float velocity = static_cast<float>(elapsed) * (keys[GLFW_KEY_LEFT_SHIFT] ? 3.0f : 1.0f);

	vec2 vOffset = vec2(0.0f);
	vec3 offset = vec3(0.0f);

	if (keys[GLFW_KEY_LEFT])
		vOffset.x += 1.0f;
	if (keys[GLFW_KEY_RIGHT])
		vOffset.x += -1.0f;
	if (keys[GLFW_KEY_UP])
		vOffset.y += -1.0f;
	if (keys[GLFW_KEY_DOWN])
		vOffset.y += 1.0f;

	if (keys[GLFW_KEY_W])
		offset.z += velocity;
	if (keys[GLFW_KEY_S])
		offset.z += -velocity;
	if (keys[GLFW_KEY_A])
		offset.x += velocity;
	if (keys[GLFW_KEY_D])
		offset.x += -velocity;
	if (keys[GLFW_KEY_SPACE])
		offset.y += velocity;
	if (keys[GLFW_KEY_LEFT_CONTROL])
		offset.y += -velocity;

	if (keys[GLFW_KEY_O])
	{
		const auto pos = camera.getPosition();
		utils::logger::log("Camera pos: %f, %f, %f", pos.x, pos.y, pos.z);
	}

	camera.rotate(vOffset * velocity * 10.0f);
	camera.move(offset);
}

// Helper function for creating a Vertex Buffer Object
inline GLuint CreateVBO(const GLfloat *data, const unsigned int size)
{
	GLuint id;
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	return id;
}
// Helper function for binding a Vertex Buffer Object
inline void BindVBO(const unsigned int idx, const unsigned int N, const GLuint id)
{
	glEnableVertexAttribArray(idx);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glVertexAttribPointer(idx, N, GL_FLOAT, GL_FALSE, 0, (void *)0);
}

// Helper function to draw a quad covering the whole screen, used to draw the three views.
void drawQuad()
{
	static GLuint VAO = 0;
	if (!VAO)
	{
		// generate buffers
		GLfloat verts[] = {-1, -1, 0, 1, -1, 0, -1, 1, 0, 1, -1, 0, -1, 1, 0, 1, 1, 0};
		GLfloat uvdata[] = {0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0};

		GLuint vertexBuffer = CreateVBO(verts, sizeof(verts));
		GLuint UVBuffer = CreateVBO(uvdata, sizeof(uvdata));
		glGenVertexArrays(1, &VAO);

		glBindVertexArray(VAO);
		BindVBO(0, 3, vertexBuffer);
		BindVBO(1, 2, UVBuffer);
		glBindVertexArray(0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void drawLine(const glm::vec3 &v1, const glm::vec3 &v2)
{
	static GLuint VBO = 0;
	static GLuint VAO = 0;

	if (!VBO || !VAO)
	{
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);
	}

	const glm::vec3 data[2] = {v1, v2};

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), data, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glLineWidth(3.0f);
	glDrawArrays(GL_LINES, 0, 3);
}
