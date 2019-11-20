
#include <iostream>
#include <opencv2/opencv.hpp>

#ifdef _DEBUG

#pragma comment(lib, "opencv_world320d.lib")

#else

#pragma comment(lib, "opencv_world320.lib")

#endif

#include "DenoiseProcessCircleModel.h"
#include "DenoiseProcessEllipseModel.h"
#include "DenoiseProcessEllipseModelHeavy.h"
#include "RGBSpectrumDifferentialProcess.h"
#include "LightDirectionSolver.h"

void main()
{
	ImageDataManager idm("index3.jpg");

	int windowsSize = 7;

	//�m�C�Y����
	if (!idm.LoadDenoisedBuffers())
	{
		DenoiseProcessCircleModel model(windowsSize);
		//DenoiseProcessEllipseModel model(windowsSize);
		//DenoiseProcessEllipseModelHeavy model(windowsSize);

		//����
		model.Process(idm);

		//�ۑ�
		idm.SaveDenoisedBuffers();
	}

	//�X�y�N�g������
	if (!idm.LoadDiffBuffers())
	{
		RGBSpectrumDifferentialProcess spectrumDiff(windowsSize);

		spectrumDiff.Process(idm);

		//�ۑ�
		idm.SaveDiffBuffers();
	}

	//�\���o�[
	LightDirectionSolver lds;
	lds.Run(idm);

	//�����摜��l�N���b�v
	idm.ClipDiffImagesWithCenterValue(0, 0, 30);

	//�\��
	idm.ShowImages(true, true, true);
}
