#pragma once

#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;

        class PSNRIImageEvaluationDataRepository : public IImageEvaluationDataRepository
        {
        public:
            explicit PSNRIImageEvaluationDataRepository() = default;
            virtual ~PSNRIImageEvaluationDataRepository() = default;

            virtual double Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2)
            {
                constexpr auto maxValue = 255.0;

                auto width = data1->Width;
                auto height = data1->Height;

                auto mse = 0.0;
                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        auto diff = data1->ImageBuffer[y][x] - data2->ImageBuffer[y][x];
                        mse += diff * diff;
                    }
                }
                mse /= ((double)width * height);

                //PSNR
                return 10.0 * std::log10(maxValue * maxValue / mse);
            }
        };
    }
}
