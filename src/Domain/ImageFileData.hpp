#pragma once
#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        class IImageFileDataRepository
        {
        public:
            enum class Channel
            {
                B,
                G,
                R
            };

            explicit IImageFileDataRepository() = default;
            virtual ~IImageFileDataRepository() = default;

            virtual FloatingPointImageData* Load(const std::string& filePath, const Channel channel) = 0;
            virtual bool Store(const FloatingPointImageData* r, const FloatingPointImageData* g, const FloatingPointImageData* b, const std::string& filePath) = 0;

            virtual bool IsExtensionSupported(const std::string& extension) = 0;
        };
    }
}
