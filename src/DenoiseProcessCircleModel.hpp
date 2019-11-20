#pragma once

#include "DenoiseProcessInterface.h"

class DenoiseProcessCircleModel : public DenoiseProcessInterface
{
	Eigen::FullPivLU<Eigen::Matrix3d> luMatrix_;

	Eigen::FullPivLU<Eigen::Matrix3d> CreateLUMatrix(const std::vector<Point>& window)
	{
		auto A11 = 0.0;
		auto A12 = 0.0;
		auto A13 = 0.0;
		auto A21 = 0.0;
		auto A22 = 0.0;
		auto A23 = 0.0;
		auto A31 = 0.0;
		auto A32 = 0.0;
		auto A33 = (double)window.size();

		for (auto i = 0; i < window.size(); i++)
		{
			A11 += std::pow(window[i].x, 4);
			A12 += std::pow(window[i].x, 2) * std::pow(window[i].y, 2);
			A13 += std::pow(window[i].x, 2);
			A22 += std::pow(window[i].y, 4);
			A23 += std::pow(window[i].y, 2);
		}

		//対称行列
		A21 = A12;
		A31 = A13;
		A32 = A23;

		//Eigenで解く
		Eigen::Matrix3d matA;
		matA << A11, A12, A13, A21, A22, A23, A31, A32, A33;

		return Eigen::FullPivLU<Eigen::Matrix3d>(matA);
	}

	bool CalcACE(const std::vector<Point>& window, const Eigen::FullPivLU<Eigen::Matrix3d>& lu, double& a, double& c, double& e)
	{
		//Ax = bを解く
		auto b1 = 0.0;
		auto b2 = 0.0;
		auto b3 = 0.0;

		for (auto i = 0; i < window.size(); i++)
		{
			b1 += std::pow(window[i].x, 2) * window[i].value;
			b2 += std::pow(window[i].y, 2) * window[i].value;
			b3 += window[i].value;
		}

		Eigen::Vector3d vecb;
		vecb << b1, b2, b3;

		auto ace = lu.solve(vecb);

		a = ace.x();
		c = ace.y();
		e = ace.z();

		return true;
	}

	double CalcB(const std::vector<Point>& window)
	{
		auto sum1 = 0.0;
		auto sum2 = 0.0;
		for (auto i = 0; i < window.size(); i++)
		{
			sum1 += window[i].x * window[i].value;
			sum2 += window[i].x * window[i].x;
		}

		return sum1 / sum2;
	}

	double CalcD(const std::vector<Point>& window)
	{
		auto sum1 = 0.0;
		auto sum2 = 0.0;
		for (auto i = 0; i < window.size(); i++)
		{
			sum1 += window[i].y * window[i].value;
			sum2 += window[i].y * window[i].y;
		}

		return sum1 / sum2;
	}


protected:
	virtual bool GetPixel(const std::vector<Point>& windowPoints, double& denoisedPixel, double& fittingErrorTotal, Eigen::Vector3d& normal)
	{
		//Oを真値として、Sを実測値とする
		//O = a*x^2 + b*x + c*y^2 + d*y + e
		//誤差 = S - Oとしたときに誤差が0となる
		//a, b, c, d, eを求める
		//上記式より誤差が0となれば S(0,0) = eとなる

		double a, b, c, d, e;
		b = CalcB(windowPoints);
		d = CalcD(windowPoints);
		CalcACE(windowPoints, luMatrix_, a, c, e);

		//ノイズ除去済み値
		denoisedPixel = e;

		//観測値と推測値の差の合計
		fittingErrorTotal = 0;
		for (auto i = 0; i < windowPoints.size(); i++)
		{
			auto observedValue = (double)windowPoints[i].value;
			auto estimatedValue = a * windowPoints[i].x * windowPoints[i].x + b * windowPoints[i].x + c * windowPoints[i].y * windowPoints[i].y + d * windowPoints[i].y + e;
			fittingErrorTotal += std::abs(ImageDataManager::SubABWithPrecise(observedValue, estimatedValue));
		}

		//法線ベクトル
		auto dzdx = b;
		auto dzdy = d;
		normal = Eigen::Vector3d(-dzdx, -dzdy, 1).normalized();	

		return true;
	}

public:
	DenoiseProcessCircleModel(const int windowSize = 5, const bool replaceGrayScale = false): DenoiseProcessInterface(windowSize, replaceGrayScale)
	{
		//LU行列の作成
		luMatrix_ = CreateLUMatrix(windowPoints);
	}
	virtual ~DenoiseProcessCircleModel()
	{

	}

};
