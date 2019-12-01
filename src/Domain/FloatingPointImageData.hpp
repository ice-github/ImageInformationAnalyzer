#pragma once

#include <vector>
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Dense>

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        using namespace std::literals::string_literals;

        class FloatingPointImageData
        {
        public:
            const int Width;
            const int Height;
            const std::vector<std::vector<double>> ImageBuffer;
            const std::vector<std::vector<Eigen::Vector3d>> NormalBuffer;

        private:
            double maxValue_;
            double minValue_;

        public:
            explicit FloatingPointImageData::FloatingPointImageData(const int width, const int height
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

            inline double GetMaxValue() const { return maxValue_; }
            inline double GetMinValue() const { return minValue_; }

            virtual ~FloatingPointImageData() = default;
        };
    }
}
