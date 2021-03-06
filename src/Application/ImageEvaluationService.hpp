#pragma once

#include "ImageEvaluationData.hpp"

#include <thread>

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;

        class ImageEvaluationService
        {
            IImageEvaluationDataRepository* repository_;
        public:
            enum class Mode
            {
                PSNR,
                SSIM
            };

        public:
            explicit ImageEvaluationService(Mode mode);

            ImageEvaluationData* Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2, const double maxValue)
            {
                if(data1->Width != data2->Width || data1->Height != data2->Height)
                {
                    throw std::invalid_argument("Image sizes are NOT the same!");
                }
                return repository_->Process(data1, data2, maxValue);
            }

            virtual ~ImageEvaluationService()
            {
                delete repository_;
            }
        };
    }
}