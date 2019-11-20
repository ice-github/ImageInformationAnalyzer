#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <ceres/ceres.h>

#include "ImageDataManager.h"

class LightDirectionSolver
{
	static const double LightLength;//�摜����Xm��
	static const double EyeLength;//�摜����Xm��

	struct CostFunctor
	{
		CostFunctor(Eigen::Vector3d point, Eigen::Vector3d normal, double skin, double value) : point_(point), normal_(normal), skin_(skin), value_(value)
		{
			//�R���X�g���N�^
		}

		template <typename T> bool operator()(const T* const light, const T* const coef, T* residual) const
		{
			//type T�֕ϊ�
			Eigen::Matrix<T, 3, 1> point(T(point_.x()), T(point_.y()), T(point_.z()));
			Eigen::Matrix<T, 3, 1> normal(T(normal_.x()), T(normal_.y()), T(normal_.z()));


			//�ɍ��W�ϊ�
			Eigen::Matrix<T, 3, 1> lightPoint;
			{
				auto theta = light[0];//��: -PI/2 to PI/2
				auto phi = light[1];//��: -PI to PI
				lightPoint = Eigen::Matrix<T, 3, 1>(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)) * T(LightDirectionSolver::LightLength);
			}

			auto coefA = T(0.0);
			auto coefB = T(0.0);
			auto coefC = T(0.0);
			{
				auto theta = coef[0];//��: 0 to PI/2
				auto phi = coef[1];//��: 0 to PI/2
				coefA = sin(theta) * cos(phi);
				coefB = sin(theta) * sin(phi);
				coefC = cos(theta);
			}

			//����
			Eigen::Matrix<T, 3, 1> lightDir = lightPoint - point;
			lightDir.normalize();

			//���_�x�N�g��
			Eigen::Matrix<T, 3, 1> eyePoint(T(0.0), T(0.0), T(LightDirectionSolver::EyeLength));
			auto eyeDir = eyePoint - point;

			//���˃x�N�g��
			auto refDir = -eyeDir + T(2.0) * normal.dot(eyeDir) * normal;

			//�덷
			residual[0] = T(value_) - (coefA * normal.dot(lightDir) + coefB * pow(lightDir.dot(refDir), 3) + coefC * skin_);

			return true;
		}

	private:
		const Eigen::Vector3d point_;
		const Eigen::Vector3d normal_;
		const double skin_;
		const double value_;

	};

	struct Data
	{
		Eigen::Vector3d Position;
		Eigen::Vector3d Normal;
		double skinValue;
		double grayValue;
	};

public:


	LightDirectionSolver()
	{

	}

	void Run(ImageDataManager& image, double pixelPitch = 0.001 * 0.01); //0.01mm
	
	virtual ~LightDirectionSolver()
	{

	}

};

