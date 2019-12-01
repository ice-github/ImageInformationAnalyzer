#pragma once

#include "FloatingPointImageData.hpp"

#include <execution>

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        class IDifferentialDataRepository
        {
        public:
            explicit IDifferentialDataRepository() = default;
            virtual ~IDifferentialDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2, std::atomic<int>* processedPixel = nullptr) = 0;
        };
    }
}