
#include "ImageFileService.hpp"
#include "GraphicFileDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Infrastructure;

        ImageFileService::ImageFileService()
        {
            graphicRepository_ = new GraphicFileDataRepository();
            jsonRepository_ = nullptr;
        }
    }
}