cmake_minimum_required(VERSION 3.15)

if(ESP_PLATFORM)
   # Build as an ESP-IDF component
   idf_component_register(
      SRCS "src/improv.cpp"
      INCLUDE_DIRS "src"
      # TODO: It would be nice to have this conditional only if arduino is already used as a component
      REQUIRES arduino
   )
   return()
endif()

project(improv VERSION 1.2.2)
