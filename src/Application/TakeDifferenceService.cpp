
#include "TakeDifferenceService.hpp"
#include "EachPixelSpectrumDifferentialDataRepository.hpp"
#include "WholePixelSpectrumDifferentialDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Infrastructure;

        TakeDifferenceService::TakeDifferenceService(Mode mode)
        {
            repository_ = nullptr;
            switch(mode)
            {
                case Mode::EachPixel:
                    repository_ = new EachPixelSpectrumDifferentialDataRepository();
                    break;
                case Mode::WholePixel:
                    repository_ = new WholePixelSpectrumDifferentialDataRepository();
                    break;
            }

        }
    }
}
