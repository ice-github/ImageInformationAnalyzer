#pragma once

#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
    	        class IScaleImageDataRepository
        {
        public:
            explicit IScaleImageDataRepository() = default;
            virtual ~IScaleImageDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data, const double oldMinValue, const double oldMaxValue, const double newMinValue, const double newMaxValue) = 0;
        };
    }
}
