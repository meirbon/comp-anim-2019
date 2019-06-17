#include "VideoPlayer.h"

#include "utils/Logger.h"

VideoPlayer::VideoPlayer(const char *file_name, int start_frame, int end_frame)
	: m_File(file_name), m_StartFrame(start_frame), m_EndFrame(end_frame)
{
	initialize();

	// Create OpenGL texture.
	glCreateTextures(GL_TEXTURE_2D, 1, &m_TexID);

	// Get initial frame to be able to read width and height of video.
	cv::Mat image;
	m_Capture >> image;

	// Set initial frame to 0.
	reset();

	// Set texture format.
	glTextureStorage2D(m_TexID, 1, GL_RGB8, image.cols, image.rows);
	glTextureParameteri(m_TexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_TexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

VideoPlayer::~VideoPlayer()
{
	// Release OpenCV video object.
	m_Capture.release();
	// Delete texture.
	glDeleteTextures(1, &m_TexID);
}

void VideoPlayer::initialize()
{
	// Open video file.
	m_Capture.open(m_File.c_str());
	if (!m_Capture.isOpened())
	{
		WARNING("Video: \"%s\" could not be loaded.", m_File.c_str());
		return;
	}

	// Initialize variables.
	m_FPS = m_Capture.get(CV_CAP_PROP_FPS);
	m_FrameCount = m_Capture.get(CV_CAP_PROP_FRAME_COUNT);

	// Width and Height are switched on the file.
	m_Width = m_Capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	m_Height = m_Capture.get(CV_CAP_PROP_FRAME_WIDTH);

	// Validating start and end frame.
	if (m_StartFrame < 0 || m_StartFrame > m_FrameCount)
		m_StartFrame = 0;

	if (m_EndFrame < 0 || m_EndFrame > m_FrameCount)
		m_EndFrame = m_FrameCount;
}

void VideoPlayer::reset()
{
	// Set the frame number on the video
	m_Capture.set(CV_CAP_PROP_POS_FRAMES, 0);
}

int VideoPlayer::getFrameCount() const
{
	return m_FrameCount;
}

cv::Mat VideoPlayer::retrieveFrameFromIndex(size_t index)
{
	if (index >= m_FrameCount)
		index %= m_FrameCount;

	m_Capture.set(CV_CAP_PROP_POS_FRAMES, index);
	return retrieveFrame();
}

// Retrieves a frame by the given lerp value between the start_frame and end_frame
cv::Mat VideoPlayer::retrieveFrameFromLerp(double find)
{
	const auto index = find * static_cast<double>((m_EndFrame - m_StartFrame) + m_StartFrame);
	m_Capture.set(CV_CAP_PROP_POS_FRAMES, index);
	return retrieveFrame();
}

// Returns a frame by index of the video
cv::Mat VideoPlayer::retrieveFrame()
{
	// Get next frame.
	m_Capture.grab();

	// Retrieve the frame
	cv::Mat frame;
	m_Capture >> frame;

	return frame;
}

void VideoPlayer::uploadNextFrame()
{
	cv::Mat image = retrieveFrame();
	glTextureSubImage2D(m_TexID, 0, 0, 0, image.cols, image.rows, GL_BGR, GL_UNSIGNED_BYTE, image.ptr());
}