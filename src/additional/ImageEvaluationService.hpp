#pragma once

#include "PSNRIImageEvaluationDataRepository.hpp"
#include "SSIMIImageEvaluationDataRepository.hpp"

#include <thread>

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;
        using namespace Infrastructure;

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
            explicit ImageEvaluationService(Mode mode) : repository_(nullptr)
            {
                switch(mode)
                {
                    case Mode::PSNR:
                        repository_ = new PSNRIImageEvaluationDataRepository();
                        break;
                    case Mode::SSIM:
                        repository_ = new SSIMIImageEvaluationDataRepository();
                        break;
                }
            }

            double Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2)
            {
                if(data1->Width != data2->Width || data1->Height != data2->Height)
                {
                    throw std::invalid_argument("Image sizes are NOT the same!");
                }
                repository_->Process(data1, data2);
            }

            virtual ~ImageEvaluationService()
            {
                delete repository_;
            }
        };
    }
}