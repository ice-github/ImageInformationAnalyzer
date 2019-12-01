add_library(ImageInformationAnalyzerInfrastructure
  STATIC
    CircleDenoiseDataRepository.cpp
    EachPixelSpectrumDifferentialDataRepository.cpp
    EllipseDenoiseDataRepository.cpp
    GraphicFileDataRepository.cpp
    HyperEllipseDenoiseDataRepository.cpp
    NormalizeScaleImageDataRepository.cpp
    PhongModelLightDirectionDataRepository.cpp
    PSNRIImageEvaluationDataRepository.cpp
    RoundOffHistogramDataRepository.cpp
    SSIMIImageEvaluationDataRepository.cpp
    WholePixelSpectrumDifferentialDataRepository.cpp
  )



target_include_directories(ImageInformationAnalyzerInfrastructure
  PRIVATE
  ${PROJECT_SOURCE_DIR}/src/Domain
  ${PROJECT_SOURCE_DIR}/src/Infrastructure
  ${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${GLOG_INCLUDE_DIR}
  ${CERES_INCLUDE_DIR}
  )

