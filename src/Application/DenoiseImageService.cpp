
#include "DenoiseImageService.hpp"
#include "CircleDenoiseDataRepository.hpp"
#include "EllipseDenoiseDataRepository.hpp"
#include "HyperEllipseDenoiseDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Infrastructure;

        DenoiseImageService::DenoiseImageService(Mode mode)
        {
            repository_ = nullptr;

            switch(mode)
            {
                case Mode::CIRCLE:
                    repository_ = new CircleDenoiseDataRepository();
                    break;
                case Mode::ELLIPSE:
                    repository_ = new EllipseDenoiseDataRepository();
                    break;
                case Mode::HYPER_ELLIPSE:
                    repository_ = new HyperEllipseDenoiseDataRepository();
                    break;
                default:
                    break;
            }
        }
    }
}
