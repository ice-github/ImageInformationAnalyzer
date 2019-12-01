#pragma once

#include "FloatingPointImageData.hpp"
#include "EachPixelSpectrumDifferentialDataRepository.hpp"

#include <stdexcept>
#include <opencv2/opencv.hpp>

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;
        using namespace std::literals::string_literals;

        class WholePixelSpectrumDifferentialDataRepository : public EachPixelSpectrumDifferentialDataRepository
        {
        public:
            explicit WholePixelSpectrumDifferentialDataRepository() = default;
            virtual ~WholePixelSpectrumDifferentialDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2, std::atomic<int>* processedPixel = nullptr) override
            {
                auto width = data1->Width;
                auto height = data1->Height;

                //Prepare buffers
                std::vector<std::vector<double>> imageBuffer;
                std::vector<std::vector<Eigen::Vector3d>> normalBuffer;
                imageBuffer.resize(height);
                normalBuffer.resize(height);
                for(auto y = 0; y < height; ++y)
                {
                    imageBuffer[y].resize(width);
                    normalBuffer[y].resize(width);
                    for(auto x = 0; x < width; ++x)
                    {
                        normalBuffer[y][x] = Eigen::Vector3d(0, 0, 1);//dummy
                    }
                }

                //Gather big window pixels
                auto windowSize = width > height ? height : width;
                windowSize = windowSize % 2 == 0 ? windowSize - 1 : windowSize;//must be odd
                auto centerX = width / 2 + 1;
                auto centerY = height / 2 + 1;
                auto window1 = ImageUtility::GetWindowPoints<ImageUtility::ImagePointBase>(data1, centerX, centerY, windowSize);
                auto window2 = ImageUtility::GetWindowPoints<ImageUtility::ImagePointBase>(data2, centerX, centerY, windowSize);

                //2Ç¬ÇÃÉsÉNÉZÉãç∑ï™ {S1 - (a*S2 + b)}^2 Çç≈è¨âªÇ∑ÇÈåWêîa, bÇíTÇ∑
                auto coef = GetBalanceCoefficient(window1, window2);

                auto a = std::get<0>(coef);
                auto b = std::get<1>(coef);

                //Set to 0
                if(processedPixel != nullptr) *processedPixel = 0;

                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        auto image1Pixel = data1->ImageBuffer[y][x];
                        auto image2Pixel = data2->ImageBuffer[y][x];

                        //diff
                        imageBuffer[y][x] = ImageUtility::DoubleSub(image1Pixel, ImageUtility::DoubleAdd(a * image2Pixel, b));

                        //Next
                        if(processedPixel != nullptr) (*processedPixel)++;
                    }
                }

                return new FloatingPointImageData(width, height, imageBuffer, normalBuffer);
            }
        };
    }
}