#pragma once

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

class DenoiseProcessInterface;
class RGBSpectrumDifferentialProcess;
class LightDirectionSolver;

class ImageDataManager
{
	//これらインターフェスはImageDataManagerの全メンバを理解した上で使っているのでフレンド化
	friend DenoiseProcessInterface;
	friend RGBSpectrumDifferentialProcess;
	friend LightDirectionSolver;

	//1チャネルのバッファを作成
	template<typename Type>
	Type** Create1chBuffer(const int width, const int height)
	{
		auto img = new Type*[height];
		for (auto y = 0; y < height; ++y)
		{
			img[y] = new Type[width];
		}
		return img;
	}

	//1チャネルのバッファを開放
	template<typename Type>
	void Delete1chBuffer(Type** buffer, const int width, const int height)
	{
		for (auto y = 0; y < height; ++y)
		{
			delete[] buffer[y];
		}
		delete[] buffer;
		buffer = nullptr;
	}

	//特定チャネルをコピー
	template<typename Type>
	bool CopyImgTo1chBuffer(const cv::Mat &img, Type** const buffer, int channelIndex)
	{
		//チャネルが範囲内か確認
		if (channelIndex >= img.channels()) return false;

		for (auto y = 0; y < img.rows; ++y)
		{
			for (auto x = 0; x < img.cols; ++x)
			{
				for (auto c = 0; c < img.channels(); ++c)
				{
					auto pix = img.data[y * img.step + x * img.elemSize() + c];

					if (c == channelIndex)
					{
						buffer[y][x] = (Type)pix;
					}
				}
			}
		}
		return true;
	}

	template<typename Type>
	bool CopyImgToRGBBuffer(Type** const buffer, const cv::Mat &img, int channelIndex)
	{
		//チャネルが範囲内か確認
		if (channelIndex >= img.channels()) return false;

		for (auto y = 0; y < img.rows; ++y)
		{
			for (auto x = 0; x < img.cols; ++x)
			{
				for (auto c = 0; c < img.channels(); ++c)
				{
					if (c == channelIndex)
					{
						img.data[y * img.step + x * img.elemSize() + c] = (uchar)(buffer[y][x] + 0.5);
					}
				}
			}
		}
		return true;
	}

	template<typename Type>
	bool ConvertRGBToGrayScale(Type** const inBufferR, Type** const inBufferG, Type** const inBufferB, Type** const outBuffer, const int width, const int height)
	{
		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				auto r = (double)inBufferR[y][x];
				auto g = (double)inBufferG[y][x];
				auto b = (double)inBufferB[y][x];

				//Y = 0.299 × R + 0.587 × G + 0.114 × B
				outBuffer[y][x] = (Type)(0.299 * r + 0.587 * g + 0.114 * b);
			}
		}
		return true;
	}

	template<typename Type>
	cv::Mat ConvertTo1chMat(Type** const buffer, const int width, const int height)
	{
		cv::Mat output(height, width, CV_64FC1);

		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				auto p = &output.data[y * output.step + x * output.elemSize()];

				*((double*)p) = (double)(buffer[y][x]);
			}
		}
		return output;
	}


	//固定の幅でスケール
	template<typename Type>
	void ChangeColorScaleFixed(Type** inoutBuffer, const int width, const int height, const Type inMaxValue = 255, const Type outMaxValue = 1)
	{
		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				auto p = (double)inoutBuffer[y][x];

				inoutBuffer[y][x] = p / inMaxValue * outMaxValue;
			}
		}
	}

	//値のminmaxによってスケール
	template<typename Type>
	void ChangeColorScaleAdapted(Type** const inoutBuffer, const int width, const int height, const Type outMinValue = 0, const Type outMaxValue = 255)
	{
		Type bottom = std::numeric_limits<Type>::max();
		Type top = std::numeric_limits<Type>::min();

		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				auto p = inoutBuffer[y][x];

				bottom = p < bottom ? p : bottom;
				top = p > top ? p : top;
			}
		}

		auto colorRange = top - bottom;
		auto valueRange = outMaxValue - outMinValue;

		std::cout << "[" << bottom << ", " << top << "] => [" << outMinValue << ", " << outMaxValue << "]" << std::endl;

		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				auto p = (double)inoutBuffer[y][x];

				//0-1に変換後戻す
				auto r = (Type)((p - bottom) / colorRange * valueRange) + outMinValue;

				inoutBuffer[y][x] = r;
			}
		}
	}

	//中心当たりの画素でクリップ
	template<typename Type>
	void ClipToCenterValue(Type** const inoutBuffer, const int width, const int height, const int centerX, const int centerY, const int size)
	{
		double dMin = DBL_MAX;
		double dMax = DBL_MIN;

		//中心付近の値を調べる
		auto centerPixels = 0.0;
		auto pixelCount = 0;
		for (auto y = centerY - size / 2; y < centerY + size / 2 + 1; ++y)
		{
			for (auto x = centerX - size / 2; x < centerX + size / 2 + 1; ++x)
			{
				auto pix = inoutBuffer[y][x];
				centerPixels += pix;
				pixelCount++;

				dMin = pix < dMin ? pix : dMin;
				dMax = pix > dMax ? pix : dMax;

			}
		}
		std::cout << "dMin=" << dMin << ", dMax=" << dMax << std::endl;
		std::cout << "center pixels= " << centerPixels / pixelCount << std::endl;

		auto c = centerPixels / pixelCount;
		auto targetMinPixel = c - (c - dMin) / 10 * 15 * 4;
		auto targetMaxPixel = c + (dMax - c) / 10 * 15 * 4;

		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				auto pix = inoutBuffer[y][x];

				pix = pix < targetMinPixel ? targetMinPixel : pix;
				pix = pix > targetMaxPixel ? targetMaxPixel : pix;

				inoutBuffer[y][x] = pix;
			}
		}
	}

	std::vector<std::string> split(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		std::stringstream ss(s);
		std::string item;
		while (getline(ss, item, delim))
		{
			if (!item.empty())
			{
				elems.push_back(item);
			}
		}
		return elems;
	}

	bool SaveBuffer(double** const inBuffer, const int width, const int height, const std::string filePath)
	{
		//ファイルを保存する
		std::ofstream ofs(filePath);
		if (ofs.fail()) return false;

		//ヘッダー
		ofs << "DoubleImage, " << width << ", " << height << std::endl;

		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				auto pix = inBuffer[y][x];

				ofs << std::setprecision(20) << pix << ", ";
			}
			//1ラインごとに改行
			ofs << std::endl;
		}

		ofs.close();

		return true;
	}

	bool LoadBuffer(double** const outBuffer, const int width, const int height, const std::string filePath)
	{
		//ファイルを開く
		std::ifstream ifs(filePath);
		if (ifs.fail()) return false;

		std::string line;

		//ヘッダ捨て
		std::getline(ifs, line);

		for (auto y = 0; y < height; ++y)
		{
			std::getline(ifs, line);
			auto values = split(line, ',');

			for (auto x = 0; x < width; ++x)
			{
				auto value = std::atof(values[x].c_str());
				outBuffer[y][x] = value;
			}
		}

		ifs.close();
		return true;
	}

	bool GetBufferSizeFromFile(int&  width, int& height, const std::string filePath)
	{
		//ファイルを開く
		std::ifstream ifs(filePath);
		if (ifs.fail()) return false;

		ifs.imbue(std::locale::classic());

		std::string line;

		//ヘッダ読み出し
		std::getline(ifs, line);

		//フォーマット確認
		if (line.find("DoubleImage") < 0)
		{
			ifs.close();
			return false;
		}

		//高さと幅確認
		auto headers = split(line, ',');
		width = std::atoi(headers[1].c_str());
		height = std::atoi(headers[2].c_str());
		ifs.close();

		return true;
	}


protected:
	int width;
	int height;
	std::string originalFilePath;

	double** diffImgBG;
	double** diffImgGR;
	double** diffImgBR;

	double** diffImgXX;//for solver

	struct channelInfo
	{
		double** imgOriginal;
		double** imgDenoised;
		double** imgNormalX;
		double** imgNormalY;
		double** imgNormalZ;
	};

	channelInfo rInfo;
	channelInfo gInfo;
	channelInfo bInfo;

	channelInfo grayScale;

public:
	ImageDataManager(std::string filePath)
	{
		//ファイル読み込み
		auto imgMat = cv::imread(filePath);
		if (imgMat.empty()) throw;

		width = imgMat.cols;
		height = imgMat.rows;
		originalFilePath = filePath;

		//バッファ作成
		rInfo.imgOriginal = Create1chBuffer<double>(width, height);
		gInfo.imgOriginal = Create1chBuffer<double>(width, height);
		bInfo.imgOriginal = Create1chBuffer<double>(width, height);

		rInfo.imgDenoised = Create1chBuffer<double>(width, height);
		gInfo.imgDenoised = Create1chBuffer<double>(width, height);
		bInfo.imgDenoised = Create1chBuffer<double>(width, height);

		rInfo.imgNormalX = Create1chBuffer<double>(width, height);
		rInfo.imgNormalY = Create1chBuffer<double>(width, height);
		rInfo.imgNormalZ = Create1chBuffer<double>(width, height);

		gInfo.imgNormalX = Create1chBuffer<double>(width, height);
		gInfo.imgNormalY = Create1chBuffer<double>(width, height);
		gInfo.imgNormalZ = Create1chBuffer<double>(width, height);

		bInfo.imgNormalX = Create1chBuffer<double>(width, height);
		bInfo.imgNormalY = Create1chBuffer<double>(width, height);
		bInfo.imgNormalZ = Create1chBuffer<double>(width, height);

		grayScale.imgOriginal = Create1chBuffer<double>(width, height);
		grayScale.imgDenoised = Create1chBuffer<double>(width, height);
		grayScale.imgNormalX = Create1chBuffer<double>(width, height);
		grayScale.imgNormalY = Create1chBuffer<double>(width, height);
		grayScale.imgNormalZ = Create1chBuffer<double>(width, height);

		diffImgBG = Create1chBuffer<double>(width, height);
		diffImgGR = Create1chBuffer<double>(width, height);
		diffImgBR = Create1chBuffer<double>(width, height);
		diffImgXX = Create1chBuffer<double>(width, height);

		//RGBへ分解(OpenCVはBGR形式)
		CopyImgTo1chBuffer(imgMat, rInfo.imgOriginal, 2);
		CopyImgTo1chBuffer(imgMat, gInfo.imgOriginal, 1);
		CopyImgTo1chBuffer(imgMat, bInfo.imgOriginal, 0);

		//[0, 255] => [0, 1]へ変換
		ChangeColorScaleFixed(rInfo.imgOriginal, width, height, 255.0, 1.0);
		ChangeColorScaleFixed(gInfo.imgOriginal, width, height, 255.0, 1.0);
		ChangeColorScaleFixed(bInfo.imgOriginal, width, height, 255.0, 1.0);

		//grayscale作成
		ConvertRGBToGrayScale(rInfo.imgOriginal, gInfo.imgOriginal, bInfo.imgOriginal, grayScale.imgOriginal, width, height);
	}

	virtual ~ImageDataManager()
	{
		//開放
		Delete1chBuffer(rInfo.imgOriginal, width, height);
		Delete1chBuffer(gInfo.imgOriginal, width, height);
		Delete1chBuffer(bInfo.imgOriginal, width, height);

		Delete1chBuffer(rInfo.imgDenoised, width, height);
		Delete1chBuffer(gInfo.imgDenoised, width, height);
		Delete1chBuffer(bInfo.imgDenoised, width, height);

		Delete1chBuffer(rInfo.imgNormalX, width, height);
		Delete1chBuffer(rInfo.imgNormalY, width, height);
		Delete1chBuffer(rInfo.imgNormalZ, width, height);

		Delete1chBuffer(gInfo.imgNormalX, width, height);
		Delete1chBuffer(gInfo.imgNormalY, width, height);
		Delete1chBuffer(gInfo.imgNormalZ, width, height);

		Delete1chBuffer(bInfo.imgNormalX, width, height);
		Delete1chBuffer(bInfo.imgNormalY, width, height);
		Delete1chBuffer(bInfo.imgNormalZ, width, height);

		Delete1chBuffer(grayScale.imgOriginal, width, height);
		Delete1chBuffer(grayScale.imgDenoised, width, height);
		Delete1chBuffer(grayScale.imgNormalX, width, height);
		Delete1chBuffer(grayScale.imgNormalY, width, height);
		Delete1chBuffer(grayScale.imgNormalZ, width, height);

		Delete1chBuffer(diffImgBG, width, height);
		Delete1chBuffer(diffImgGR, width, height);
		Delete1chBuffer(diffImgBR, width, height);
		Delete1chBuffer(diffImgXX, width, height);
	}

	bool SaveDenoisedBuffers(bool overwrite = true)
	{
		auto extensionPos = originalFilePath.rfind('.');
		auto rPath = originalFilePath.substr(0, extensionPos) + "_denoiseR.cvs";
		auto gPath = originalFilePath.substr(0, extensionPos) + "_denoiseG.cvs";
		auto bPath = originalFilePath.substr(0, extensionPos) + "_denoiseB.cvs";
		auto grayPath = originalFilePath.substr(0, extensionPos) + "_denoiseGray.cvs";

		auto nxPath = originalFilePath.substr(0, extensionPos) + "_normalX.cvs";
		auto nyPath = originalFilePath.substr(0, extensionPos) + "_normalY.cvs";
		auto nzPath = originalFilePath.substr(0, extensionPos) + "_normalZ.cvs";

		if (!overwrite)
		{
			//ファイルの存在を確認
			std::ifstream ifsR(rPath);
			std::ifstream ifsG(gPath);
			std::ifstream ifsB(bPath);
			std::ifstream ifsGray(grayPath);
			std::ifstream ifsNX(nxPath);
			std::ifstream ifsNY(nyPath);
			std::ifstream ifsNZ(nzPath);

			//上書き不可なのにファイルが存在したとき
			if (ifsR.is_open() || ifsG.is_open() || ifsB.is_open() || ifsGray.is_open())
			{
				return false;
			}
			if (ifsNX.is_open() || ifsNY.is_open() || ifsNZ.is_open())
			{
				return false;
			}
		}

		//ファイル書き出し
		SaveBuffer(rInfo.imgDenoised, width, height, rPath);
		SaveBuffer(gInfo.imgDenoised, width, height, gPath);
		SaveBuffer(bInfo.imgDenoised, width, height, bPath);
		SaveBuffer(grayScale.imgDenoised, width, height, grayPath);
		SaveBuffer(grayScale.imgNormalX, width, height, nxPath);
		SaveBuffer(grayScale.imgNormalY, width, height, nyPath);
		SaveBuffer(grayScale.imgNormalZ, width, height, nzPath);
		return true;
	}

	bool SaveDiffBuffers(bool overwrite = true)
	{
		auto extensionPos = originalFilePath.rfind('.');
		auto bgPath = originalFilePath.substr(0, extensionPos) + "_diffBG.cvs";
		auto grPath = originalFilePath.substr(0, extensionPos) + "_diffGR.cvs";
		auto brPath = originalFilePath.substr(0, extensionPos) + "_diffBR.cvs";

		if (!overwrite)
		{
			//ファイルの存在を確認
			std::ifstream ifsBG(bgPath);
			std::ifstream ifsGR(grPath);
			std::ifstream ifsBR(brPath);

			//上書き不可なのにファイルが存在したとき
			if (ifsBG.is_open() || ifsGR.is_open() || ifsBR.is_open())
			{
				return false;
			}
		}

		//ファイル書き出し
		SaveBuffer(diffImgBG, width, height, bgPath);
		SaveBuffer(diffImgGR, width, height, grPath);
		SaveBuffer(diffImgBR, width, height, brPath);

		return true;
	}

	bool LoadDenoisedBuffers()
	{
		auto extensionPos = originalFilePath.rfind('.');
		auto rPath = originalFilePath.substr(0, extensionPos) + "_denoiseR.cvs";
		auto gPath = originalFilePath.substr(0, extensionPos) + "_denoiseG.cvs";
		auto bPath = originalFilePath.substr(0, extensionPos) + "_denoiseB.cvs";
		auto grayPath = originalFilePath.substr(0, extensionPos) + "_denoiseGray.cvs";

		auto nxPath = originalFilePath.substr(0, extensionPos) + "_normalX.cvs";
		auto nyPath = originalFilePath.substr(0, extensionPos) + "_normalY.cvs";
		auto nzPath = originalFilePath.substr(0, extensionPos) + "_normalZ.cvs";

		//ファイルの存在を確認
		{
			std::ifstream ifsR(rPath);
			std::ifstream ifsG(gPath);
			std::ifstream ifsB(bPath);
			std::ifstream ifsGray(grayPath);
			std::ifstream ifsNX(nxPath);
			std::ifstream ifsNY(nyPath);
			std::ifstream ifsNZ(nzPath);

			//ファイルがなかった時
			if (!ifsR.is_open() || !ifsG.is_open() || !ifsB.is_open() || !ifsGray.is_open())
			{
				return false;
			}
			if (!ifsNX.is_open() || !ifsNY.is_open() || !ifsNZ.is_open())
			{
				return false;
			}
		}

		//読み込み
		LoadBuffer(rInfo.imgDenoised, width, height, rPath);
		LoadBuffer(gInfo.imgDenoised, width, height, gPath);
		LoadBuffer(bInfo.imgDenoised, width, height, bPath);
		LoadBuffer(grayScale.imgDenoised, width, height, grayPath);
		LoadBuffer(grayScale.imgNormalX, width, height, nxPath);
		LoadBuffer(grayScale.imgNormalY, width, height, nyPath);
		LoadBuffer(grayScale.imgNormalZ, width, height, nzPath);

		return true;
	}

	bool LoadDiffBuffers()
	{
		auto extensionPos = originalFilePath.rfind('.');
		auto bgPath = originalFilePath.substr(0, extensionPos) + "_diffBG.cvs";
		auto grPath = originalFilePath.substr(0, extensionPos) + "_diffGR.cvs";
		auto brPath = originalFilePath.substr(0, extensionPos) + "_diffBR.cvs";

		//ファイルの存在を確認
		{
			std::ifstream ifsBG(bgPath);
			std::ifstream ifsGR(grPath);
			std::ifstream ifsBR(brPath);


			//ファイルがなかった時
			if (!ifsBG.is_open() || !ifsGR.is_open() || !ifsBR.is_open())
			{
				return false;
			}
		}

		//ファイル書き出し
		LoadBuffer(diffImgBG, width, height, bgPath);
		LoadBuffer(diffImgGR, width, height, grPath);
		LoadBuffer(diffImgBR, width, height, brPath);

		return true;
	}

	static double SubABWithPrecise(const double& a, const double& b)
	{
		return std::pow(a, 2) - std::pow(b, 2) / (a + b);
	}

	void ReplaceGrayBufferAsDenoisedGrayBuffer()
	{
		ConvertRGBToGrayScale(rInfo.imgDenoised, gInfo.imgDenoised, bInfo.imgDenoised, grayScale.imgOriginal, width, height);
	}

	void ClipDiffImagesWithCenterValue(const int centerOffsetX = 0, const int centerOffsetY = 0, const int size = 50);

	void ShowImages(bool original = false, bool denoised = false, bool diff = true);

};


