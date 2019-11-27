#pragma once

#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;

        class SSIMIImageEvaluationDataRepository : public IImageEvaluationDataRepository
        {
        public:
            explicit SSIMIImageEvaluationDataRepository() = default;
            virtual ~SSIMIImageEvaluationDataRepository() = default;

            virtual double Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2)
            {
                auto width = data1->Width;
                auto height = data1->Height;

                auto average1 = 0.0;
                auto average2 = 0.0;
                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        average1 += data1->ImageBuffer[y][x];
                        average2 += data2->ImageBuffer[y][x];
                    }
                }
                average1 /= ((double)width * height);
                average2 /= ((double)width * height);

                auto variance1 = 0.0;
                auto variance2 = 0.0;
                auto covariance = 0.0;

                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        variance1 += (data1->ImageBuffer[y][x] - average1) * (data1->ImageBuffer[y][x] - average1);
                        variance2 += (data2->ImageBuffer[y][x] - average2) * (data2->ImageBuffer[y][x] - average2);
                        covariance += (data1->ImageBuffer[y][x] - average1) * (data2->ImageBuffer[y][x] - average2);
                    }
                }
                variance1 /= ((double)width * height);
                variance2 /= ((double)width * height);
                covariance /= ((double)width * height);

                constexpr auto K1 = 0.01;
                constexpr auto K2 = 0.03;
                constexpr auto dynamicRange = 255.0;

                constexpr auto C1 = (K1 * dynamicRange) * (K1 * dynamicRange);
                constexpr auto C2 = (K2 * dynamicRange) * (K2 * dynamicRange);
                constexpr auto C3 = C2 / 2.0;

                auto l = (2.0 * average1 * average2 + C1) / (average1 * average1 + average2 * average2 + C1);
                auto c = (2.0 * std::sqrt(variance1) * std::sqrt(variance2) + C2) / (variance1 * variance2 + C2);
                auto s = (covariance + C3) / (std::sqrt(variance1) * std::sqrt(variance2) + C3);

                constexpr auto alpha = 1.0;
                constexpr auto beta = 1.0;
                constexpr auto gamma = 1.0;

                //SSIM
                return std::pow(l, alpha) + std::pow(c, beta) + std::pow(s, gamma);
            }
        };
    }
}
