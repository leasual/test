
#ifndef __LANDMARK_DETECTOR_MODEL_h_
#define __LANDMARK_DETECTOR_MODEL_h_

// OpenCV dependencies
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect.hpp>

// dlib dependencies for face detection
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

#include "PDM.h"
#include "Patch_experts.h"
#include "LandmarkDetectorParameters.h"


using namespace std;

namespace LandmarkDetector {


    class CLNF {
    public:
        //===========================================================================
        PDM pdm;
        Patch_experts patch_experts;
        cv::Mat_<float> params_local;
        cv::Vec6f params_global;

        vector<CLNF> hierarchical_models;
        vector<string> hierarchical_model_names;
        vector<vector<pair<int, int>>> hierarchical_mapping;
        vector<FaceModelParameters> hierarchical_params;

        bool detection_success;
        bool tracking_initialised;
        float detection_certainty;
        bool eye_model;
        vector<cv::Mat_<int> > triangulations;


        cv::Mat_<float> detected_landmarks;
        float model_likelihood;
        cv::Mat_<float> landmark_likelihoods;

        vector<cv::Vec3d> centers;
        vector<cv::Mat_<int> > visibilities;

        int failures_in_a_row;
        cv::Mat_<uchar> face_template;
        cv::Point_<double> preference_det;
        int view_used;
        bool loaded_successfully;

        CLNF();


        CLNF(string fname);

        ~CLNF() {}

        bool DetectLandmarks(const cv::Mat_<uchar> &image, FaceModelParameters &params);

        bool DetectLandmarksfx(const cv::Mat_<uchar> &image, FaceModelParameters &params,
                               bool useclnf = true);


        cv::Mat_<float> GetShape(float fx, float fy, float cx, float cy) const;

        cv::Rect_<float> GetBoundingBox() const;

        cv::Mat_<int> GetVisibilities() const;

        cv::Mat_<int> GetVisibilities11() const;

        void Reset();

        void Reset(double x, double y);

        void Read(string name);

    private:
        void readctviews(const std::string &model_dir);

        int updateEntireByParts();

        int UpdatePartsByEntire(int partmodel, cv::Mat_<float> &part_locs);


        // Helper reading function
        bool Read_CLNF(string clnf_location);
        // the speedup of RLMS using precalculated KDE responses (described in Saragih 2011 RLMS paper)

        map<int, cv::Mat_<float> > kde_resp_precalc;
        // The model fitting: patch response computation and optimisation steps

        bool Fit(const cv::Mat_<float> &intensity_image, const std::vector<int> &window_sizes,
                 const FaceModelParameters &parameters);

        // Mean shift computation that uses precalculated kernel density estimators (the one actually used)
        void NonVectorisedMeanShift_precalc_kde(cv::Mat_<float> &out_mean_shifts,
                                                const vector<cv::Mat_<float> > &patch_expert_responses,
                                                const cv::Mat_<float> &dxs, const cv::Mat_<float> &dys, int resp_size,
                                                float a, int scale, int view_id,
                                                map<int, cv::Mat_<float> > &mean_shifts);

        // The actual model optimisation (update step), returns the model likelihood
        float NU_RLMS(cv::Vec6f &final_global, cv::Mat_<float> &final_local,
                      const vector<cv::Mat_<float> > &patch_expert_responses, const cv::Vec6f &initial_global,
                      const cv::Mat_<float> &initial_local,
                      const cv::Mat_<float> &base_shape, const cv::Matx22f &sim_img_to_ref,
                      const cv::Matx22f &sim_ref_to_img, int resp_size, int view_idx, bool rigid, int scale,
                      cv::Mat_<float> &landmark_lhoods, const FaceModelParameters &parameters, bool compute_lhood);

        // Generating the weight matrix for the Weighted least squares
        void
        GetWeightMatrix(cv::Mat_<float> &WeightMatrix, int scale, int view_id, const FaceModelParameters &parameters);

    };
    //===========================================================================
}


#endif
