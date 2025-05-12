cmake_minimum_required(VERSION 3.15)

if(ESP_PLATFORM)
	idf_component_register(
      SRCS "src/improv.cpp"
      INCLUDE_DIRS "src"
   )
	return()
endif()

project(improv VERSION 1.2.5)
