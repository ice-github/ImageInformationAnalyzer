
#include <iostream>
#include <opencv2/opencv.hpp>

#include "DenoiseProcessCircleModel.hpp"
#include "DenoiseProcessEllipseModel.hpp"
#include "DenoiseProcessEllipseModelHeavy.hpp"
#include "RGBSpectrumDifferentialProcess.hpp"
#include "LightDirectionSolver.hpp"

#include "ImageInformationPresenter.hpp"

int main(int argc, char* argv[])
{
    if(argc < 2) return -1;

    using namespace ImageInformationAnalyzer::Presentation;
    ImageInformationPresenter iip(DenoiseImageService::Mode::ELLIPSE, ImageEvaluationService::Mode::SSIM, TakeDifferenceService::Mode::WholePixel);

    std::string filepath(argv[1]);

    try
    {
        iip.LoadImage(filepath);
        iip.Scale(); //Must excecute Scaler for Ellipse/HyperEllipse
        iip.DenoiseImage();
        iip.Evaluate();
        iip.Diff();
        //iip.LightEstimation();
        iip.TakeHistogram();
        iip.Show();
    }
    catch(const std::exception& e)
    {
        std::cout << "Exception: "s << e.what() << std::endl;
    }

    return 0;
}
