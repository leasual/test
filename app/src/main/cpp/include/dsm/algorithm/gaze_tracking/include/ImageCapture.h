
#ifndef __IMAGE_CAPTURE_h_
#define __IMAGE_CAPTURE_h_

// System includes
#include <fstream>
#include <sstream>
#include <vector>

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Utilities
{

	//===========================================================================
	/**
	A class for capturing sequences from video, webcam, and image directories
	*/
	class ImageCapture {

	public:

		// Default constructor
		ImageCapture() {};

		// Opening based on command line arguments
		bool Open(std::vector<std::string>& arguments);

		// Direct opening

		// Image sequence in the directory
		bool OpenDirectory(std::string directory, std::string bbox_directory="", float fx = -1, float fy = -1, float cx = -1, float cy = -1);

		// Video file
		bool OpenImageFiles(const std::vector<std::string>& image_files, float fx = -1, float fy = -1, float cx = -1, float cy = -1);

		// Getting the next frame
		cv::Mat GetNextImage();

		// Getting the most recent grayscale frame (need to call GetNextImage first)
		cv::Mat_<uchar> GetGrayFrame();

		// Return bounding boxes associated with the image (if defined)
		std::vector<cv::Rect_<float> > GetBoundingBoxes();

		// Parameters describing the sequence and it's progress (what's the proportion of images opened)
		double GetProgress();

		int image_width;
		int image_height;

		float fx, fy, cx, cy;

		// Name of the video file, image directory, or the webcam
		std::string name;

		bool has_bounding_boxes;

	private:

		// Blocking copy and move, as it doesn't make sense to have several readers pointed at the same source
		ImageCapture & operator= (const ImageCapture& other);
		ImageCapture & operator= (const ImageCapture&& other);
		ImageCapture(const ImageCapture&& other);
		ImageCapture(const ImageCapture& other);

		// Storing the latest captures
		cv::Mat latest_frame;
		cv::Mat_<uchar> latest_gray_frame;

		// Keeping track of how many files are read and the filenames
		size_t  frame_num;
		std::vector<std::string> image_files;

		// Could optionally read the bounding box locations from files (each image could have multiple bounding boxes)
		std::vector<std::vector<cv::Rect_<float> > > bounding_boxes;

		void SetCameraIntrinsics(float fx, float fy, float cx, float cy);

		bool image_focal_length_set;
		bool image_optical_center_set;

		bool no_input_specified;

	};
}
#endif