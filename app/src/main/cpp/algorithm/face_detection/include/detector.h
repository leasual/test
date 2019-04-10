#ifndef _include_opencv_mtcnn_detector_h_
#define _include_opencv_mtcnn_detector_h_

#include "face.h"
#include "onet.h"
#include "pnet.h"
#include "rnet.h"
#include <memory.h>
class MTCNNDetector {
private:
    std::unique_ptr<ProposalNetwork> _pnet;
    std::unique_ptr<RefineNetwork> _rnet;
    std::unique_ptr<OutputNetwork> _onet;

    cv::Rect2i make_square(const cv::Rect2i &rect);
public:
    MTCNNDetector(const ProposalNetwork::Config &pConfig,
                const RefineNetwork::Config &rConfig,
                const OutputNetwork::Config &oConfig);
    MTCNNDetector(const std::string &mtcnn_dir, const float p_threshold=0.6f, const float r_threshold=0.7f, const float o_threshold=0.7f);

    std::vector<Face> detect(const cv::Mat &img, const float minFaceSize = 20.f,
                             const float scaleFactor = 0.709f);
    Face get_largest_face(const std::vector<Face> &faces);
};

#endif
