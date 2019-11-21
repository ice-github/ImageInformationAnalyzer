
#include <iostream>
#include <opencv2/opencv.hpp>

#include "DenoiseProcessCircleModel.hpp"
#include "DenoiseProcessEllipseModel.hpp"
#include "DenoiseProcessEllipseModelHeavy.hpp"
#include "RGBSpectrumDifferentialProcess.hpp"
#include "LightDirectionSolver.hpp"

void main()
{
	ImageDataManager idm("index3.jpg");

	int windowsSize = 7;

	//ノイズ除去
	if (!idm.LoadDenoisedBuffers())
	{
		DenoiseProcessCircleModel model(windowsSize);
		//DenoiseProcessEllipseModel model(windowsSize);
		//DenoiseProcessEllipseModelHeavy model(windowsSize);

		//処理
		model.Process(idm);

		//保存
		idm.SaveDenoisedBuffers();
	}

	//スペクトル差分
	if (!idm.LoadDiffBuffers())
	{
		RGBSpectrumDifferentialProcess spectrumDiff(windowsSize);

		spectrumDiff.Process(idm);

		//保存
		idm.SaveDiffBuffers();
	}

	//ソルバー
	LightDirectionSolver lds;
	lds.Run(idm);

	//差分画像を値クリップ
	idm.ClipDiffImagesWithCenterValue(0, 0, 30);

	//表示
	idm.ShowImages(true, true, true);
}
