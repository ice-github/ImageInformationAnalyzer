#pragma once
#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        class ImageEvaluationData
        {
        public:
            const double Result;

            explicit ImageEvaluationData(const double& result) : Result(result)
            {

            }
            virtual ~ImageEvaluationData() = default;
        };

        class IImageEvaluationDataRepository
        {
        public:
            explicit IImageEvaluationDataRepository() = default;
            virtual ~IImageEvaluationDataRepository() = default;

            virtual ImageEvaluationData* Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2, const double maxValue) = 0;
        };

    }
}
