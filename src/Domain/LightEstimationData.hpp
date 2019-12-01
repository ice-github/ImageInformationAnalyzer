#pragma once

#include "FloatingPointImageData.hpp"

#include <execution>

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        class ILightEstimationDataRepository
        {
        public:
            explicit ILightEstimationDataRepository() = default;
            virtual ~ILightEstimationDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* denoisedR, const FloatingPointImageData* denoisedG, const FloatingPointImageData* denoisedB, const FloatingPointImageData* differentialB_G, const double pixelPitch, std::atomic<double>* progress = nullptr) = 0;
        };

    }
}
