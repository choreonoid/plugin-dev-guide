set(sources DevGuidePlugin.cpp BodyPositionItem.cpp BodyPositionItemRegistration.cpp BodyPositionItemView.cpp)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  # Build as a master project
  cmake_minimum_required(VERSION 3.10)
  project(DevGuidePlugin)
  find_package(Choreonoid REQUIRED)
  set(CMAKE_CXX_STANDARD ${CHOREONOID_CXX_STANDARD})
  choreonoid_add_plugin(CnoidDevGuidePlugin ${sources})
  target_link_libraries(CnoidDevGuidePlugin Choreonoid::CnoidBody)

else()
  # Build as a bundled project
  choreonoid_add_plugin(CnoidDevGuidePlugin ${sources})
  target_link_libraries(CnoidDevGuidePlugin CnoidBodyPlugin)
endif()
