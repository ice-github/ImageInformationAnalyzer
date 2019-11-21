add_library(ImageInformationAnalyzer 
  STATIC
    DenoiseProcessCircleModel.cpp
    DenoiseProcessEllipseModel.cpp
    DenoiseProcessEllipseModelHeavy.cpp
    DenoiseProcessInterface.cpp
    ImageDataManager.cpp
    LightDirectionSolver.cpp
    RGBSpectrumDifferentialProcess.cpp
  )

target_include_directories(ImageInformationAnalyzer
  PUBLIC
  ${PROJECT_SOURCE_DIR}/src
  ${OpenCV_INCLUDE_DIRS}
  )

