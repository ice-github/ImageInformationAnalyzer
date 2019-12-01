#pragma once

#include "DenoiseImageService.hpp"
#include "ImageFileService.hpp"
#include "TakeHistogramService.hpp"
#include "ImageEvaluationService.hpp"
#include "ScaleImageService.hpp"
#include "TakeDifferenceService.hpp"
#include "EstimateLightDirectionService.hpp"

#include "ImageInformationModel.hpp"

#include <opencv2/opencv.hpp>

#define CVUI_IMPLEMENTATION
#include <cvui.h>

#include <string>
#include <stdexcept>
#include <fstream>
#include <map>

#include <algorithm>
#include <iostream> //std::cout

namespace ImageInformationAnalyzer
{
    namespace Presentation
    {
        using namespace Application;

        class ImageInformationPresenter
        {
            DenoiseImageService denoiseService_;
            ImageFileService imageFileService_;
            TakeHistogramService takeHistogramService_;
            ImageEvaluationService imageEvaluationService_;
            ScaleImageService scaleImageService_;
            TakeDifferenceService takeDifferenceService_;
            EstimateLightDirectionService estimateLightDirectionService_;


            ImageInformationModel model_;

            cv::Mat Convert(const FloatingPointImageData* R, const FloatingPointImageData* G, const FloatingPointImageData* B)
            {
                auto width = R->Width;
                auto height = R->Height;

                cv::Mat output(height, width, CV_64FC3);

                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        auto b = &output.data[y * output.step + x * output.elemSize() + 0];
                        auto g = &output.data[y * output.step + x * output.elemSize() + 8];
                        auto r = &output.data[y * output.step + x * output.elemSize() + 16];

                        *((double*)r) = R->ImageBuffer[y][x];
                        *((double*)g) = G->ImageBuffer[y][x];
                        *((double*)b) = B->ImageBuffer[y][x];
                    }
                }
                return output;
            }

            cv::Mat Convert(const FloatingPointImageData* data)
            {
                cv::Mat output(data->Height, data->Width, CV_64FC1);

                for(auto y = 0; y < data->Height; ++y)
                {
                    for(auto x = 0; x < data->Width; ++x)
                    {
                        auto p = &output.data[y * output.step + x * output.elemSize()];

                        *((double*)p) = data->ImageBuffer[y][x];
                    }
                }
                return output;
            }

            cv::Mat Histogram(const HistogramData* data)
            {
                auto width = (int)data->Data.size();
                auto height = (int)data->Data.size();

                cv::Mat output(height, width, CV_64FC1);

                for(auto i = 0; i < width; ++i)
                {
                    cv::line(output, cv::Point(i, height), cv::Point(i, height - static_cast<int>(data->Data[i] / data->GetMaxValue() * height + 0.5)), cv::Scalar(1, 1, 1, 1));
                }
                return output;
            }


            bool DifferentialSettingWindow(const std::string& windowName, const int histogramSize, const double minValue, const double maxValue, double& currentMinValue, double& currentMaxValue, cv::Mat& frame)
            {
                auto buttonPressed = false;

                //clear
                frame = cv::Scalar(49, 52, 49);

                if(cvui::button(frame, (histogramSize + 20) / 2 - 30, 20, "Apply"))
                {
                    buttonPressed = true;
                }

                cvui::text(frame, 10, 60, "Min value");
                cvui::trackbar<double>(frame, 10, 70, histogramSize, &currentMinValue, minValue, maxValue, 1, "%.3Lf");

                cvui::text(frame, 10, 130, "Max value");
                cvui::trackbar<double>(frame, 10, 140, histogramSize, &currentMaxValue, minValue, maxValue, 1, "%.3Lf");

                cvui::update(windowName);

                return buttonPressed;
            }           

        public:
            explicit ImageInformationPresenter(DenoiseImageService::Mode denoiseMode, ImageEvaluationService::Mode evaluationMode, TakeDifferenceService::Mode diffMode) : denoiseService_(denoiseMode), imageEvaluationService_(evaluationMode), takeDifferenceService_(diffMode)
            {

            }

            bool LoadImage(const std::string& filePath)
            {
                auto b = imageFileService_.Load(filePath, IImageFileDataRepository::Channel::B);
                auto g = imageFileService_.Load(filePath, IImageFileDataRepository::Channel::G);
                auto r = imageFileService_.Load(filePath, IImageFileDataRepository::Channel::R);

                if(r == nullptr || g == nullptr || b == nullptr) return false;

                model_.R.reset(r);
                model_.G.reset(g);
                model_.B.reset(b);

                return true;
            }

            bool Scale()
            {
                if(model_.R == nullptr || model_.G == nullptr || model_.B == nullptr) throw std::logic_error("RGB images don't exist");

                auto scaledR = scaleImageService_.Process(model_.R.get(), 0.0, 255.0, 0.0, 1.0);
                auto scaledG = scaleImageService_.Process(model_.G.get(), 0.0, 255.0, 0.0, 1.0);
                auto scaledB = scaleImageService_.Process(model_.B.get(), 0.0, 255.0, 0.0, 1.0);

                model_.R.release();
                model_.G.release();
                model_.B.release();

                if(scaledR == nullptr || scaledG == nullptr || scaledB == nullptr) throw std::logic_error("Failed to scale");

                model_.R.reset(scaledR);
                model_.G.reset(scaledG);
                model_.B.reset(scaledB);

                return true;
            }

            bool DenoiseImage()
            {
                if(model_.R == nullptr || model_.G == nullptr || model_.B == nullptr) throw std::logic_error("RGB images don't exist");

                auto denoisedR = denoiseService_.Process(model_.R.get());
                auto denoisedG = denoiseService_.Process(model_.G.get());
                auto denoisedB = denoiseService_.Process(model_.B.get());

                if(denoisedR == nullptr || denoisedG == nullptr || denoisedB == nullptr) throw std::logic_error("Dailed to denoise");

                model_.DenoisedR.reset(denoisedR);
                model_.DenoisedG.reset(denoisedG);
                model_.DenoisedB.reset(denoisedB);

                return true;
            }

            bool DenoiseDenoisedImage()
            {
                if(model_.DenoisedR == nullptr || model_.DenoisedG == nullptr || model_.DenoisedB == nullptr) throw std::logic_error("Denoised RGB images don't exist");

                auto denoisedR = denoiseService_.Process(model_.DenoisedR.get());
                auto denoisedG = denoiseService_.Process(model_.DenoisedG.get());
                auto denoisedB = denoiseService_.Process(model_.DenoisedB.get());

                model_.DenoisedR.release();
                model_.DenoisedG.release();
                model_.DenoisedB.release();

                if(denoisedR == nullptr || denoisedG == nullptr || denoisedB == nullptr)  throw std::logic_error("Failed to denoise");

                model_.DenoisedR.reset(denoisedR);
                model_.DenoisedG.reset(denoisedG);
                model_.DenoisedB.reset(denoisedB);

                return true;
            }


            bool Evaluate()
            {
                if(model_.R == nullptr || model_.G == nullptr || model_.B == nullptr) throw std::logic_error("RGB images don't exist");
                if(model_.DenoisedR == nullptr || model_.DenoisedG == nullptr || model_.DenoisedB == nullptr) throw std::logic_error("Denoised RGB images don't exist");

                //denoisedをoriginalだとして計算

                auto resultR = imageEvaluationService_.Process(model_.DenoisedR.get(), model_.R.get(), 1.0);
                auto resultG = imageEvaluationService_.Process(model_.DenoisedG.get(), model_.G.get(), 1.0);
                auto resultB = imageEvaluationService_.Process(model_.DenoisedB.get(), model_.B.get(), 1.0);

                model_.EvaluatedR.reset(resultR);
                model_.EvaluatedG.reset(resultG);
                model_.EvaluatedB.reset(resultB);

                std::cout << std::setprecision(15) << "Result R: "s << resultR->Result << std::endl;
                std::cout << std::setprecision(15) << "Result G: "s << resultG->Result << std::endl;
                std::cout << std::setprecision(15) << "Result B: "s << resultB->Result << std::endl;

                return true;
            }

            bool Diff()
            {
                if(model_.DenoisedR == nullptr || model_.DenoisedG == nullptr || model_.DenoisedB == nullptr) throw std::logic_error("Denoised RGB images don't exist");

                auto differentialB_G = takeDifferenceService_.Process(model_.DenoisedB.get(), model_.DenoisedG.get());
                auto differentialG_R = takeDifferenceService_.Process(model_.DenoisedG.get(), model_.DenoisedR.get());
                auto differentialB_R = takeDifferenceService_.Process(model_.DenoisedB.get(), model_.DenoisedR.get());

                if(differentialB_G == nullptr || differentialG_R == nullptr || differentialB_R == nullptr) throw std::logic_error("Failed to take image differentials");

                model_.DifferentialB_G.reset(differentialB_G);
                model_.DifferentialG_R.reset(differentialG_R);
                model_.DifferentialB_R.reset(differentialB_R);//輝度の高い部分が血管

                return true;
            }

            bool LightEstimation()
            {
                if(model_.DenoisedR == nullptr || model_.DenoisedG == nullptr || model_.DenoisedB == nullptr) throw std::logic_error("Denoised RGB images don't exist");
                if(model_.DifferentialB_G == nullptr || model_.DifferentialG_R == nullptr || model_.DifferentialB_R == nullptr) throw std::logic_error("Differential images don's exist");

                const double pixelPitch = 0.001 * 0.01; //0.01mm

                auto surface = estimateLightDirectionService_.Process(model_.DenoisedR.get(), model_.DenoisedB.get(), model_.DenoisedB.get(), model_.DifferentialB_G.get(), pixelPitch);

                if(surface == nullptr) throw std::logic_error("Failed to estimate light direction");
                model_.Surface.reset(surface);

                return true;
            }

            bool TakeHistogram()
            {
                const auto histogramSize = 512;//256以外だとRGBイメージは間がスカスカになる

                if(model_.R != nullptr && model_.G != nullptr && model_.B != nullptr)
                {
                    auto histogramR = takeHistogramService_.Process(model_.R.get(), histogramSize, 0.0, 1.0);
                    auto histogramG = takeHistogramService_.Process(model_.G.get(), histogramSize, 0.0, 1.0);
                    auto histogramB = takeHistogramService_.Process(model_.B.get(), histogramSize, 0.0, 1.0);

                    model_.HistogramR.reset(histogramR);
                    model_.HistogramG.reset(histogramG);
                    model_.HistogramB.reset(histogramB);
                }

                if(model_.DenoisedR != nullptr && model_.DenoisedG != nullptr && model_.DenoisedB != nullptr)
                {
                    auto histogramDenoisedR = takeHistogramService_.Process(model_.DenoisedR.get(), histogramSize, 0.0, 1.0);
                    auto histogramDenoisedG = takeHistogramService_.Process(model_.DenoisedG.get(), histogramSize, 0.0, 1.0);
                    auto histogramDenoisedB = takeHistogramService_.Process(model_.DenoisedB.get(), histogramSize, 0.0, 1.0);

                    model_.HistogramDenoisedR.reset(histogramDenoisedR);
                    model_.HistogramDenoisedG.reset(histogramDenoisedG);
                    model_.HistogramDenoisedB.reset(histogramDenoisedB);
                }

                if(model_.DifferentialB_G != nullptr && model_.DifferentialG_R != nullptr && model_.DifferentialB_R != nullptr)
                {
                    auto histogramB_G = takeHistogramService_.Process(model_.DifferentialB_G.get(), histogramSize, model_.DifferentialB_G->GetMinValue(), model_.DifferentialB_G->GetMaxValue());
                    auto histogramG_R = takeHistogramService_.Process(model_.DifferentialG_R.get(), histogramSize, model_.DifferentialG_R->GetMinValue(), model_.DifferentialG_R->GetMaxValue());
                    auto histogramB_R = takeHistogramService_.Process(model_.DifferentialB_R.get(), histogramSize, model_.DifferentialB_R->GetMinValue(), model_.DifferentialB_R->GetMaxValue());

                    model_.HistogramB_G.reset(histogramB_G);
                    model_.HistogramG_R.reset(histogramG_R);
                    model_.HistogramB_R.reset(histogramB_R);
                }
                if(model_.Surface != nullptr)
                {
                    auto histogramSurface = takeHistogramService_.Process(model_.Surface.get(), histogramSize, model_.Surface->GetMinValue(), model_.Surface->GetMaxValue());
                    model_.HistogramSurface.reset(histogramSurface);
                }

                return true;
            }

            void Show()
            {
                //Create Mat data
                cv::Mat RGB;
                if(model_.R && model_.G && model_.B) RGB = Convert(model_.R.get(), model_.G.get(), model_.B.get());

                cv::Mat differentialB_G, differentialG_R, differentialB_R;
                if(model_.DifferentialB_G) differentialB_G = Convert(model_.DifferentialB_G.get());
                if(model_.DifferentialG_R) differentialG_R = Convert(model_.DifferentialG_R.get());
                if(model_.DifferentialB_R) differentialB_R = Convert(model_.DifferentialB_R.get());

                cv::Mat histogramB_G, histogramG_R, histogramB_R, histogramSurface;
                if(model_.HistogramB_G) histogramB_G = Histogram(model_.HistogramB_G.get());
                if(model_.HistogramG_R) histogramG_R = Histogram(model_.HistogramG_R.get());
                if(model_.HistogramB_R) histogramB_R = Histogram(model_.HistogramB_R.get());
                if(model_.HistogramSurface) histogramSurface = Histogram(model_.HistogramSurface.get());

                cv::Mat surface;
                if(model_.Surface) surface = Convert(model_.Surface.get());

                cv::Mat B_GSetting, G_RSetting, B_RSetting, surfaceSetting;
                const auto histogramSize = (int)model_.HistogramR->Data.size();
                if(model_.HistogramB_G) B_GSetting = cv::Mat(200, histogramSize + 20, CV_8UC3);
                if(model_.HistogramG_R) G_RSetting = cv::Mat(200, histogramSize + 20, CV_8UC3);
                if(model_.HistogramB_R) B_RSetting = cv::Mat(200, histogramSize + 20, CV_8UC3);
                if(model_.HistogramSurface) surfaceSetting = cv::Mat(200, histogramSize + 20, CV_8UC3);

                //Setting parameters
                auto B_GSettingMinValue = model_.DifferentialB_G ? model_.DifferentialB_G->GetMinValue() : 0.0;
                auto B_GSettingMaxValue = model_.DifferentialB_G ? model_.DifferentialB_G->GetMaxValue() : 0.0;
                auto G_RSettingMinValue = model_.DifferentialG_R ? model_.DifferentialG_R->GetMinValue() : 0.0;
                auto G_RSettingMaxValue = model_.DifferentialG_R ? model_.DifferentialG_R->GetMaxValue() : 0.0;
                auto B_RSettingMinValue = model_.DifferentialB_R ? model_.DifferentialB_R->GetMinValue() : 0.0;
                auto B_RSettingMaxValue = model_.DifferentialB_R ? model_.DifferentialB_R->GetMaxValue() : 0.0;
                auto surfaceSettingMinValue = model_.Surface ? model_.Surface->GetMinValue() : 0.0;
                auto surfaceSettingMaxValue = model_.Surface ? model_.Surface->GetMaxValue() : 0.0;

                //ウインドウの登録
                std::vector<std::tuple<std::string, cv::Mat*, FloatingPointImageData*, cv::Mat*, double*, double*>> windows;
                windows.push_back(std::tuple("RGB", &RGB, nullptr, nullptr, nullptr, nullptr));
                //windows.push_back(std::tuple("Differential B-G", &differentialB_G, nullptr, nullptr, nullptr, nullptr));
                //windows.push_back(std::tuple("Differential G-R", &differentialG_R, nullptr, nullptr, nullptr, nullptr));
                windows.push_back(std::tuple("Differential B-R", &differentialB_R, nullptr, nullptr, nullptr, nullptr));//Blood vessel
                //windows.push_back(std::tuple("Histogram B-G", &histogramB_G, nullptr, nullptr, nullptr, nullptr));
                //windows.push_back(std::tuple("Histogram G-R", &histogramG_R, nullptr, nullptr, nullptr, nullptr));
                windows.push_back(std::tuple("Histogram B-R", &histogramB_R, nullptr, nullptr, nullptr, nullptr));
                windows.push_back(std::tuple("Light Estimated Surface", &surface, nullptr, nullptr, nullptr, nullptr));
                windows.push_back(std::tuple("Histogram Surface", &histogramSurface, nullptr, nullptr, nullptr, nullptr));
                //windows.push_back(std::tuple("B-G Setting", &B_GSetting, model_.DifferentialB_G.get(), &differentialB_G, &B_GSettingMinValue, &B_GSettingMaxValue));
                //windows.push_back(std::tuple("G-R Setting", &G_RSetting, model_.DifferentialG_R.get(), &differentialG_R, &G_RSettingMinValue, &G_RSettingMaxValue));
                windows.push_back(std::tuple("B-R Setting", &B_RSetting, model_.DifferentialB_R.get(), &differentialB_R, &B_RSettingMinValue, &B_RSettingMaxValue));
                windows.push_back(std::tuple("Surface Setting", &surfaceSetting, model_.Surface.get(), &surface, &surfaceSettingMinValue, &surfaceSettingMaxValue));

                auto windowNames = new std::string[windows.size()];
                auto windowIndex = 0;
                for(auto window : windows)
                {
                    auto windowSize = std::get<0>(window);
                    auto frame = std::get<1>(window);
                    if(frame->empty()) continue;

                    windowNames[windowIndex++] = windowSize;
                }

                cvui::init(windowNames, windowIndex);

                while(cv::waitKey(20) != 'q')
                {
                    for(auto window : windows)
                    {
                        auto windowName = std::get<0>(window);
                        auto frame = std::get<1>(window);

                        if(frame->empty()) continue;

                        cvui::context(windowName);

                        if(windowName.find("Setting"s) != std::string::npos)
                        {
                            auto imageData = std::get<2>(window);
                            auto targetMat = std::get<3>(window);
                            auto minValue = std::get<4>(window);
                            auto maxValue = std::get<5>(window);

                            if(imageData == nullptr) continue;

                            if(DifferentialSettingWindow(windowName, histogramSize, imageData->GetMinValue(), imageData->GetMaxValue(), *minValue, *maxValue, *frame))
                            {
                                //画像の更新
                                auto scaledImage = scaleImageService_.Process(imageData, *minValue, *maxValue, 0.0, 1.0);
                                {
                                    auto scaledMat = Convert(scaledImage);
                                    scaledMat.copyTo(*targetMat);
                                }
                                delete scaledImage;
                            }
                        }

                        cvui::imshow(windowName, *frame);
                    }
                }
            }


            virtual ~ImageInformationPresenter() = default;
        };
    }
}
