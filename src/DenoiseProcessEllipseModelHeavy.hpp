#pragma once

#include "DenoiseProcessEllipseModel.h"


class DenoiseProcessEllipseModelHeavy: public DenoiseProcessEllipseModel
{
protected:

	Eigen::Matrix<double, 7, 7> MakeOperatorS(Eigen::Matrix<double, 7, 7>& mat)
	{
		return (mat + mat.transpose()) / 2;
	}

	Eigen::Matrix<double, 7, 7> MakeOperatorSeZeta(Eigen::Matrix<double, 7, 1>& zeta)
	{
		Eigen::Matrix<double, 7, 1> e;
		e << 1, 0, 1, 0, 0, 0, 0;//��2����^2�Ŏc��Ƃ���̂�

		Eigen::Matrix<double, 7, 7> A = zeta * e.transpose();

		return MakeOperatorS(A);
	}

	//�Ώ̍s��ɑ΂��郉���N6�̋t�s��
	Eigen::Matrix<double, 7, 7> CalcMi6(const Eigen::Matrix<double, 7, 7>& M)
	{
		// Eigen Solver for Self Adjoint Matrix
		Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 7, 7>> GES(M);

		Eigen::Matrix<double, 7, 7> Mi = Eigen::Matrix<double, 7, 7>().Zero();
		for (int i = 1; i < 7; i++)
		{
			//�Ώ̍s��Ȃ̂ł��̐������K�p�ł��� = �ŗL�x�N�g���ŋt�s��쐬
			Mi += GES.eigenvectors().col(i) * GES.eigenvectors().col(i).transpose() / GES.eigenvalues()(i);
		}
		return Mi;
	}

	bool HyperAccurateCorrection(Eigen::Matrix<double, 7, 1>& theta0, const std::vector<PointCustom>& window, const int maxLoop = 100, const double epsilonRatio = 50);

	virtual bool GetPixel(const std::vector<Point>& windowPoints, double& denoisedPixel, double& fittingErrorTotal, Eigen::Vector3d& normal);

public:
	DenoiseProcessEllipseModelHeavy(const int windowSize = 5, const bool replaceGrayScale = false) : DenoiseProcessEllipseModel(windowSize, replaceGrayScale)
	{
	
	}

	virtual ~DenoiseProcessEllipseModelHeavy()
	{
	
	}

};