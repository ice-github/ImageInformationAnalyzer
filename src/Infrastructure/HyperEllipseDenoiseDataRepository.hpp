#pragma once

#include "FloatingPointImageData.hpp"
#include "EllipseDenoiseDataRepository.hpp"

#include <tuple>

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;

        class HyperEllipseDenoiseDataRepository : public EllipseDenoiseDataRepository
        {
        protected:
            inline Eigen::Matrix<double, 7, 7> GetOperatorS(Eigen::Matrix<double, 7, 7>& mat)
            {
                return (mat + mat.transpose()) / 2;
            }

            inline Eigen::Matrix<double, 7, 7> GetOperatorSeZeta(Eigen::Matrix<double, 7, 1>& zeta)
            {
                Eigen::Matrix<double, 7, 1> e;
                e << 1, 0, 1, 0, 0, 0, 0;//��2����^2�Ŏc��Ƃ���̂�

                Eigen::Matrix<double, 7, 7> A = zeta * e.transpose();

                return GetOperatorS(A);
            }

            //�Ώ̍s��ɑ΂��郉���N6�̋t�s��
            inline Eigen::Matrix<double, 7, 7> CalcMi6(const Eigen::Matrix<double, 7, 7>& M)
            {
                // Eigen Solver for Self Adjoint Matrix
                Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 7, 7>> GES(M);

                Eigen::Matrix<double, 7, 7> Mi = Eigen::Matrix<double, 7, 7>().Zero();
                for(int i = 1; i < 7; i++)
                {
                    //�Ώ̍s��Ȃ̂ł��̐������K�p�ł��� = �ŗL�x�N�g���ŋt�s��쐬
                    Mi += GES.eigenvectors().col(i) * GES.eigenvectors().col(i).transpose() / GES.eigenvalues()(i);
                }
                return Mi;
            }

            virtual bool Renormalize(Eigen::Matrix<double, 7, 1>& theta0, const std::vector<ImagePointEllipse>& windowPoints) override
            { 
                std::vector<double> Ws;
                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    Ws.push_back(1.0);//1.0�ŏ�����
                }

                std::vector<Eigen::Matrix<double, 7, 7>> coeffNs;

                for(auto i = 0; i < windowPoints.size(); i++)
                {
                    auto coeffN = windowPoints[i].Variance0Matrix + 2 * windowPoints[i].OperatorSMatrix;
                    coeffNs.push_back(coeffN);
                }

                for(auto loop = 0; loop < MAX_LOOP; loop++)
                {
                    //M�̎Z�o
                    Eigen::Matrix<double, 7, 7> M = Eigen::Matrix<double, 7, 7>().Zero();
                    for(auto i = 0; i < windowPoints.size(); i++)
                    {
                        M += Ws[i] * windowPoints[i].ZetaMatrix;
                    }
                    M = M / windowPoints.size();

                    //Mi�̎Z�o
                    auto Mi = CalcMi6(M);

                    //N�̎Z�o
                    Eigen::Matrix<double, 7, 7> N1 = Eigen::Matrix<double, 7, 7>().Zero();
                    Eigen::Matrix<double, 7, 7> N2 = Eigen::Matrix<double, 7, 7>().Zero();
                    for(auto i = 0; i < windowPoints.size(); i++)
                    {
                        N1 += Ws[i] * coeffNs[i];

                        Eigen::Matrix<double, 7, 7> tmpMat = windowPoints[i].Variance0Matrix * Mi * windowPoints[i].ZetaMatrix;
                        N2 += Ws[i] * Ws[i] * (windowPoints[i].ZetaVector.transpose() * Mi * windowPoints[i].ZetaVector * windowPoints[i].Variance0Matrix + 2 * GetOperatorS(tmpMat));
                    }
                    N1 = N1 / windowPoints.size();
                    N2 = N2 / (windowPoints.size() * windowPoints.size());

                    Eigen::Matrix<double, 7, 7> N = N1 - N2;

                    //N�� = 1/�� * M�� �Ƃ���1/�ɂ��ő�ɂȂ�ŗL�l��T��
                    //�Ȃ̂�N��M���t�ɂȂ�

                    //��ʌŗL�l������
                    Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::Matrix<double, 7, 7>> GES(N, M);

                    //�ő�ŗL�l
                    Eigen::Matrix<double, 7, 1> theta = GES.eigenvectors().col(6).normalized();

                    //�I���`�F�b�N
                    auto distance = GetVectorDistance(theta0, theta);
                    if(distance <= ERROR_THRESHOLD)
                    {
                        theta0 = theta;
                        //std::cout << "loop: " << loop << std::endl;
                        break;
                    }

                    //�X�V
                    for(auto i = 0; i < windowPoints.size(); i++)
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

        public:
            explicit HyperEllipseDenoiseDataRepository()
            {
            }
            virtual ~HyperEllipseDenoiseDataRepository() = default;

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
                    windowPoints[i].OperatorSMatrix = GetOperatorSeZeta(windowPoints[i].ZetaVector);
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
                    auto observedValue = (double)windowPoints[i].Value;
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