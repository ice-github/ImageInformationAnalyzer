#pragma once

#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;

        class RoundOffHistogramDataRepository : public IHistogramDataRepository
        {
        public:
            explicit RoundOffHistogramDataRepository() = default;
            virtual ~RoundOffHistogramDataRepository() = default;

            virtual HistogramData* Process(const FloatingPointImageData* data, const int histogramSize, const double histogramMinValue, const double histogramMaxValue)
            {
                std::vector<double> histogram;
                histogram.resize(histogramSize);

                for(auto y = 0; y < data->Height; ++y)
                {
                    for(auto x = 0; x < data->Width; ++x)
                    {
                        auto value = data->ImageBuffer[y][x];

                        if(value < histogramMinValue) continue;
                        if(value > histogramMaxValue) continue;

                        //Convert to [0,1]
                        auto normalized = (value - histogramMinValue) / (histogramMaxValue - histogramMinValue);

                        //Convert to [0, histogramSize-1]
                        auto index = static_cast<int>(normalized * (histogramSize - 1.0) + 0.5);

                        histogram[index] += 1.0;
                    }
                }

                for(auto i = 0; i < histogram.size(); ++i)
                {
                    histogram[i] /= histogram.size();
                }

                return new HistogramData(histogram);
            }
        };
    }
}
