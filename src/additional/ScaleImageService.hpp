#pragma once

#include "NormalizeScaleImageDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;
        using namespace Infrastructure;

        class ScaleImageService
        {
            IScaleImageDataRepository* repository_;

        public:
            explicit ScaleImageService()
            {
                repository_ = new NormalizeScaleImageDataRepository();
            }

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
