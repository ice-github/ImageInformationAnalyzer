add_library(ImageInformationAnalyzerPresentation
  STATIC
    ImageInformationModel.cpp
    ImageInformationPresenter.cpp
  )



target_include_directories(ImageInformationAnalyzerPresentation
  PRIVATE
  ${PROJECT_SOURCE_DIR}/src/Domain
  ${PROJECT_SOURCE_DIR}/src/Application
  ${PROJECT_SOURCE_DIR}/src/Presentation
  ${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${CVUI_INCLUDE_DIR}
  )

