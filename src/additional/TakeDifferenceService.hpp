#pragma once

#include "EachPixelSpectrumDifferentialDataRepository.hpp"
#include "WholePixelSpectrumDifferentialDataRepository.hpp"

namespace ImageInformationAnalyzer
{
    namespace Application
    {
        using namespace Domain;
        using namespace Infrastructure;

        class TakeDifferenceService
        {
            IDifferentialDataRepository* repository_;

        public:
            enum class Mode
            {
                EachPixel, //very slow
                WholePixel
            };

            explicit TakeDifferenceService(Mode mode)
            {
                repository_ = nullptr;
                switch(mode)
                {
                    case Mode::EachPixel:
                        repository_ = new EachPixelSpectrumDifferentialDataRepository();
                        break;
                    case Mode::WholePixel:
                        repository_ = new WholePixelSpectrumDifferentialDataRepository();
                        break;
                }

            }

            virtual FloatingPointImageData* Process(const FloatingPointImageData* data1, const FloatingPointImageData* data2)
            {
                if(data1->Width != data2->Width || data1->Height != data2->Height)
                {
                    throw std::invalid_argument("Image sizes are NOT the same!");
                }

                std::atomic<int> processedPixel(0);

                auto done = false;
                auto elapsedMillisecounds = 0ll;

                FloatingPointImageData* result = nullptr;

                std::thread thread([&]
                {
                    auto start = std::chrono::system_clock::now();
                    {
                        result = repository_->Process(data1, data2, &processedPixel);
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

                    std::cout << "Progress: "s << std::setprecision(3) << 100.0 * processedPixel / ((double)data1->Width * data1->Height) << "%"s << std::endl;
                }
                thread.join();

                std::cout << "Take image differential completed: "s << elapsedMillisecounds << "ms"s << std::endl;

                return repository_->Process(data1, data2);
            }

            virtual ~TakeDifferenceService()
            {
                delete repository_;
            }
        };
    }
}
