#pragma once

#include "FloatingPointImageData.hpp"

#ifdef _MSC_VER//for MSVC
#define _USE_MATH_DEFINES
#include <corecrt_math_defines.h>
#endif
#include <ceres/ceres.h>

namespace ImageInformationAnalyzer
{
    namespace Infrastructure
    {
        using namespace Domain;

        class PhongModelLightDirectionDataRepository : public ILightEstimationDataRepository
        {
            struct CostFunctor
            {
                explicit CostFunctor(Eigen::Vector3d point, Eigen::Vector3d normal, double skin, double value) : point_(point), normal_(normal), skin_(skin), value_(value)
                {

                }
                virtual ~CostFunctor() = default;

                template <typename T> bool operator()(const T* const light, const T* const coef, T* residual) const
                {
                    //type Tへ変換
                    Eigen::Matrix<T, 3, 1> point(T(point_.x()), T(point_.y()), T(point_.z()));
                    Eigen::Matrix<T, 3, 1> normal(T(normal_.x()), T(normal_.y()), T(normal_.z()));


                    //極座標変換
                    Eigen::Matrix<T, 3, 1> lightPoint;
                    {
                        auto theta = light[0];//θ: -PI/2 to PI/2
                        auto phi = light[1];//φ: -PI to PI
                        lightPoint = Eigen::Matrix<T, 3, 1>(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)) * T(LightLength);
                    }

                    auto coefA = T(0.0);
                    auto coefB = T(0.0);
                    auto coefC = T(0.0);
                    {
                        auto theta = coef[0];//θ: 0 to PI/2
                        auto phi = coef[1];//φ: 0 to PI/2
                        coefA = sin(theta) * cos(phi);
                        coefB = sin(theta) * sin(phi);
                        coefC = cos(theta);
                    }

                    //光源
                    Eigen::Matrix<T, 3, 1> lightDir = lightPoint - point;
                    lightDir.normalize();

                    //視点ベクトル
                    Eigen::Matrix<T, 3, 1> eyePoint(T(0.0), T(0.0), T(EyeLength));
                    auto eyeDir = eyePoint - point;

                    //反射ベクトル
                    auto refDir = -eyeDir + T(2.0) * normal.dot(eyeDir) * normal;

                    //誤差
                    residual[0] = T(value_) - (coefA * normal.dot(lightDir) + coefB * pow(lightDir.dot(refDir), SpecularPower) + coefC * skin_);

                    return true;
                }

            private:
                const Eigen::Vector3d point_;
                const Eigen::Vector3d normal_;
                const double skin_;
                const double value_;
            };

            struct Point3D
            {
                Eigen::Vector3d Position;
                Eigen::Vector3d Normal;
                double SurfaceValue;
                double GrayscaleValue;
            };

        protected:
            std::vector<std::vector<double>> GetAverageImage(const int width, const int height, const FloatingPointImageData* denoisedR, const FloatingPointImageData* denoisedG, const FloatingPointImageData* denoisedB)
            {
                std::vector<std::vector<double>> imageBuffer;
                imageBuffer.resize(height);

                for(auto y = 0; y < height; ++y)
                {
                    imageBuffer[y].resize(width);
                    for(auto x = 0; x < width; ++x)
                    {
                        auto r = denoisedR->ImageBuffer[y][x];
                        auto g = denoisedG->ImageBuffer[y][x];
                        auto b = denoisedB->ImageBuffer[y][x];

                        //ITU-R Rec BT.601
                        imageBuffer[y][x] = 0.299 * r + 0.587 * g + 0.114 * b;
                    }
                }
                return imageBuffer;
            }

            std::vector<std::vector<Eigen::Vector3d>> GetAverageNormal(const int width, const int height, const FloatingPointImageData* denoisedR, const FloatingPointImageData* denoisedG, const FloatingPointImageData* denoisedB)
            {
                std::vector<std::vector<Eigen::Vector3d>> normalBuffer;
                normalBuffer.resize(height);

                for(auto y = 0; y < height; ++y)
                {
                    normalBuffer[y].resize(width);
                    for(auto x = 0; x < width; ++x)
                    {
                        auto r = denoisedR->NormalBuffer[y][x];
                        auto g = denoisedG->NormalBuffer[y][x];
                        auto b = denoisedB->NormalBuffer[y][x];

                        //average
                        normalBuffer[y][x] = (r + g + b).normalized();
                    }
                }
                return normalBuffer;
            }

            class Callback : public ceres::IterationCallback
            {
                std::atomic<double>* progress_;
            public:
                explicit Callback(std::atomic<double>* progress) : progress_(progress)
                {

                }
                virtual ~Callback() = default;
                virtual ceres::CallbackReturnType operator()(const  ceres::IterationSummary& summary) override
                {
                    if(progress_ != nullptr) *progress_ = summary.iteration / (double)MaxIteration;
                    return ceres::CallbackReturnType::SOLVER_CONTINUE;
                }
            };


        public:
            explicit PhongModelLightDirectionDataRepository() = default;
            virtual ~PhongModelLightDirectionDataRepository() = default;

            constexpr static int MaxIteration = 100;
            constexpr static double LightLength = 2.0;//画像からXm先
            constexpr static double EyeLength = 0.5;//画像からXm先
            constexpr static double SpecularPower = 3.0;

            virtual FloatingPointImageData* Process(const FloatingPointImageData* denoisedR, const FloatingPointImageData* denoisedG, const FloatingPointImageData* denoisedB, const FloatingPointImageData* differentialB_G, const double pixelPitch, std::atomic<double>* progress)  override
            {
                auto width = denoisedR->Width;
                auto height = denoisedR->Height;

                //average = grayscale
                auto averageImageBuffer = GetAverageImage(width, height, denoisedR, denoisedG, denoisedB);
                auto averageNormalBuffer = GetAverageNormal(width, height, denoisedR, denoisedG, denoisedB);

                //output data
                std::vector < std::vector <double>> imageBuffer;
                imageBuffer.resize(height);
                for(auto y = 0; y < height; ++y)
                {
                    imageBuffer[y].resize(width);
                }

                //最適化されるパラメータ
                Eigen::Vector2d resultLight(0, 0);
                Eigen::Vector2d resultCoef(0, 0);

                //情報取り出し
                std::vector<Point3D> data;
                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        Point3D d;
                        d.Position = Eigen::Vector3d((x - width / 2.0) * pixelPitch, (x - height / 2.0) * pixelPitch, 0);
                        d.Normal = Eigen::Vector3d(averageNormalBuffer[y][x].x(), averageNormalBuffer[y][x].y(), averageNormalBuffer[y][x].z());
                        d.GrayscaleValue = averageImageBuffer[y][x];
                        d.SurfaceValue = differentialB_G->ImageBuffer[y][x];
                        data.push_back(d);
                    }
                }

                //情報挿入
                ceres::Problem problem;
                for(auto d : data)
                {
                    auto f = new ceres::AutoDiffCostFunction<CostFunctor, 1, 2, 2>(new CostFunctor(d.Position, d.Normal, d.SurfaceValue, d.GrayscaleValue));
                    problem.AddResidualBlock(f, new ceres::CauchyLoss(0.2), resultLight.data(), resultCoef.data());
                }

                //Upper and lower
                problem.SetParameterLowerBound(resultLight.data(), 0, -M_PI / 2);
                problem.SetParameterUpperBound(resultLight.data(), 0, M_PI / 2);
                problem.SetParameterLowerBound(resultLight.data(), 1, -M_PI);
                problem.SetParameterUpperBound(resultLight.data(), 1, M_PI);

                //Whole region
                problem.SetParameterLowerBound(resultCoef.data(), 0, -M_PI);
                problem.SetParameterUpperBound(resultCoef.data(), 0, M_PI);
                problem.SetParameterLowerBound(resultCoef.data(), 1, -M_PI);
                problem.SetParameterUpperBound(resultCoef.data(), 1, M_PI);

                //Callback
                Callback callback(progress);

                //Solve
                ceres::Solver::Options options;
                //options.minimizer_progress_to_stdout = true;
                options.function_tolerance = 1e-6;//default
                options.max_num_iterations = MaxIteration;//Changed from 50
                options.update_state_every_iteration = true;
                options.callbacks.push_back(&callback);
                ceres::Solver::Summary summary;
                ceres::Solve(options, &problem, &summary);

                //光源
                Eigen::Vector3d lightPoint;
                {
                    auto theta = resultLight[0];
                    auto phi = resultLight[1];
                    lightPoint = Eigen::Vector3d(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)) * LightLength;
                    //std::cout << "LightPoint: \n" << lightPoint << "\n--------" << std::endl;
                }

                //係数
                Eigen::Vector3d coefs;
                {
                    auto theta = resultCoef[0];
                    auto phi = resultCoef[1];
                    coefs = Eigen::Vector3d(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
                    //std::cout << "Coefs: \n" << coefs << "\n--------" << std::endl;
                }

                //パラメータから復元
                for(auto y = 0; y < height; ++y)
                {
                    for(auto x = 0; x < width; ++x)
                    {
                        auto point = Eigen::Vector3d((x - width / 2.0) * pixelPitch, (x - height / 2.0) * pixelPitch, 0);
                        auto normal = Eigen::Vector3d(averageNormalBuffer[y][x].x(), averageNormalBuffer[y][x].y(), averageNormalBuffer[y][x].z());
                        auto grayscaleValue = averageImageBuffer[y][x];

                        //光源
                        Eigen::Vector3d lightDir = lightPoint - point;
                        lightDir.normalize();

                        //視点ベクトル
                        Eigen::Vector3d eyePoint(0, 0, EyeLength);
                        auto eyeDir = eyePoint - point;

                        //反射ベクトル
                        auto refDir = -eyeDir + (2.0) * normal.dot(eyeDir) * normal;

                        //拡散反射と鏡面反射
                        auto diffuse = normal.dot(lightDir);
                        auto specular = pow(lightDir.dot(refDir), SpecularPower);

                        //復元結果
                        auto surfaceValue = ImageUtility::DoubleSub(grayscaleValue, coefs.x() * diffuse - coefs.y() * specular) / coefs.z();
                        imageBuffer[y][x] = surfaceValue;
                    }
                }

                return new FloatingPointImageData(width, height, imageBuffer, averageNormalBuffer);
            }
        };
    }
}
