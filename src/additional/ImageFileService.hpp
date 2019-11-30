#pragma once

#include "GraphicFileDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;
        using namespace Infrastructure;

        class ImageFileService
        {
            IImageFileDataRepository* graphicRepository_;
            IImageFileDataRepository* jsonRepository_;

        public:
            explicit ImageFileService()
            {
                graphicRepository_ = new GraphicFileDataRepository();
                jsonRepository_ = nullptr;
            }

            virtual FloatingPointImageData* Load(const std::string& filePath, const IImageFileDataRepository::Channel channel)
            {
                auto extensionPos = filePath.rfind("."s);
                if(extensionPos == std::string::npos) throw std::invalid_argument("invalid image file!"s);

                auto extension = filePath.substr(extensionPos + 1);
                if(graphicRepository_->IsExtensionSupported(extension))
                {
                    return graphicRepository_->Load(filePath, channel);
                }

                //if(jsonRepository_->IsExtensionSupported(extension))
                //{
                //    return jsonRepository_->Load(filePath, channel);
                //}

                throw std::logic_error("file type not supported!"s);
            }
            virtual bool Store(const FloatingPointImageData* r, const FloatingPointImageData* g, const FloatingPointImageData* b, const std::string& filePath)
            {
                auto extensionPos = filePath.rfind("."s);
                if(extensionPos == std::string::npos) throw std::invalid_argument("invalid image file!"s);

                auto extension = filePath.substr(filePath.length() + 1);
                
                if(graphicRepository_->IsExtensionSupported(extension))
                {
                    return graphicRepository_->Store(r, g, b, filePath);
                }
                
                //if(jsonRepository_->IsExtensionSupported(extension))
                //{
                //    return jsonRepository_->Store(r, g, b, filePath);
                //}

                throw std::logic_error("file type not supported!"s);
            }

            virtual ~ImageFileService()
            {
                delete graphicRepository_;
                delete jsonRepository_;
            }
        };
    }
}
