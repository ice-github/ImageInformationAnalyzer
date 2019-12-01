#pragma once

#include "ImageEvaluationData.hpp"

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

            virtual ImageEvaluationData* Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2, const double maxValue) override
            {
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
                auto psnr = 10.0 * std::log10(maxValue * maxValue / mse);
                return new ImageEvaluationData(psnr);
            }
        };
    }
}
