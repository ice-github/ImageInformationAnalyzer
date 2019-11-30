
#include "DenoiseProcessEllipseModel.hpp"

bool DenoiseProcessEllipseModel::GetPixel(const std::vector<Point>& windowPoints, double& denoisedPixel, double& fittingErrorTotal, Eigen::Vector3d& normal)
{
	//値を入れる
	for (auto i = 0; i < windowPoints.size(); i++)
	{
		customWindow_[i].value = windowPoints[i].value;
		UpdateZetaVectorZ(customWindow_[i].zetaVector, customWindow_[i].x, customWindow_[i].y, customWindow_[i].value, f0_);
		UpdateZetaMatrixZ(customWindow_[i].zetaMatrix, customWindow_[i].x, customWindow_[i].y, customWindow_[i].value, f0_);
	}

	//くりこみ法
	Eigen::Matrix<double, 7, 1> theta = Eigen::Matrix<double, 7, 1>().Zero();
	if (!Correction(theta, customWindow_))
	{
		//計算が収束しなかった場合
		denoisedPixel = windowPoints[windowPoints.size() / 2].value;
		fittingErrorTotal = 0;
		normal = Eigen::Vector3d(0, 0, 1);
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

	//ノイズ除去済み値
	denoisedPixel = F * f0_ / (2 * G);
	
	//観測値と推測値の差の合計
	fittingErrorTotal = 0;
	for (auto i = 0; i < windowPoints.size(); i++)
	{
		auto observedValue = (double)windowPoints[i].value;
		auto estimatedValue =
			(A * windowPoints[i].x * windowPoints[i].x
				+ B * windowPoints[i].x * windowPoints[i].y
				+ C * windowPoints[i].y * windowPoints[i].y
				+ D * 2 * f0_ * windowPoints[i].x
				+ E * 2 * f0_ * windowPoints[i].y
				+ F * f0_ * f0_) / (G * 2 * f0_);
		fittingErrorTotal += std::abs(ImageDataManager::SubABWithPrecise(observedValue, estimatedValue));
	}

	//dz'/dx = (2Ax + 2By + 2Df0) / (G * 2f0)
	//dz'/dx(0,0) = D / G

	//dz'/dy = (2Bx + 2Cy + 2Ef0) / (G * 2f0)
	//dz'/dy(0,0) = E / G

	//法線ベクトル
	auto dzdx = D / G;
	auto dzdy = E / G;
	normal = Eigen::Vector3d(-dzdx, -dzdy, 1).normalized();

	return true;
}

bool DenoiseProcessEllipseModel::Correction(Eigen::Matrix<double, 7, 1>& theta0, const std::vector<PointCustom>& window, const int maxLoop, const double epsilonRatio)
{
	std::vector<double> Ws;
	for (auto i = 0; i < window.size(); i++)
	{
		Ws.push_back(1.0);//1.0で初期化
	}

	for (auto loop = 0; loop < maxLoop; loop++)
	{
		//Mの算出
		Eigen::Matrix<double, 7, 7> M = Eigen::Matrix<double, 7, 7>().Zero();
		for (auto i = 0; i < window.size(); i++)
		{
			M += Ws[i] * window[i].zetaMatrix;
		}
		M = M / window.size();


		//Nの算出
		Eigen::Matrix<double, 7, 7> N = Eigen::Matrix<double, 7, 7>().Zero();
		for (auto i = 0; i < window.size(); i++)
		{
			N += Ws[i] * window[i].variance0Matrix;
		}
		N = N / window.size();


		//Nθ = 1/λ * Mθ として1/λが最大になる固有値を探す
		//なのでNとMが逆になる

		//一般固有値を解く
		Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::Matrix<double, 7, 7>> GES(N, M);
		//Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 7, 7>> SES(M);

		//最大固有値
		Eigen::Matrix<double, 7, 1> theta = GES.eigenvectors().col(6).normalized();
		//Eigen::Matrix<double, 7, 1> theta = SES.eigenvectors().col(0).normalized();

		//終了チェック
		auto distance = CalcVectorDistance(theta0, theta);
		if (distance <= std::numeric_limits<double>::epsilon() * epsilonRatio)
		{
			theta0 = theta;
			//std::cout << "loop: " << loop << std::endl;
			break;
		}

		//更新
		for (auto i = 0; i < window.size(); i++)
		{
			Ws[i] = 1 / (theta.transpose() * window[i].variance0Matrix * theta);
		}
		theta0 = theta;

		if (loop == maxLoop - 1)
		{
			//std::cout << "detected max loop!: " << maxLoop << std::endl;
			return false;
		}
	}

	return true;
}
