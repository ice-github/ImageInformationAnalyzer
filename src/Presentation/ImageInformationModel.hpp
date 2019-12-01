#pragma once

#include "FloatingPointImageData.hpp"
#include "ImageEvaluationData.hpp"
#include "HistogramData.hpp"

#include <memory>

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

            std::unique_ptr<FloatingPointImageData> R;
            std::unique_ptr<FloatingPointImageData> G;
            std::unique_ptr<FloatingPointImageData> B;

            std::unique_ptr<FloatingPointImageData> DenoisedR;
            std::unique_ptr<FloatingPointImageData> DenoisedG;
            std::unique_ptr<FloatingPointImageData> DenoisedB;

            std::unique_ptr<ImageEvaluationData> EvaluatedR;
            std::unique_ptr<ImageEvaluationData> EvaluatedG;
            std::unique_ptr<ImageEvaluationData> EvaluatedB;

            std::unique_ptr<HistogramData> HistogramR;
            std::unique_ptr<HistogramData> HistogramG;
            std::unique_ptr<HistogramData> HistogramB;
            std::unique_ptr<HistogramData> HistogramDenoisedR;
            std::unique_ptr<HistogramData> HistogramDenoisedG;
            std::unique_ptr<HistogramData> HistogramDenoisedB;

            std::unique_ptr<FloatingPointImageData> DifferentialB_G;
            std::unique_ptr<FloatingPointImageData> DifferentialG_R;
            std::unique_ptr<FloatingPointImageData> DifferentialB_R;

            std::unique_ptr<HistogramData> HistogramB_G;
            std::unique_ptr<HistogramData> HistogramG_R;
            std::unique_ptr<HistogramData> HistogramB_R;

            std::unique_ptr<FloatingPointImageData> Surface;
            std::unique_ptr<HistogramData> HistogramSurface;
        };
    }
}
