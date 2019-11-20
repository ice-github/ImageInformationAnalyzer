#include "LightDirectionSolver.hpp"

#ifdef _DEBUG
#pragma comment(lib, "ceres-debug.lib")
#else
#pragma comment(lib, "ceres.lib")
#endif

const double LightDirectionSolver::LightLength = 2.0;//画像からXm先
const double LightDirectionSolver::EyeLength = 0.5;//画像からXm先

void LightDirectionSolver::Run(ImageDataManager& image, double pixelPitch)
{
	//最適化されるパラメータ
	Eigen::Vector2d resultLight(0, 0);
	Eigen::Vector2d resultCoef(0, 0);

	//情報取り出し
	std::vector<Data> datas;
	for (auto y = 0; y < image.height; ++y)
	{
		for (auto x = 0; x < image.width; ++x)
		{
			Data d;
			d.Position = Eigen::Vector3d((x - image.width / 2) * pixelPitch, (x - image.height / 2) * pixelPitch, 0);
			d.Normal = Eigen::Vector3d(image.grayScale.imgNormalX[y][x], image.grayScale.imgNormalY[y][x], image.grayScale.imgNormalZ[y][x]);
			d.grayValue = image.grayScale.imgDenoised[y][x];
			d.skinValue = image.diffImgBG[y][x];//image.diffImgBR[y][x]
			datas.push_back(d);
		}
	}

	//情報挿入
	ceres::Problem problem;
	for (auto d : datas)
	{
		auto f = new ceres::AutoDiffCostFunction<CostFunctor, 1, 2, 2>(new CostFunctor(d.Position, d.Normal, d.skinValue, d.grayValue));
		problem.AddResidualBlock(f, new ceres::CauchyLoss(0.2), resultLight.data(), resultCoef.data());
	}

	//上限と下限の設定
	problem.SetParameterLowerBound(resultLight.data(), 0, -M_PI / 2);
	problem.SetParameterUpperBound(resultLight.data(), 0, M_PI / 2);
	problem.SetParameterLowerBound(resultLight.data(), 1, -M_PI);
	problem.SetParameterUpperBound(resultLight.data(), 1, M_PI);

	//全領域
	problem.SetParameterLowerBound(resultCoef.data(), 0, -M_PI);
	problem.SetParameterUpperBound(resultCoef.data(), 0, M_PI);
	problem.SetParameterLowerBound(resultCoef.data(), 1, -M_PI);
	problem.SetParameterUpperBound(resultCoef.data(), 1, M_PI);


	// Run the solver!
	ceres::Solver::Options options;
	//options.minimizer_progress_to_stdout = true;
	options.function_tolerance = 1e-9;//1e-6から変更
	ceres::Solver::Summary summary;
	Solve(options, &problem, &summary);

	//光源
	Eigen::Vector3d lightPoint;
	{
		auto theta = resultLight[0];
		auto phi = resultLight[1];
		lightPoint = Eigen::Vector3d(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)) * 1.0;
		std::cout << "LightPoint: \n" << lightPoint << "\n--------" << std::endl;
	}

	//係数
	Eigen::Vector3d coefs;
	{
		auto theta = resultCoef[0];
		auto phi = resultCoef[1];
		coefs = Eigen::Vector3d(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
		std::cout << "Coefs: \n" << coefs << "\n--------" << std::endl;
	}

	//パラメータから復元
	for (auto y = 0; y < image.height; ++y)
	{
		for (auto x = 0; x < image.width; ++x)
		{
			auto point = Eigen::Vector3d((x - image.width / 2) * pixelPitch, (x - image.height / 2) * pixelPitch, 0);
			auto normal = Eigen::Vector3d(image.grayScale.imgNormalX[y][x], image.grayScale.imgNormalY[y][x], image.grayScale.imgNormalZ[y][x]);
			auto grayValue = image.grayScale.imgDenoised[y][x];

			//光源
			Eigen::Vector3d lightDir = lightPoint - point;
			lightDir.normalize();

			//視点ベクトル
			Eigen::Vector3d eyePoint(0, 0, 1);
			auto eyeDir = eyePoint - point;

			//反射ベクトル
			auto refDir = -eyeDir + (2.0) * normal.dot(eyeDir) * normal;

			//拡散反射と鏡面反射
			auto diffuse = normal.dot(lightDir);
			auto specular = pow(lightDir.dot(refDir), 3);

			//復元結果
			auto skinValue = (grayValue - coefs.x() * diffuse - coefs.y() * specular) / coefs.z();
			image.diffImgXX[y][x] = skinValue;
		}
	}
}
