#pragma once

#include "ImageDataManager.hpp"

#include <chrono>

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Dense>

class DenoiseProcessInterface
{
protected:
	int windowSize_;
	bool replaceGrayScale_;

	struct Point
	{
		int x;
		int y;
		double value;
	};
	std::vector<Point> windowPoints;

	struct DenoiseArgs
	{
		double** const outImgBuffer;
		double** const inImgBuffer;
		int width;
		int height;
		double** const outNormalXBuffer;
		double** const outNormalYBuffer;
		double** const outNormalZBuffer;
	};

	std::vector<Point> CreateWindow(int m)
	{
		//範囲チェック
		if (m < 5 || m % 2 == 0) return std::vector<Point>();

		int n = m * m;
		std::vector<Point> v;

		for (auto y = -m / 2; y < m / 2 + 1; y++)
		{
			for (auto x = -m / 2; x < m / 2 + 1; x++)
			{
				v.push_back({ x,y,0 });
			}
		}
		return v;
	}

	//継承先のクラスはこの関数をオーバーライドする
	virtual bool GetPixel(const std::vector<Point>& windowPoints, double& denoisedPixel, double& fittingErrorTotal, Eigen::Vector3d& normal) = 0;

	bool DenoiseBuffer(const DenoiseArgs& args, std::vector<Point>& windowPoints, bool log = false);

public:
	DenoiseProcessInterface(const int windowSize = 5, bool replaceGrayScale = false): windowSize_(windowSize), replaceGrayScale_(replaceGrayScale)
	{
		//ウインドウ作成
		windowPoints = CreateWindow(windowSize_);
	}
	virtual ~DenoiseProcessInterface()
	{

	}

	virtual bool Process(ImageDataManager& image)
	{
		//R
		{
			std::cout << "Red" << std::endl;
			DenoiseArgs arg = {image.rInfo.imgDenoised , image.rInfo.imgOriginal, image.width, image.height, image.rInfo.imgNormalX, image.rInfo.imgNormalY, image.rInfo.imgNormalZ};
			DenoiseBuffer(arg, windowPoints, true);
		}

		//G
		{
			std::cout << "Green" << std::endl;
			DenoiseArgs arg = { image.gInfo.imgDenoised , image.gInfo.imgOriginal, image.width, image.height, image.gInfo.imgNormalX, image.gInfo.imgNormalY, image.gInfo.imgNormalZ };
			DenoiseBuffer(arg, windowPoints, true);
		}

		//B
		{
			std::cout << "Blue" << std::endl;
			DenoiseArgs arg = { image.bInfo.imgDenoised , image.bInfo.imgOriginal, image.width, image.height, image.bInfo.imgNormalX, image.bInfo.imgNormalY, image.bInfo.imgNormalZ };
			DenoiseBuffer(arg, windowPoints, true);
		}

		//Replace GrayScale to Denoised GrayScale
		if(replaceGrayScale_)
		{
			std::cout << "Replace GrayScale to Denoised GrayScale" << std::endl;
			//ノイズ除去済みのグレースケール作成
			image.ReplaceGrayBufferAsDenoisedGrayBuffer();
		}

		//GrayScale
		{
			std::cout << "GrayScale" << std::endl;
			DenoiseArgs arg = { image.grayScale.imgDenoised , image.grayScale.imgOriginal, image.width, image.height, image.grayScale.imgNormalX, image.grayScale.imgNormalY, image.grayScale.imgNormalZ };
			DenoiseBuffer(arg, windowPoints, true);
		}

		return true;
	}
};
