#pragma once

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <execution>
#include <iostream> //std::cout
#include <fstream>
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Dense>

namespace ImageInformationAnalyzer
{
    namespace Domain
    {
        using namespace std::literals::string_literals;

        class FloatingPointImageData
        {
        public:
            const int Width;
            const int Height;
            const std::vector<std::vector<double>> ImageBuffer;
            const std::vector<std::vector<Eigen::Vector3d>> NormalBuffer;

        private:
            double maxValue_;
            double minValue_;

        public:
            explicit FloatingPointImageData(const int width, const int height, const std::vector<std::vector<double>> imageBuffer, const std::vector<std::vector<Eigen::Vector3d>> normalBuffer);

            inline double GetMaxValue() const { return maxValue_; }
            inline double GetMinValue() const { return minValue_; }

            virtual ~FloatingPointImageData() = default;
        };

        class HistogramData
        {
            double maxValue_;
            double minValue_;

        public:
            const std::vector<double> Data;

            explicit HistogramData(std::vector<double> data) : Data(data)
            {
                maxValue_ = DBL_MIN;
                minValue_ = DBL_MAX;

                for(auto value : data)
                {
                    if(value > maxValue_)
                    {
                        maxValue_ = value;
                    }
                    if(value < minValue_)
                    {
                        minValue_ = value;
                    }
                }
            }

            inline double GetMaxValue() const { return maxValue_; }
            inline double GetMinValue() const { return minValue_; }

            virtual ~HistogramData() = default;
        };

        class ImageEvaluationData
        {
        public:
            const double Result;

            explicit ImageEvaluationData(const double& result) : Result(result)
            {

            }
            virtual ~ImageEvaluationData() = default;
        };

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

        class IHistogramDataRepository
        {
        public:
            explicit IHistogramDataRepository() = default;
            virtual ~IHistogramDataRepository() = default;

            virtual HistogramData* Process(const FloatingPointImageData* data, const int histogramSize, const double histogramMinValue, const double histogramMaxValue) = 0;
        };

        class IScaleImageDataRepository
        {
        public:
            explicit IScaleImageDataRepository() = default;
            virtual ~IScaleImageDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data, const double oldMinValue, const double oldMaxValue, const double newMinValue, const double newMaxValue) = 0;
        };

        class IImageEvaluationDataRepository
        {
        public:
            explicit IImageEvaluationDataRepository() = default;
            virtual ~IImageEvaluationDataRepository() = default;

            virtual ImageEvaluationData* Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2, const double maxValue) = 0;
        };

        class IDifferentialDataRepository
        {
        public:
            explicit IDifferentialDataRepository() = default;
            virtual ~IDifferentialDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2, std::atomic<int>* processedPixel = nullptr) = 0;
        };

        class ILightEstimationDataRepository
        {
        public:
            explicit ILightEstimationDataRepository() = default;
            virtual ~ILightEstimationDataRepository() = default;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* denoisedR, const FloatingPointImageData* denoisedG, const FloatingPointImageData* denoisedB, const FloatingPointImageData* differentialB_G, const double pixelPitch, std::atomic<double>* progress = nullptr) = 0;
        };


        //Utility
        class ImageUtility
        {
        public:
            struct ImagePointBase
            {
                int X;
                int Y;
                int OffsetX;
                int OffsetY;
                double Value;
            };

            template<typename ImagePoint>
            static inline std::vector<ImagePoint> GetWindowPoints(const int windowSize)
            {
                //範囲チェック
                if(windowSize < 5 || windowSize % 2 == 0)
                {
                    throw std::invalid_argument("window size must be greater than 5 and odd");
                }

                std::vector<ImagePoint> points;
                for(auto offsetY = -windowSize / 2; offsetY < windowSize / 2 + 1; ++offsetY)
                {
                    for(auto offsetX = -windowSize / 2; offsetX < windowSize / 2 + 1; ++offsetX)
                    {
                        points.push_back(
                            {
                                0,
                                0,
                                offsetX,
                                offsetY,
                                0
                            });
                    }
                }
                return points;
            }

            template<typename ImagePoint>
            static inline std::vector<ImagePoint> GetWindowPoints(const FloatingPointImageData* data, const int x, const int y, const int windowSize)
            {
                auto points = GetWindowPoints<ImagePoint>(windowSize);

                auto width = data->Width;
                auto height = data->Height;

                for(auto i = 0; i < points.size(); ++i)
                {
                    auto targetX = (x + width + points[i].OffsetX) % width;
                    auto targetY = (y + height + points[i].OffsetY) % height;

                    points[i].X = targetX;
                    points[i].Y = targetY;
                    points[i].Value = data->ImageBuffer[targetY][targetX];
                }

                return points;
            }

            static inline double DoubleAdd(double a, double b)
            {
                auto sub = a - b;
                if(std::abs(sub) < 1e-10) return a + b;

                return (a * a - b * b) / (a - b);
            }

            static inline double DoubleSub(double a, double b)
            {
                auto add = a + b;
                if(std::abs(add) < 1e-10) return a - b;

                return (a * a - b * b) / (a + b);
            }

        };

    }
}
