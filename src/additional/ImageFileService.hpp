#pragma once

#include "JpegFileDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;
        using namespace Infrastructure;

        class ImageFileService
        {
            IImageFileDataRepository* jpegRepository_;
            IImageFileDataRepository* jsonRepository_;

        public:
            explicit ImageFileService()
            {
                jpegRepository_ = new JpegFileDataRepository();
                jsonRepository_ = nullptr;
            }

            virtual FloatingPointImageData* Load(const std::string& filePath, const IImageFileDataRepository::Channel channel)
            {
                auto extensionPos = filePath.rfind("."s);
                if(extensionPos == std::string::npos) throw std::invalid_argument("invalid image file!"s);

                auto extension = filePath.substr(extensionPos + 1);
                if(extension == "jpg"s || extension == "jpeg"s)
                {
                    return jpegRepository_->Load(filePath, channel);
                }

                if(extension == "json"s)
                {
                    return jsonRepository_->Load(filePath, channel);
                }

                throw std::logic_error("file type not supported!"s);
            }
            virtual bool Store(const FloatingPointImageData* r, const FloatingPointImageData* g, const FloatingPointImageData* b, const std::string& filePath)
            {
                auto extensionPos = filePath.rfind("."s);
                if(extensionPos == std::string::npos) throw std::invalid_argument("invalid image file!"s);

                auto extension = filePath.substr(filePath.length() + 1);
                if(extension == "jpg"s || extension == "jpeg"s)
                {
                    return jpegRepository_->Store(r, g, b, filePath);
                }

                if(extension == "json"s)
                {
                    return jsonRepository_->Store(r, g, b, filePath);
                }

                throw std::logic_error("file type not supported!"s);
            }

            virtual ~ImageFileService()
            {
                delete jpegRepository_;
                delete jsonRepository_;
            }
        };
    }
}
