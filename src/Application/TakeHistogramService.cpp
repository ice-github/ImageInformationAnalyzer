
#include "TakeHistogramService.hpp"
#include "RoundOffHistogramDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Infrastructure;

        TakeHistogramService::TakeHistogramService()
        {
            repository_ = new RoundOffHistogramDataRepository();
        }

    }
}
