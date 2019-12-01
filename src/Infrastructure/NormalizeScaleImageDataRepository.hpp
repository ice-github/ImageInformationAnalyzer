#pragma once

#include "ScaleImageData.hpp"
#include "ImageUtility.hpp"

#include <stdexcept>
#include <opencv2/opencv.hpp>

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;
        using namespace Misc;
        using namespace std::literals::string_literals;

        class NormalizeScaleImageDataRepository : public IScaleImageDataRepository
        {
        public:
            explicit NormalizeScaleImageDataRepository() = default;
            virtual ~NormalizeScaleImageDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data, const double oldMinValue, const double oldMaxValue, const double newMinValue, const double newMaxValue) override
            {
                auto width = data->Width;
                auto height = data->Height;

                std::vector < std::vector <double>> imageBuffer;
                std::vector<std::vector<Eigen::Vector3d>> normalBuffer;

                imageBuffer.resize(height);
                normalBuffer.resize(height);

                for(auto y = 0; y < height; ++y)
                {
                    imageBuffer[y].resize(width);
                    normalBuffer[y].resize(width);
                    for(auto x = 0; x < width; ++x)
                    {
                        auto value = data->ImageBuffer[y][x];

                        //[OldMinValue, OldMaxValue] => [0, 1] => [NewMinValue, NewMaxValue]
                        auto normalized = ImageUtility::DoubleSub(value, oldMinValue) / (oldMaxValue - oldMinValue);

                        imageBuffer[y][x] = ImageUtility::DoubleAdd(normalized * (newMaxValue - newMinValue), newMinValue);
                        normalBuffer[y][x] = data->NormalBuffer[y][x];
                    }
                }

                return new FloatingPointImageData(width, height, imageBuffer, normalBuffer);
            }
        };
    }
}