#pragma once

#include "FloatingPointImageData.hpp"

#include <tuple>

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;

        class CircleDenoiseDataRepository : public IDenoiseImageDataRepository
        {
        protected:
            Eigen::FullPivLU<Eigen::Matrix3d> windowLUMatrix_;

            //WindowSize�����܂�Έ�ӂɌ��܂�s��
            Eigen::FullPivLU<Eigen::Matrix3d> CreateWindowLUMatrix(std::vector<ImageUtility::ImagePointBase>& windowPoints)
            {
                auto A11 = 0.0;
                auto A12 = 0.0;
                auto A13 = 0.0;
                auto A21 = 0.0;
                auto A22 = 0.0;
                auto A23 = 0.0;
                auto A31 = 0.0;
                auto A32 = 0.0;
                auto A33 = (double)windowPoints.size();

                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    A11 += std::pow(windowPoints[i].OffsetX, 4);
                    A12 += std::pow(windowPoints[i].OffsetX, 2) * std::pow(windowPoints[i].OffsetY, 2);
                    A13 += std::pow(windowPoints[i].OffsetX, 2);
                    A22 += std::pow(windowPoints[i].OffsetY, 4);
                    A23 += std::pow(windowPoints[i].OffsetY, 2);
                }

                //�Ώ̍s��
                A21 = A12;
                A31 = A13;
                A32 = A23;

                //Eigen�ŉ���
                Eigen::Matrix3d matA;
                matA << A11, A12, A13, A21, A22, A23, A31, A32, A33;

                return Eigen::FullPivLU<Eigen::Matrix3d>(matA);
            }

            //Param: A,B,C,D,E
            inline std::tuple<double, double, double> GetParamAandCandE(const std::vector<ImageUtility::ImagePointBase>& windowPoints) const
            {
                //Ax = b������
                auto b1 = 0.0;
                auto b2 = 0.0;
                auto b3 = 0.0;

                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    b1 += std::pow(windowPoints[i].OffsetX, 2) * windowPoints[i].Value;
                    b2 += std::pow(windowPoints[i].OffsetY, 2) * windowPoints[i].Value;
                    b3 += windowPoints[i].Value;
                }

                Eigen::Vector3d vecb;
                vecb << b1, b2, b3;

                auto ace = windowLUMatrix_.solve(vecb);

                return std::tuple<double, double, double>(ace.x(), ace.y(), ace.z());
            }

            inline std::tuple<double, double> GetParamBandD(const std::vector<ImageUtility::ImagePointBase>& windowPoints) const
            {
                auto B1 = 0.0;
                auto B2 = 0.0;
                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    //���W�����ɏd�݂Â���ω�
                    B1 += (double)windowPoints[i].OffsetX * windowPoints[i].Value;
                    B2 += (double)windowPoints[i].OffsetX * windowPoints[i].OffsetX;
                }
                auto D1 = 0.0;
                auto D2 = 0.0;
                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    D1 += (double)windowPoints[i].OffsetY * windowPoints[i].Value;
                    D2 += (double)windowPoints[i].OffsetY * windowPoints[i].OffsetY;
                }

                return std::tuple<double, double>(B1 / B2, D1 / D2);
            }

        public:
            explicit CircleDenoiseDataRepository()
            {
                auto windowPoints = ImageUtility::GetWindowPoints<ImageUtility::ImagePointBase>(WINDOW_SIZE);
                windowLUMatrix_ = CreateWindowLUMatrix(windowPoints);
            }
            virtual ~CircleDenoiseDataRepository() = default;

        protected:
            virtual inline bool DenoisePixel(const FloatingPointImageData* data, const int x, const int y, const int windowSize, double& denoisedPixel, Eigen::Vector3d& normal, double& fittingError) override
            {
                auto width = data->Width;
                auto height = data->Height;

                auto windowPoints = ImageUtility::GetWindowPoints<ImageUtility::ImagePointBase>(data, x, y, windowSize);

                //O��^�l�Ƃ��āAS�������l�Ƃ���
                //O = a*x^2 + b*x + c*y^2 + d*y + e
                //�덷 = S - O�Ƃ����Ƃ��Ɍ덷��0�ƂȂ�
                //a, b, c, d, e�����߂�
                //��L�����덷��0�ƂȂ�Β��S�_ S(0,0) = e�ƂȂ�

                auto ace = GetParamAandCandE(windowPoints);
                auto bd = GetParamBandD(windowPoints);

                auto a = std::get<0>(ace);
                auto b = std::get<0>(bd);
                auto c = std::get<1>(ace);
                auto d = std::get<1>(bd);
                auto e = std::get<2>(ace);

                //�m�C�Y�����ςݒl
                denoisedPixel = e;

                //�ϑ��l�Ɛ����l�̍��̍��v
                fittingError = 0;
                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    auto observedValue = (double)windowPoints[i].Value;
                    auto estimatedValue = a * windowPoints[i].OffsetX * windowPoints[i].OffsetX + b * windowPoints[i].OffsetX + c * windowPoints[i].OffsetY * windowPoints[i].OffsetY + d * windowPoints[i].OffsetY + e;
                    fittingError += std::abs(ImageUtility::DoubleSub(observedValue, estimatedValue));
                }

                //�@���x�N�g��
                auto dzdx = b;
                auto dzdy = d;
                normal = Eigen::Vector3d(-dzdx, -dzdy, 1).normalized();

                return true;
            }
        };
    }
}