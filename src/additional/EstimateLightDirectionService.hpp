#pragma once

#include "PhongModelLightDirectionDataRepository.hpp"

#include <thread>
#include <iomanip> //for cout

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;
        using namespace Infrastructure;

        class EstimateLightDirectionService
        {
            ILightEstimationDataRepository* repository_;

        public:
            explicit EstimateLightDirectionService()
            {
                repository_ = new PhongModelLightDirectionDataRepository();
            }

            virtual FloatingPointImageData* Process(const FloatingPointImageData* denoisedR, const FloatingPointImageData* denoisedG, const FloatingPointImageData* denoisedB, const FloatingPointImageData* differentialB_G, const double pixelPitch)
            {
                if(denoisedR->Width != denoisedG->Width || denoisedR->Width != denoisedB->Width) throw std::invalid_argument("Input image sizes are not the same!");
                if(denoisedR->Height != denoisedG->Height || denoisedR->Height != denoisedB->Height) throw std::invalid_argument("Input image sizes are not the same!");

                std::atomic<double> progress(0);

                auto done = false;
                auto elapsedMillisecounds = 0ll;

                FloatingPointImageData* result = nullptr;

                std::thread thread([&]
                {
                    auto start = std::chrono::system_clock::now();
                    {
                        result = repository_->Process(denoisedR, denoisedG, denoisedB, differentialB_G, pixelPitch, &progress);
                    }
                    auto end = std::chrono::system_clock::now();
                    elapsedMillisecounds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    done = true;
                });

                while(!done)
                {
                #ifdef _DEBUG
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                #else
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                #endif

                    std::cout << "Progress: "s << std::setprecision(3) << 100.0 * progress << "%"s << std::endl;
                }
                thread.join();

                std::cout << "Light estimation completed: "s << elapsedMillisecounds << "ms"s << std::endl;

                return result;
            }

            virtual ~EstimateLightDirectionService()
            {
                delete repository_;
            }
        };
    }
}