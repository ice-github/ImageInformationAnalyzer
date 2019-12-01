
#include "ScaleImageService.hpp"
#include "NormalizeScaleImageDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Infrastructure;

        ScaleImageService::ScaleImageService()
        {
            repository_ = new NormalizeScaleImageDataRepository();
        }
    }
}
