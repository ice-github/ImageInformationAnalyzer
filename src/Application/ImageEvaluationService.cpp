
#include "ImageEvaluationService.hpp"
#include "PSNRIImageEvaluationDataRepository.hpp"
#include "SSIMIImageEvaluationDataRepository.hpp"


namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Infrastructure;

        ImageEvaluationService::ImageEvaluationService(Mode mode) : repository_(nullptr)
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

    }
}
