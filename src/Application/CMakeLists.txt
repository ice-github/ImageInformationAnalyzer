add_library(ImageInformationAnalyzerApplication
  STATIC
    DenoiseImageService.cpp
    EstimateLightDirectionService.cpp
    ImageEvaluationService.cpp
    ImageFileService.cpp
    ScaleImageService.cpp
    TakeDifferenceService.cpp
    TakeHistogramService.cpp
  )




target_include_directories(ImageInformationAnalyzerApplication
  PRIVATE
  ${PROJECT_SOURCE_DIR}/src/Domain
  ${PROJECT_SOURCE_DIR}/src/Application
  ${PROJECT_SOURCE_DIR}/src/Infrastructure #without DI
  ${EIGEN3_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIR}
  ${CERES_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  )

