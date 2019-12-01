add_executable(sample main.cpp)

target_include_directories(sample
  PRIVATE
  ${PROJECT_SOURCE_DIR}/src/Domain
  ${PROJECT_SOURCE_DIR}/src/Application
  ${PROJECT_SOURCE_DIR}/src/Presentation
  ${EIGEN3_INCLUDE_DIR}
  ${CVUI_INCLUDE_DIR}
  )

target_link_libraries(sample 
    ImageInformationAnalyzerDomain
    ImageInformationAnalyzerApplication
    ImageInformationAnalyzerPresentation
    ImageInformationAnalyzerInfrastructure
    ${OpenCV_LIBS}
    ${CERES_LIBRARIES}
    ${GLOG_LIBRARIES}
  )

