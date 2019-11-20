#include "DenoiseProcessInterface.hpp"

bool DenoiseProcessInterface::DenoiseBuffer(const DenoiseArgs& args, std::vector<Point>& windowPoints, bool log)
{
	auto totalError = 0.0;
	auto maxError = DBL_MIN;
	auto minError = DBL_MAX;

	auto start = std::chrono::system_clock::now();

	int pixelError = 0;

	for (auto y = 0; y < args.height; ++y)
	{
		for (auto x = 0; x < args.width; ++x)
		{
			double denoisedPixel = 0;
			double error = 0;
			Eigen::Vector3d normal = Eigen::Vector3d(0, 0, -1);

			if (y < windowSize_ / 2 || x < windowSize_ / 2) goto noprocess;
			if (y > args.height - (windowSize_ / 2 + 1) || x > args.width - (windowSize_ / 2 + 1)) goto noprocess;

			//ウインドウの値を設定
			for (auto i = 0; i < windowPoints.size(); i++)
			{
				windowPoints[i].value = args.inImgBuffer[y + windowPoints[i].y][x + windowPoints[i].x];
			}

			//ピクセルごとの処理
			if (!GetPixel(windowPoints, denoisedPixel, error, normal))
			{
				pixelError++;
			}

		noprocess:
			//挿入
			args.outImgBuffer[y][x] = denoisedPixel;
			args.outNormalXBuffer[y][x] = normal.x();
			args.outNormalYBuffer[y][x] = normal.y();
			args.outNormalZBuffer[y][x] = normal.z();

			//誤差収集
			totalError += error;
			maxError = error > maxError ? error : maxError;
			minError = error < minError ? error : minError;

			//ログ出力
			if (log)
			{
				auto end = std::chrono::system_clock::now();
				auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				if (elapsed > 10 * 1000 || (y == args.height - 1 && x == args.width - 1))
				{
					std::cout << "Progress: " << y / (args.height - 1.0f) * 100 << "%, ";
					std::cout << "Error/pixel: " << totalError / (y * args.width) / (windowSize_ * windowSize_) << ", ";
					std::cout << "Error(Min, Max): " << minError << ", " << maxError << ", ";
					std::cout << "PixelErrors: " << pixelError << std::endl;
					start = end;
				}
			}
		}
	}

	return true;
}
