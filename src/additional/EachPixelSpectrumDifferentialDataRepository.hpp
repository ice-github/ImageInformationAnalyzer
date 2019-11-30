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

        class EachPixelSpectrumDifferentialDataRepository : public IDifferentialDataRepository
        {
            enum
            {
                WINDOW_SIZE = 55//wide area feature detection
            };
        protected:
            std::tuple<double, double> GetBalanceCoefficient(const std::vector<ImageUtility::ImagePointBase>& window1, const std::vector<ImageUtility::ImagePointBase>& window2)
            {
                const auto windowBufferSize = window1.size();

                //Ax=c
                auto A11 = 0.0;
                auto A12 = 0.0;
                auto A21 = 0.0;
                auto A22 = (double)windowBufferSize;

                auto c1 = 0.0;
                auto c2 = 0.0;

                for(auto i = 0; i < windowBufferSize; i++)
                {
                    A11 += std::pow(window2[i].Value, 2);
                    A12 += window2[i].Value;

                    c1 += window2[i].Value * window1[i].Value;
                    c2 += window1[i].Value;
                }
                //ëŒèÃê´ÇÊÇË
                A21 = A12;

                //EigenÇ≈âÇ≠
                Eigen::Matrix2d A;
                A << A11, A12, A21, A22;
                Eigen::Vector2d c;
                c << c1, c2;

                Eigen::FullPivLU<Eigen::Matrix2d> lu(A);
                auto ab = lu.solve(c);

                std::tuple<double, double> result(ab.x(), ab.y());
                return result;
            }


        public:
            explicit EachPixelSpectrumDifferentialDataRepository() = default;
            virtual ~EachPixelSpectrumDifferentialDataRepository() = default;

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

                //Set to 0
                if(processedPixel != nullptr) *processedPixel = 0;

                std::vector<std::tuple<int, int, double, double, double>> processBuffer;
                processBuffer.resize((size_t)width * height);

                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        processBuffer[(size_t)y * width + x] = std::tuple(x, y, data1->ImageBuffer[y][x], data2->ImageBuffer[y][x], 0.0);
                    }
                }

            #ifdef _DEBUG
                auto parallelPolicy = std::execution::par;
            #else
                auto parallelPolicy = std::execution::par;
            #endif
                std::for_each(parallelPolicy, processBuffer.begin(), processBuffer.end(), [&](std::tuple<int, int, double, double, double> param)
                {
                    const auto x = std::get<0>(param);
                    const auto y = std::get<1>(param);
                    const auto image1Pixel = std::get<2>(param);
                    const auto image2Pixel = std::get<3>(param);

                    auto window1 = ImageUtility::GetWindowPoints<ImageUtility::ImagePointBase>(data1, x, y, WINDOW_SIZE);
                    auto window2 = ImageUtility::GetWindowPoints<ImageUtility::ImagePointBase>(data2, x, y, WINDOW_SIZE);

                    //2Ç¬ÇÃÉsÉNÉZÉãç∑ï™ {S1 - (a*S2 + b)}^2 Çç≈è¨âªÇ∑ÇÈåWêîa, bÇíTÇ∑
                    auto coef = GetBalanceCoefficient(window1, window2);

                    auto a = std::get<0>(coef);
                    auto b = std::get<1>(coef);

                    //Diff
                    std::get<4>(processBuffer[(size_t)y * width + x]) = ImageUtility::DoubleSub(image1Pixel, ImageUtility::DoubleAdd(a * image2Pixel, b));

                    //Next
                    if(processedPixel != nullptr) (*processedPixel)++;
                });

                //result
                for(std::tuple<int, int, double, double, double> param : processBuffer)
                {
                    const auto x = std::get<0>(param);
                    const auto y = std::get<1>(param);
                    const auto diffPixel = std::get<4>(param);
                    imageBuffer[y][x] = diffPixel;
                }

                return new FloatingPointImageData(width, height, imageBuffer, normalBuffer);
            }
        };
    }
}