#pragma once

#include "CircleDenoiseDataRepository.hpp"
#include "EllipseDenoiseDataRepository.hpp"
#include "HyperEllipseDenoiseDataRepository.hpp"

#include <thread>
#include <iomanip> //for cout

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;
        using namespace Infrastructure;

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
            explicit DenoiseImageService(Mode mode)
            {
                repository_ = nullptr;

                switch(mode)
                {
                    case Mode::CIRCLE:
                        repository_ = new CircleDenoiseDataRepository();
                        break;
                    case Mode::ELLIPSE:
                        repository_ = new EllipseDenoiseDataRepository();
                        break;
                    case Mode::HYPER_ELLIPSE:
                        repository_ = new HyperEllipseDenoiseDataRepository();
                        break;
                    default:
                        break;
                }
            }

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
                    std::this_thread::sleep_for(std::chrono::seconds(1));
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