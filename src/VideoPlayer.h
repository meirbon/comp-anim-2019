#pragma once

#include <GL/glew.h>
#include <iostream>
#include <opencv2/opencv.hpp>

#include <string>

class VideoPlayer
{
  public:
	VideoPlayer(const char *file_name, int start_frame = 0, int end_frame = -1);
	~VideoPlayer();

	cv::Mat retrieveFrameFromLerp(double find);
	cv::Mat retrieveFrameFromIndex(size_t index);

	/*
	 * Resets video to its initial frame.
	 */
	void reset();

	/*
	 * Get number of frames.
	 */
	int getFrameCount() const;
	
	/*
	 * Returns number of frames per second.
	 */
	int getFPS() const { return m_FPS; }

	/*
	 * Returns OpenGL texture ID to which video is uploaded.
	 */
	GLuint getTextureID() const { return m_TexID; }

	/*
	 * Grabs next frame from video and uploads it to OpenGL texture.
	 */
	void uploadNextFrame();

  private:
	int m_Width, m_Height;
	int m_StartFrame, m_EndFrame;
	int m_FrameCount;
	int m_FPS = 30;
	std::string m_File;
	cv::VideoCapture m_Capture;
	GLuint m_TexID = 0;

	/*
	 * Initializes video stream from OpenCV.
	 */
	void initialize();

	/*
	 * Retrieves a frame from the video.
	 */
	cv::Mat retrieveFrame();
};