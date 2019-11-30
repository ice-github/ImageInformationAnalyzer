#pragma once

#include "FloatingPointImageData.hpp"

#include <stdexcept>
#include <opencv2/opencv.hpp>

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;
        using namespace std::literals::string_literals;

        class GraphicFileDataRepository: public IImageFileDataRepository
        {
            inline std::vector<std::vector<double>> ReadCVMat(const cv::Mat& img, const IImageFileDataRepository::Channel channel)
            {
                if(static_cast<int>(channel) >= img.channels()) throw std::invalid_argument("channel not found!");

                std::vector<std::vector<double>> data;

                //Copy
                data.resize(img.rows);
                for(auto y = 0; y < img.rows; ++y)
                {
                    data[y].resize(img.cols);
                    for(auto x = 0; x < img.cols; ++x)
                    {
                        for(auto c = 0; c < img.channels(); ++c)
                        {
                            auto pix = img.data[y * img.step + x * img.elemSize() + c];

                            if(c == static_cast<int>(channel))
                            {
                                data[y][x] = (double)pix;
                            }
                        }
                    }
                }
                return data;
            }
            inline cv::Mat WriteCVMat(const int width, const int height, const std::vector<std::vector<double>>& r, const std::vector<std::vector<double>>& g, const std::vector<std::vector<double>>& b)
            {
                cv::Mat img(height, width, CV_8UC3);

                for(auto y = 0; y < img.rows; ++y)
                {
                    for(auto x = 0; x < img.cols; ++x)
                    {
                        //BGR
                        img.data[y * img.step + x * img.elemSize() + 0] = static_cast<uchar>(b[y][x] + 0.5);
                        img.data[y * img.step + x * img.elemSize() + 1] = static_cast<uchar>(g[y][x] + 0.5);
                        img.data[y * img.step + x * img.elemSize() + 2] = static_cast<uchar>(r[y][x] + 0.5);
                    }
                }

                return img;
            }


        public:
            explicit GraphicFileDataRepository() = default;
            virtual ~GraphicFileDataRepository() = default;

            virtual FloatingPointImageData* Load(const std::string& filePath, const IImageFileDataRepository::Channel channel) override
            { 
                //Read file
                auto imgMat = cv::imread(filePath);
                if(imgMat.empty()) throw std::invalid_argument("file not found!: "s + filePath);

                auto width = imgMat.cols;
                auto height = imgMat.rows;
                auto imageBuffer = ReadCVMat(imgMat, channel);

                //Write dummy normal
                std::vector<std::vector<Eigen::Vector3d>> normalBuffer;
                normalBuffer.resize(height);
                for(auto y = 0; y < height; ++y)
                {
                    normalBuffer[y].resize(width);
                    for(auto x = 0; x < width; ++x)
                    {
                        normalBuffer[y][x] = Eigen::Vector3d(0, 0, 1);//dummy
                    }
                }

                return new FloatingPointImageData(width, height, imageBuffer, normalBuffer);
            }

            virtual bool Store(const FloatingPointImageData* r, const FloatingPointImageData* g, const FloatingPointImageData* b, const std::string& filePath) override
            {
                auto width = r->Width;
                auto height = r->Height;

                if(width != g->Width || height != g->Height || width != b->Width || height != b->Height) throw std::invalid_argument("image size mismatched!");

                auto imgMat = WriteCVMat(width, height, r->ImageBuffer, g->ImageBuffer, b->ImageBuffer);

                //Write to file
                cv::imwrite(filePath, imgMat);

                return true;
            }

            virtual bool IsExtensionSupported(const std::string& extension) override
            {
                if(extension == "jpg"s || extension == "jpeg"s || extension == "png"s || extension == "bmp"s)
                {
                    return true;
                }
                return false;
            }
        };
    }
}