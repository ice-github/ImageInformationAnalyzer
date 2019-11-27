#include "FloatingPointImageData.hpp"


namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        FloatingPointImageData::FloatingPointImageData(const int width, const int height
            , const std::vector<std::vector<double>> imageBuffer
            , const std::vector<std::vector<Eigen::Vector3d>> normalBuffer) : Width(width), Height(height), ImageBuffer(imageBuffer), NormalBuffer(normalBuffer)
        {
            maxValue_ = DBL_MIN;
            minValue_ = DBL_MAX;

            for(auto line : imageBuffer)
            {
                for(auto value : line)
                {
                    if(value > maxValue_)
                    {
                        maxValue_ = value;
                    }
                    if(value < minValue_)
                    {
                        minValue_ = value;
                    }
                }
            }
        }
    }
}