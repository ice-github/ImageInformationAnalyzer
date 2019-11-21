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
  ${EIGEN3_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIR}
  ${CERES_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  )

