#pragma once

#include "FloatingPointImageData.hpp"

#include <tuple>

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;

        class EllipseDenoiseDataRepository : public IDenoiseImageDataRepository
        {
        protected:
            const int MAX_LOOP = 1000;
            const double ERROR_THRESHOLD = 0.0001;

            struct ImagePointEllipse: ImageUtility::ImagePointBase
            {
                Eigen::Matrix<double, 7, 1> ZetaVector;
                Eigen::Matrix<double, 7, 7> ZetaMatrix;
                Eigen::Matrix<double, 7, 7> Variance0Matrix;
                Eigen::Matrix<double, 7, 7> OperatorSMatrix;
            };

            //A*x^2 + B*2xy + C*y^2 + D*2f0x + E*2f0y + F * f0^2  + G * (-2f0z) = 0
            //�� = [A, B, C, D, E, F, G]
            //�� = [x^2, 2xy, y^2, 2f0x, 2f0y, f0^2, -2f0z]
            //(��, ��) = 0
            //(��, ��t�ă�) = 0

            inline Eigen::Matrix<double, 7, 1> GetZetaVector(const double x, const double y, const double z, const double f0)
            {
                Eigen::Matrix<double, 7, 1> zeta;
                zeta << x * x, 2 * x * y, y* y, 2 * f0 * x, 2 * f0 * y, f0* f0, -2 * f0 * z;
                return zeta;
            }

            inline Eigen::Matrix<double, 7, 7> GetZetaMatrix(const double x, const double y, const double z, const double f0)
            {
                auto zeta = GetZetaVector(x, y, z, f0);
                return zeta * zeta.transpose();
            }

            inline Eigen::Matrix<double, 7, 7> GetVariance0(const double x, const double y, const double f0)
            {
                Eigen::Matrix<double, 7, 7> mat;

                mat <<
                    x * x, x* y, 0, f0* x, 0, 0, 0,
                    x* y, x* x + y * y, x* y, f0* y, f0* x, 0, 0,
                    0, x* y, y* y, 0, f0* y, 0, 0,
                    f0* x, f0* y, 0, f0* f0, 0, 0, 0,
                    0, f0* x, f0* y, 0, f0* f0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, f0* f0;

                return 4 * mat;
            }

            //���肱�ݖ@
            virtual bool Renormalize(Eigen::Matrix<double, 7, 1>& theta0, const std::vector<ImagePointEllipse>& windowPoints)
            {
                std::vector<double> Ws;
                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    Ws.push_back(1.0);//1.0�ŏ�����
                }

                for(auto loop = 0; loop < MAX_LOOP; ++loop)
                {
                    //M�̎Z�o
                    Eigen::Matrix<double, 7, 7> M = Eigen::Matrix<double, 7, 7>().Zero();
                    for(auto i = 0; i < windowPoints.size(); i++)
                    {
                        M += Ws[i] * windowPoints[i].ZetaMatrix;
                    }
                    M = M / windowPoints.size();


                    //N�̎Z�o
                    Eigen::Matrix<double, 7, 7> N = Eigen::Matrix<double, 7, 7>().Zero();
                    for(auto i = 0; i < windowPoints.size(); ++i)
                    {
                        N += Ws[i] * windowPoints[i].Variance0Matrix;
                    }
                    N = N / windowPoints.size();


                    //N�� = 1/�� * M�� �Ƃ���1/�ɂ��ő�ɂȂ�ŗL�l��T��
                    //�Ȃ̂�N��M���t�ɂȂ�

                    //��ʌŗL�l������
                    Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::Matrix<double, 7, 7>> GES(N, M);
                    //Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 7, 7>> SES(M);

                    //�ő�ŗL�l
                    Eigen::Matrix<double, 7, 1> theta = GES.eigenvectors().col(6).normalized();
                    //Eigen::Matrix<double, 7, 1> theta = SES.eigenvectors().col(0).normalized();

                    //�I���`�F�b�N
                    auto distance = GetVectorDistance(theta0, theta);
                    if(distance <= ERROR_THRESHOLD)
                    {
                        theta0 = theta;
                        //std::cout << "loop: " << loop << std::endl;
                        break;
                    }

                    //�X�V
                    for(auto i = 0; i < windowPoints.size(); ++i)
                    {
                        Ws[i] = 1 / (theta.transpose() * windowPoints[i].Variance0Matrix * theta);
                    }
                    theta0 = theta;

                    if(loop == MAX_LOOP - 1)
                    {
                        //std::cout << "detected max loop!: "s << distance << std::endl;
                        return false;
                    }
                }

                return true;
            }


            static inline double GetVectorDistance(const Eigen::Matrix<double, 7, 1>& theta1, const Eigen::Matrix<double, 7, 1>& theta2)
            {
                Eigen::Matrix<double, 7, 1> diff = theta1 - theta2;

                auto result = 0.0;
                for(auto i = 0; i < 7; i++)
                {
                    result += std::abs(diff(i));
                }
                return result;
            }

        public:
            explicit EllipseDenoiseDataRepository()
            {
            }
            virtual ~EllipseDenoiseDataRepository() = default;

        protected:       
            virtual inline bool DenoisePixel(const FloatingPointImageData* data, const int x, const int y, const int windowSize, double& denoisedPixel, Eigen::Vector3d& normal, double& fittingError) override
            {
                auto width = data->Width;
                auto height = data->Height;

                auto windowPoints = ImageUtility::GetWindowPoints<ImagePointEllipse>(data, x, y, windowSize);

                auto f0 = windowSize;

                for(auto i = 0; i < windowPoints.size(); ++i)
                {
                    windowPoints[i].ZetaVector = GetZetaVector(windowPoints[i].OffsetX, windowPoints[i].OffsetY, windowPoints[i].Value, f0);
                    windowPoints[i].ZetaMatrix = GetZetaMatrix(windowPoints[i].OffsetX, windowPoints[i].OffsetY, windowPoints[i].Value, f0);
                    windowPoints[i].Variance0Matrix = GetVariance0(windowPoints[i].OffsetX, windowPoints[i].OffsetY, f0);
                    windowPoints[i].OperatorSMatrix = Eigen::Matrix<double, 7, 7>().Zero();
                }

                //�œK��
                Eigen::Matrix<double, 7, 1> theta = Eigen::Matrix<double, 7, 1>().Zero();
                if(!Renormalize(theta, windowPoints))
                {
                    //�v�Z���������Ȃ������ꍇ
                    denoisedPixel = data->ImageBuffer[y][x];
                    normal = Eigen::Vector3d(0, 0, 1);
                    fittingError = 0;
                    return false;
                }

                //z'(x,y) = (A*x^2 + B*2xy + C*y^2 + D*2f0x + E*2f0y + F * f0^2) / (G * 2f0)
                //z'(0,0) = F * f0^2 / (G * 2f0) = F * f0 / (2*G)

                auto A = theta(0);
                auto B = theta(1);
                auto C = theta(2);
                auto D = theta(3);
                auto E = theta(4);
                auto F = theta(5);
                auto G = theta(6);

                //�m�C�Y�����ςݒl
                denoisedPixel = F * f0 / (2 * G);

                //�ϑ��l�Ɛ����l�̍��̍��v
                fittingError = 0;
                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    auto observedValue = windowPoints[i].Value;
                    auto estimatedValue =
                        (A * windowPoints[i].OffsetX * windowPoints[i].OffsetX
                            + B * windowPoints[i].OffsetX * windowPoints[i].OffsetY
                            + C * windowPoints[i].OffsetY * windowPoints[i].OffsetY
                            + D * 2 * f0 * windowPoints[i].OffsetX
                            + E * 2 * f0 * windowPoints[i].OffsetY
                            + F * f0 * f0) / (G * 2 * f0);
                    fittingError += std::abs(ImageUtility::DoubleSub(observedValue, estimatedValue));
                }

                //dz'/dx = (2Ax + 2By + 2Df0) / (G * 2f0)
                //dz'/dx(0,0) = D / G

                //dz'/dy = (2Bx + 2Cy + 2Ef0) / (G * 2f0)
                //dz'/dy(0,0) = E / G

                //�@���x�N�g��
                auto dzdx = D / G;
                auto dzdy = E / G;
                normal = Eigen::Vector3d(-dzdx, -dzdy, 1).normalized();

                return true;
            }
        };
    }
}