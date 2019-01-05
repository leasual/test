
#ifndef __IMAGE_MANIPULATION_HELPERS_h_
#define __IMAGE_MANIPULATION_HELPERS_h_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

namespace Utilities
{
	//===========================================================================
	// Converting between color spaces and bit depths
	//===========================================================================

	static void ConvertToGrayscale_8bit(const cv::Mat& in, cv::Mat& out)
	{
		if (in.channels() == 3)
		{
			// Make sure it's in a correct format
			if (in.depth() == CV_16U)
			{
				cv::Mat tmp = in / 256;
				tmp.convertTo(out, CV_8U);
				cv::cvtColor(out, out, CV_BGR2GRAY);
			}
			else
			{
				cv::cvtColor(in, out, CV_BGR2GRAY);
			}
		}
		else if (in.channels() == 4)
		{
			if (in.depth() == CV_16U)
			{
				cv::Mat tmp = in / 256;
				tmp.convertTo(out, CV_8U);
				cv::cvtColor(out, out, CV_BGRA2GRAY);
			}
			else
			{
				cv::cvtColor(in, out, CV_BGRA2GRAY);
			}
		}
		else
		{
			if (in.depth() == CV_16U)
			{
				cv::Mat tmp = in / 256;
				tmp.convertTo(out, CV_8U);
			}
			else if (in.depth() == CV_8U)
			{
				out = in.clone();
			}
		}
	}


}
#endif