#pragma once

#include "DenoiseImageService.hpp"
#include "ImageFileService.hpp"
#include "TakeHistogramService.hpp"
#include "ImageEvaluationService.hpp"
#include "ScaleImageService.hpp"

#include "ImageInformationModel.hpp"

#include <opencv2/opencv.hpp>
#include <string>
#include <stdexcept>
#include <fstream>

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

            ImageInformationModel model_;

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

        public:
            explicit ImageInformationPresenter(DenoiseImageService::Mode denoiseMode, ImageEvaluationService::Mode evaluationMode) : denoiseService_(denoiseMode), imageEvaluationService_(evaluationMode)
            {

            }

            bool LoadImage(const std::string& filePath)
            {
                auto b = imageFileService_.Load(filePath, IImageFileDataRepository::Channel::B);
                auto g = imageFileService_.Load(filePath, IImageFileDataRepository::Channel::G);
                auto r = imageFileService_.Load(filePath, IImageFileDataRepository::Channel::R);

                if(r == nullptr || g == nullptr || b == nullptr) return false;

                model_.R = r;
                model_.G = g;
                model_.B = b;

                return true;
            }

            bool Scale()
            {
                if(model_.R == nullptr || model_.G == nullptr || model_.B == nullptr) return false;

                auto scaledR = scaleImageService_.Process(model_.R, 0.0, 255.0, 0.0, 1.0);
                auto scaledG = scaleImageService_.Process(model_.G, 0.0, 255.0, 0.0, 1.0);
                auto scaledB = scaleImageService_.Process(model_.B, 0.0, 255.0, 0.0, 1.0);

                delete model_.R;
                delete model_.G;
                delete model_.B;

                model_.R = scaledR;
                model_.G = scaledG;
                model_.B = scaledB;

                return true;
            }

            bool DenoiseImage()
            {
                if(model_.R == nullptr || model_.G == nullptr || model_.B == nullptr) return false;

                auto denoisedR = denoiseService_.Process(model_.R);
                auto denoisedG = denoiseService_.Process(model_.G);
                auto denoisedB = denoiseService_.Process(model_.B);

                if(denoisedR == nullptr || denoisedG == nullptr || denoisedB == nullptr) return false;

                model_.DenoisedR = denoisedR;
                model_.DenoisedG = denoisedG;
                model_.DenoisedB = denoisedB;

                return true;
            }

            bool TakeHistogram()
            {
                if(model_.R == nullptr || model_.G == nullptr || model_.B == nullptr) return false;
                if(model_.DenoisedR == nullptr || model_.DenoisedG == nullptr || model_.DenoisedB == nullptr) return false;

                const auto histogramSize = 1024;//256以外だとRGBイメージは間がスカスカになる

                auto histogramR = takeHistogramService_.Process(model_.R, histogramSize, 0, 255);
                auto histogramG = takeHistogramService_.Process(model_.G, histogramSize, 0, 255);
                auto histogramB = takeHistogramService_.Process(model_.B, histogramSize, 0, 255);

                auto histogramDenoisedR = takeHistogramService_.Process(model_.DenoisedR, histogramSize, 0, 255);
                auto histogramDenoisedG = takeHistogramService_.Process(model_.DenoisedG, histogramSize, 0, 255);
                auto histogramDenoisedB = takeHistogramService_.Process(model_.DenoisedB, histogramSize, 0, 255);

                std::ofstream ofs("histogram.csv");
                ofs << ", R, R', G, G', B, B'"s << std::endl;
                for(auto i = 0; i < histogramSize; ++i)
                {
                    ofs << std::to_string(i) << ", "s;
                    ofs << histogramR->Data[i] << ", "s << histogramDenoisedR->Data[i] << ", "s;
                    ofs << histogramG->Data[i] << ", "s << histogramDenoisedG->Data[i] << ", "s;
                    ofs << histogramB->Data[i] << ", "s << histogramDenoisedB->Data[i] << std::endl;
                }
                ofs.close();

                return true;
            }

            bool Evaluate()
            {
                if(model_.R == nullptr || model_.G == nullptr || model_.B == nullptr) return false;
                if(model_.DenoisedR == nullptr || model_.DenoisedG == nullptr || model_.DenoisedB == nullptr) return false;

                //denoisedをoriginalだとして計算

                auto resultR = imageEvaluationService_.Process(model_.DenoisedR, model_.R, 1.0);
                auto resultG = imageEvaluationService_.Process(model_.DenoisedG, model_.G, 1.0);
                auto resultB = imageEvaluationService_.Process(model_.DenoisedB, model_.B, 1.0);


                std::cout << std::setprecision(15) << "Result R: "s << resultR << std::endl;
                std::cout << std::setprecision(15) << "Result G: "s << resultG << std::endl;
                std::cout << std::setprecision(15) << "Result B: "s << resultB << std::endl;

                return true;
            }

            void Show()
            {
                cv::Mat R, G, B;
                cv::Mat denoisedR, denoisedG, denoisedB;

                if(model_.R) R = Convert(model_.R);
                if(model_.G) G = Convert(model_.G);
                if(model_.B) B = Convert(model_.B);
                if(model_.DenoisedR) denoisedR = Convert(model_.DenoisedR);
                if(model_.DenoisedG) denoisedG = Convert(model_.DenoisedG);
                if(model_.DenoisedB) denoisedB = Convert(model_.DenoisedB);

                while('q' != cv::waitKey(0))
                {
                    if(!R.empty()) cv::imshow("R", R);
                    if(!G.empty()) cv::imshow("G", G);
                    if(!B.empty()) cv::imshow("B", B);

                    if(!denoisedR.empty()) cv::imshow("Denoised R", denoisedR);
                    if(!denoisedG.empty()) cv::imshow("Denoised G", denoisedG);
                    if(!denoisedB.empty()) cv::imshow("Denoised B", denoisedB);
                }

            }

            virtual ~ImageInformationPresenter()
            {
                if(model_.R) delete model_.R;
                if(model_.G) delete model_.G;
                if(model_.B) delete model_.B;
                if(model_.DenoisedR) delete model_.DenoisedR;
                if(model_.DenoisedG) delete model_.DenoisedG;
                if(model_.DenoisedB) delete model_.DenoisedB;
            }
        };
    }
}
