
#ifndef __VISUALIZER_h_
#define __VISUALIZER_h_

// System includes
#include <vector>

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Utilities {

    //===========================================================================
    /**
    A class for recording data processed by OpenFace (facial landmarks, head pose, facial action units, aligned face, HOG features, and tracked video
    */
    class Visualizer {

    public:

        // The constructor for the visualizer that specifies what to visualize
        Visualizer(std::vector<std::string> arguments);

        Visualizer(bool vis_track, bool vis_hog, bool vis_align, bool vis_aus);

        inline void setStatus(bool vis_track,
                              bool vis_hog,
                              bool vis_align, bool vis_aus) {
            this->vis_track = vis_track;
            this->vis_hog = vis_hog;
            this->vis_align = vis_align;
            this->vis_aus = vis_aus;
        }

        // Adding observations to the visualizer
        // Pose related observations
        void SetImage(const cv::Mat &canvas, float fx, float fy, float cx, float cy);

        // All observations relevant to facial landmarks (optional visibilities parameter to not display all landmarks)
        void SetObservationLandmarks(const cv::Mat_<float> &landmarks_2D, double confidence,
                                     const cv::Mat_<int> &visibilities = cv::Mat_<int>());
        void SetObservationLandmarks1(cv::Mat & rgbframe,
                                      const cv::Mat_<float> &landmarks_2D,
                                     const cv::Mat_<int> &visibilities = cv::Mat_<int>());


        void SetEyeLandmarks(
                const cv::Mat_<float> &landmarks_2D, double confidence);

        void SetGazeLandmarks(
                const cv::Mat_<float> &landmarks_2D, double confidence);

        // Pose related observations
        void SetObservationPose(const cv::Vec6f &pose, double confidence);
        void SetObservationPose1(cv::Mat & rgbframe,const cv::Vec6f &pose);


        void SetObservationActionUnits(const std::vector<std::pair<std::string, double> > &au_intensities,
                                       const std::vector<std::pair<std::string, double> > &au_occurences);

        // Gaze related observations
        void SetObservationGaze(const cv::Point3f &gazeDirection0, const cv::Point3f &gazeDirection1,
                                const std::vector<cv::Point2f> &eye_landmarks,
                                const std::vector<cv::Point3f> &eye_landmarks3d, double confidence);
        void SetObservationGaze1(cv::Mat & rgbframe,const cv::Point3f &gazeDirection0,
                                 const cv::Point3f &gazeDirection1,
                                const std::vector<cv::Point2f> &eye_landmarks,
                                const std::vector<cv::Point3f> &eye_landmarks3d);

        // Face alignment related observations
        void SetObservationFaceAlign(const cv::Mat &aligned_face);

        // HOG feature related observations
        void SetObservationHOG(const cv::Mat_<double> &hog_descriptor, int num_cols, int num_rows);

        void SetFps(double fps);

        // Return key-press that could have resulted in the open windows
        char ShowObservation();

        cv::Mat GetVisImage();

        cv::Mat GetHOGVis();

        // Keeping track of what we're visualizing
        bool vis_track;
        bool vis_hog;
        bool vis_align;
        bool vis_aus;

        // Can be adjusted to show less confident frames
        double visualisation_boundary = 0.4;

    private:

        // Temporary variables for visualization
        cv::Mat captured_image; // out canvas
        cv::Mat tracked_image;
        cv::Mat hog_image;
        cv::Mat aligned_face_image;
        cv::Mat action_units_image;

        // Useful for drawing 3d
        float fx, fy, cx, cy;

    };
}
#endif