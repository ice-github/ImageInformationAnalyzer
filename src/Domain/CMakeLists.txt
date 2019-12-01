add_library(ImageInformationAnalyzerDomain 
  STATIC
    DenoiseImageData.cpp
    DifferentialData.cpp
    FloatingPointImageData.cpp
    HistogramData.cpp
    ImageEvaluationData.cpp
    ImageFileData.cpp
    ImageUtility.cpp
    LightEstimationData.cpp
    ScaleImageData.cpp
  )

target_include_directories(ImageInformationAnalyzerDomain
  PRIVATE
  ${PROJECT_SOURCE_DIR}/src/Domain
  ${EIGEN3_INCLUDE_DIR}
  )

