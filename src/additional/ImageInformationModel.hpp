#pragma once

#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
    namespace Presentation
    {
        using namespace ImageInformationAnalyzer::Domain;

        class ImageInformationModel
        {
        public:
            explicit ImageInformationModel() = default;
            virtual ~ImageInformationModel() = default;

            FloatingPointImageData* R;
            FloatingPointImageData* G;
            FloatingPointImageData* B;

            FloatingPointImageData* DenoisedR;
            FloatingPointImageData* DenoisedG;
            FloatingPointImageData* DenoisedB;
        };
    }
}
