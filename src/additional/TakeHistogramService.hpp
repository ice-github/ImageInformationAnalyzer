#pragma once

#include "RoundOffHistogramDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;
        using namespace Infrastructure;

        class TakeHistogramService
        {
            IHistogramDataRepository* repository_;

        public:
            explicit TakeHistogramService()
            {
                repository_ = new RoundOffHistogramDataRepository();
            }

            virtual HistogramData* Process(const FloatingPointImageData* data, const int histogramSize, const double histogramMinValue, const double histogramMaxValue)
            {
                return repository_->Process(data, histogramSize, histogramMinValue, histogramMaxValue);
            }

            virtual ~TakeHistogramService()
            {
                delete repository_;
            }
        };
    }
}
