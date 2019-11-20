#pragma once

#include "ImageDataManager.h"
#include "DenoiseProcessInterface.h"

class RGBSpectrumDifferentialProcess: public DenoiseProcessInterface
{
	bool CalcOffsetAB(const std::vector<Point>& window1, const std::vector<Point>& window2, double& a, double& b)
	{
		const auto windowBufferSize = window1.size();

		//Ax=c
		auto A11 = 0.0;
		auto A12 = 0.0;
		auto A21 = 0.0;
		auto A22 = (double)windowBufferSize;

		auto c1 = 0.0;
		auto c2 = 0.0;

		for (auto i = 0; i < windowBufferSize; i++)
		{
			A11 += std::pow(window2[i].value, 2);
			A12 += window2[i].value;

			c1 += window2[i].value * window1[i].value;
			c2 += window1[i].value;
		}
		//対称性より
		A21 = A12;

		//Eigenで解く
		Eigen::Matrix2d A;
		A << A11, A12, A21, A22;
		Eigen::Vector2d c;
		c << c1, c2;

		Eigen::FullPivLU<Eigen::Matrix2d> lu(A);
		auto ab = lu.solve(c);

		a = ab.x();
		b = ab.y();

		return true;
	}

	bool DiffBuffer(double** const inBuffer1, double** const inBuffer2, double** const outBuffer, const int width, const int height)
	{
		//inBuffer1 - inBuffer2をする
		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				outBuffer[y][x] = 0.0;

				if (y < windowSize_ / 2 || x < windowSize_ / 2) continue;
				if (y > height - (windowSize_ / 2 + 1) || x > width - (windowSize_ / 2 + 1)) continue;

				//ウインドウ作成
				auto w1 = CreateWindow(windowSize_);
				auto w2 = CreateWindow(windowSize_);

				//値を入れる
				for (auto i = 0; i < w1.size(); i++)
				{
					w1[i].value = inBuffer1[y + w1[i].y][x + w1[i].x];
					w2[i].value = inBuffer2[y + w2[i].y][x + w2[i].x];
				}

				//{S1 - (a*S2 + b)}^2 を最小化する
				auto a = 0.0;
				auto b = 0.0;
				CalcOffsetAB(w1, w2, a, b);

				//差分を出力
				outBuffer[y][x] = ImageDataManager::SubABWithPrecise(inBuffer1[y][x], a * inBuffer2[y][x] + b);
			}
		}

		return true;
	}

	virtual bool GetPixel(const std::vector<Point>& windowPoints, double& denoisedPixel, double& fittingErrorTotal, Eigen::Vector3d& normal)
	{
		return true;
	}

public:
	RGBSpectrumDifferentialProcess(const int windowSize = 5): DenoiseProcessInterface(windowSize, false)
	{
	
	}

	virtual ~RGBSpectrumDifferentialProcess()
	{
	
	}

	bool Process(ImageDataManager& image)
	{
		DiffBuffer(image.bInfo.imgDenoised, image.gInfo.imgDenoised, image.diffImgBG, image.width, image.height); //B-G
		DiffBuffer(image.gInfo.imgDenoised, image.rInfo.imgDenoised, image.diffImgGR, image.width, image.height); //G-R
		DiffBuffer(image.bInfo.imgDenoised, image.rInfo.imgDenoised, image.diffImgBR, image.width, image.height); //B-R

		return true;
	}
};
