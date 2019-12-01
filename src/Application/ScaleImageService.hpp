#pragma once

#include "ScaleImageData.hpp"

class NormalizeScaleImageDataRepository;

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;

        class ScaleImageService
        {
            IScaleImageDataRepository* repository_;

        public:
            explicit ScaleImageService();

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data, const double oldMinValue, const double oldMaxValue, const double newMinValue, const double newMaxValue)
            {
                return repository_->Process(data, oldMinValue, oldMaxValue, newMinValue, newMaxValue);
            }

            virtual ~ScaleImageService()
            {
                delete repository_;
            }
        };
    }
}
