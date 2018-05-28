#include <opencv2/core/mat.hpp>

class CameraExtensions
{
public:
    static cv::Mat normalizeLuminance(cv:: Mat src);
    static cv::Mat removeShadowInBlackAndWhite(cv::Mat src, bool normalize = true);
    static void reduceShadowEffect(std::string src);
private:
    CameraExtensions() {}
};