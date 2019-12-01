
#include "EstimateLightDirectionService.hpp"
#include "PhongModelLightDirectionDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Infrastructure;

        EstimateLightDirectionService::EstimateLightDirectionService()
        {
            repository_ = new PhongModelLightDirectionDataRepository();
        }

    }
}
