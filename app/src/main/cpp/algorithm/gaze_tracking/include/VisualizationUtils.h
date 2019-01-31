
#ifndef __VISUALIZATION_UTILS_h_
#define __VISUALIZATION_UTILS_h_

#include <opencv2/core/core.hpp>

#include <vector>
#include <queue>

namespace Utilities
{

	// Drawing a bounding box around the face in an image
	void DrawBox(cv::Mat image, cv::Vec6f pose, cv::Scalar color, int thickness, float fx, float fy, float cx, float cy);
	void DrawBox(const std::vector<std::pair<cv::Point2f, cv::Point2f>>& lines, cv::Mat image, cv::Scalar color, int thickness);

	// Computing a bounding box to be drawn
	std::vector<std::pair<cv::Point2f, cv::Point2f>> CalculateBox(cv::Vec6f pose, float fx, float fy, float cx, float cy);

    void Visualise_FHOG(const cv::Mat_<double>& descriptor, int num_rows, int num_cols, cv::Mat& visualisation);

	class FpsTracker
	{
	public:
		
		double history_length;

		void AddFrame();

		double GetFPS();

		FpsTracker();

	private:
		std::queue<double> frame_times;

		void DiscardOldFrames();

	};

}
#endif