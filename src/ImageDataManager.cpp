
#include "ImageDataManager.hpp"

void ImageDataManager::ShowImages(bool original, bool denoised, bool diff)
{
	//[0, 1]Ç÷ïœä∑
	ChangeColorScaleAdapted(rInfo.imgDenoised, width, height, 0.0, 1.0);
	ChangeColorScaleAdapted(gInfo.imgDenoised, width, height, 0.0, 1.0);
	ChangeColorScaleAdapted(bInfo.imgDenoised, width, height, 0.0, 1.0);
	ChangeColorScaleAdapted(grayScale.imgDenoised, width, height, 0.0, 1.0);
	ChangeColorScaleAdapted(diffImgBG, width, height, 0.0, 1.0);
	ChangeColorScaleAdapted(diffImgGR, width, height, 0.0, 1.0);
	ChangeColorScaleAdapted(diffImgBR, width, height, 0.0, 1.0);
	ChangeColorScaleAdapted(diffImgXX, width, height, 0.0, 1.0);

	//åãâ èoóÕ
	auto rDenoisedMat = ConvertTo1chMat(rInfo.imgDenoised, width, height);
	auto gDenoisedMat = ConvertTo1chMat(gInfo.imgDenoised, width, height);
	auto bDenoisedMat = ConvertTo1chMat(bInfo.imgDenoised, width, height);
	auto grayDenoisedMat = ConvertTo1chMat(grayScale.imgDenoised, width, height);
	auto diffBGMat = ConvertTo1chMat(diffImgBG, width, height);
	auto diffGRMat = ConvertTo1chMat(diffImgGR, width, height);
	auto diffBRMat = ConvertTo1chMat(diffImgBR, width, height);
	auto diffXXMat = ConvertTo1chMat(diffImgXX, width, height);

	cv::namedWindow("DenoiseR", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("DenoiseG", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("DenoiseB", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("DenoiseGray", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("DenoiseR", rDenoisedMat);
	cv::imshow("DenoiseG", gDenoisedMat);
	cv::imshow("DenoiseB", bDenoisedMat);
	cv::imshow("DenoiseGray", grayDenoisedMat);

	cv::namedWindow("BG", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("GR", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("BR", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("BG", diffBGMat);
	cv::imshow("GR", diffGRMat);
	cv::imshow("BR", diffBRMat);

	cv::namedWindow("XX", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("XX", diffXXMat);


	//ÉLÅ[ì¸óÕë“Çø
	cv::waitKey(0);
}

void ImageDataManager::ClipDiffImagesWithCenterValue(const int centerOffsetX, const int centerOffsetY, const int size)
{
	ClipToCenterValue(diffImgBG, width, height, width / 2 + centerOffsetX, height / 2 + centerOffsetY, size);
	ClipToCenterValue(diffImgGR, width, height, width / 2 + centerOffsetX, height / 2 + centerOffsetY, size);
	ClipToCenterValue(diffImgBR, width, height, width / 2 + centerOffsetX, height / 2 + centerOffsetY, size);
	ClipToCenterValue(diffImgXX, width, height, width / 2 + centerOffsetX, height / 2 + centerOffsetY, size);
}
