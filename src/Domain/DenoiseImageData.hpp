#pragma once
#include "FloatingPointImageData.hpp"

#include <algorithm>
#include <execution>
#include <iostream> //std::cout

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        class IDenoiseImageDataRepository
        {
        protected:
            enum
            {
                WINDOW_SIZE = 7
            };

        public:
            explicit IDenoiseImageDataRepository()
            {

            }
            virtual ~IDenoiseImageDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data, std::atomic<int>* processedPixel = nullptr)
            {
                auto width = data->Width;
                auto height = data->Height;

                //Prepare buffers
                std::vector<std::vector<double>> imageBuffer;
                std::vector<std::vector<Eigen::Vector3d>> normalBuffer;
                imageBuffer.resize(height);
                normalBuffer.resize(height);
                for(auto y = 0; y < height; ++y)
                {
                    imageBuffer[y].resize(width);
                    normalBuffer[y].resize(width);
                }

                //set to 0%
                if(processedPixel != nullptr) *processedPixel = 0;

                //Denoise process
                std::vector<std::tuple<int, int, double, Eigen::Vector3d, double>> processBuffer;
                processBuffer.resize((size_t)width * height);

                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        processBuffer[(size_t)y * width + x] = std::tuple(x, y, 0.0, Eigen::Vector3d::Zero(), 0.0);
                    }
                }

                //C++17
                std::atomic<int> errorPixel(0);

            #ifdef _DEBUG
                auto parallelPolicy = std::execution::par;
            #else
                auto parallelPolicy = std::execution::par;
            #endif
                std::for_each(parallelPolicy, processBuffer.begin(), processBuffer.end(), [&](std::tuple<int, int, double, Eigen::Vector3d, double> param)
                {
                    const auto x = std::get<0>(param);
                    const auto y = std::get<1>(param);

                    auto denoisedPixel = 0.0;
                    auto fittingError = 0.0;
                    Eigen::Vector3d normal;

                    if(!DenoisePixel(data, x, y, WINDOW_SIZE, denoisedPixel, normal, fittingError))
                    {
                        errorPixel++;
                    }

                    //Results
                    {
                        std::get<2>(processBuffer[(size_t)y * width + x]) = denoisedPixel;
                        std::get<3>(processBuffer[(size_t)y * width + x]) = normal;
                        std::get<4>(processBuffer[(size_t)y * width + x]) = fittingError;

                        if(processedPixel != nullptr) (*processedPixel)++;
                    }
                });

                auto totalFittingError = 0.0;
                for(std::tuple<int, int, double, Eigen::Vector3d, double> param : processBuffer)
                {
                    const auto x = std::get<0>(param);
                    const auto y = std::get<1>(param);
                    const auto denoisedPixel = std::get<2>(param);
                    const auto normal = std::get<3>(param);
                    const auto fittingError = std::get<4>(param);
                    //const auto originalPixel = data->ImageBuffer[y][x];

                    totalFittingError += fittingError;
                    imageBuffer[y][x] = denoisedPixel;
                    normalBuffer[y][x] = normal;
                }
                std::cout << "Error Pixel: "s << errorPixel << std::endl;
                std::cout << "Fitting error/pixel: "s << totalFittingError / processBuffer.size() << std::endl;

                return new FloatingPointImageData(width, height, imageBuffer, normalBuffer);
            }
        protected:
            virtual inline bool DenoisePixel(const FloatingPointImageData* data, const int x, const int y, const int windowSize, double& denoisedPixel, Eigen::Vector3d& normal, double& fittingError) = 0;
        };
    }
}
