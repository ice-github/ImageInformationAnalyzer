#pragma once

#include "DenoiseProcessInterface.hpp"

class DenoiseProcessEllipseModel : public DenoiseProcessInterface
{
protected:

	double f0_;

	struct PointCustom
	{
		int x;
		int y;
		double value;
		Eigen::Matrix<double, 7, 1> zetaVector;
		Eigen::Matrix<double, 7, 7> zetaMatrix;
		Eigen::Matrix<double, 7, 7> variance0Matrix;
		Eigen::Matrix<double, 7, 7> operatorSMatrix;
	};

	std::vector<PointCustom> customWindow_;

	//A*x^2 + B*2xy + C*y^2 + D*2f0x + E*2f0y + F * f0^2  + G * (-2f0z) = 0
	//θ = [A, B, C, D, E, F, G]
	//ζ = [x^2, 2xy, y^2, 2f0x, 2f0y, f0^2, -2f0z]
	//(θ, ζ) = 0
	//(θ, ζtζθ) = 0

	Eigen::Matrix<double, 7, 1> MakeZetaVector(const double x, const double y, const double z, const double f0)
	{
		Eigen::Matrix<double, 7, 1> zeta;
		zeta << x*x, 2 * x*y, y*y, 2 * f0*x, 2 * f0*y, f0*f0, -2 * f0*z;
		return zeta;
	}


	Eigen::Matrix<double, 7, 7> MakeZetaMatrix(const double x, const double y, const double z, const double f0)
	{
		auto zeta = MakeZetaVector(x, y, z, f0);
		return zeta * zeta.transpose();
	}

	void UpdateZetaVectorZ(Eigen::Matrix<double, 7, 1>& vec, const double x, const double y, const double z, const double f0)
	{
		vec(6) = -2 * f0*z;
	}

	void UpdateZetaMatrixZ(Eigen::Matrix<double, 7, 7>& mat, const double x, const double y, const double z, const double f0)
	{
		double tmp = -2 * f0*z;

		mat(0, 6) = x*x* tmp;
		mat(1, 6) = 2 * x*y* tmp;
		mat(2, 6) = y*y * tmp;
		mat(3, 6) = 2 * f0*x *tmp;
		mat(4, 6) = 2 * f0*y * tmp;
		mat(5, 6) = f0*f0 * tmp;

		//対称性を利用
		mat(6, 0) = mat(0, 6);
		mat(6, 1) = mat(1, 6);
		mat(6, 2) = mat(2, 6);
		mat(6, 3) = mat(3, 6);
		mat(6, 4) = mat(4, 6);
		mat(6, 5) = mat(5, 6);

		mat(6, 6) = tmp * tmp;
	}


	Eigen::Matrix<double, 7, 7> MakeVariance0(const double x, const double y, const double f0)
	{
		Eigen::Matrix<double, 7, 7> mat;

		mat <<
			x*x, x*y, 0, f0*x, 0, 0, 0,
			x*y, x*x + y*y, x*y, f0*y, f0*x, 0, 0,
			0, x*y, y*y, 0, f0*y, 0, 0,
			f0*x, f0*y, 0, f0*f0, 0, 0, 0,
			0, f0*x, f0*y, 0, f0*f0, 0, 0,
			0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, f0* f0;


		return 4 * mat;
	}

	std::vector<PointCustom> CreateCustomWindow(const int m, const double f0)
	{
		//範囲チェック
		if (m < 5 || m % 2 == 0) return std::vector<PointCustom>();

		int n = m * m;
		std::vector<PointCustom> v;

		for (auto y = -m / 2; y < m / 2 + 1; y++)
		{
			for (auto x = -m / 2; x < m / 2 + 1; x++)
			{
				v.push_back({ x, y, 0, MakeZetaVector(x, y, 0, f0), MakeZetaMatrix(x, y, 0, f0),  MakeVariance0(x, y, f0), Eigen::Matrix<double, 7, 7>().Zero() });
			}
		}
		return v;
	}

	double CalcVectorDistance(const Eigen::Matrix<double, 7, 1>& theta1, const Eigen::Matrix<double, 7, 1>& theta2)
	{
		Eigen::Matrix<double, 7, 1> diff = theta1 - theta2;

		auto result = 0.0;
		for (auto i = 0; i < 7; i++)
		{
			result += std::abs(diff(i));
		}
		return result;
	}

	bool Correction(Eigen::Matrix<double, 7, 1>& theta0, const std::vector<PointCustom>& window, const int maxLoop = 100, const double epsilonRatio = 50);


	virtual bool GetPixel(const std::vector<Point>& windowPoints, double& denoisedPixel, double& fittingErrorTotal, Eigen::Vector3d& normal);

public:
	DenoiseProcessEllipseModel(const int windowSize = 5, const bool replaceGrayScale = false) : DenoiseProcessInterface(windowSize, replaceGrayScale)
	{
		f0_ = windowSize;
		customWindow_ = CreateCustomWindow(windowSize, f0_);
	}

	virtual ~DenoiseProcessEllipseModel()
	{

	}

};
