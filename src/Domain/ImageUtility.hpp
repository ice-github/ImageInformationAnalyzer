#pragma once

#include "FloatingPointImageData.hpp"

namespace ImageInformationAnalyzer
{
	namespace Misc
	{
        using namespace Domain;

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
