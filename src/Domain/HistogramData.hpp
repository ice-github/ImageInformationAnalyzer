#pragma once

#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        class HistogramData
        {
            double maxValue_;
            double minValue_;

        public:
            const std::vector<double> Data;

            explicit HistogramData(std::vector<double> data) : Data(data)
            {
                maxValue_ = DBL_MIN;
                minValue_ = DBL_MAX;

                for(auto value : data)
                {
                    if(value > maxValue_)
                    {
                        maxValue_ = value;
                    }
                    if(value < minValue_)
                    {
                        minValue_ = value;
                    }
                }
            }

            inline double GetMaxValue() const { return maxValue_; }
            inline double GetMinValue() const { return minValue_; }

            virtual ~HistogramData() = default;
        };

        class IHistogramDataRepository
        {
        public:
            explicit IHistogramDataRepository() = default;
            virtual ~IHistogramDataRepository() = default;

            virtual HistogramData* Process(const FloatingPointImageData* data, const int histogramSize, const double histogramMinValue, const double histogramMaxValue) = 0;
        };
    }
}

