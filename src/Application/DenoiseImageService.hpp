#pragma once

#include "DenoiseImageData.hpp"

#include <thread>
#include <iomanip> //for cout

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;

        class DenoiseImageService
        {
        public:
            enum class Mode
            {
                CIRCLE,
                ELLIPSE,
                HYPER_ELLIPSE
            };

            IDenoiseImageDataRepository* repository_;

        public:
            explicit DenoiseImageService(Mode mode);

            FloatingPointImageData* Process(const FloatingPointImageData* data)
            {
                std::atomic<int> processedPixel(0);

                auto done = false;
                auto elapsedMillisecounds = 0ll;

                FloatingPointImageData* result = nullptr;

                std::thread thread([&]
                {
                    auto start = std::chrono::system_clock::now();
                    {
                        result = repository_->Process(data, &processedPixel);
                    }
                    auto end = std::chrono::system_clock::now();
                    elapsedMillisecounds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    done = true;
                });

                while(!done)
                {
                #ifdef _DEBUG
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                #else
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                #endif

                    std::cout << "Progress: "s << std::setprecision(3) << 100.0 * processedPixel / ((double)data->Width * data->Height) << "%"s << std::endl;
                }
                thread.join();

                std::cout << "Denoise completed: "s << elapsedMillisecounds << "ms"s << std::endl;

                return result;
            }

            virtual ~DenoiseImageService()
            {
                delete repository_;
            }
        };
    }
}