# Enable Qt MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

# Executable
add_executable(librepcb_cli
    # Sources
    commandlineinterface.cpp
    main.cpp

    # Headers
    commandlineinterface.h
)
target_include_directories(librepcb_cli PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../libs")
target_link_libraries(librepcb_cli
    LibrePCB::LibraryManager
    LibrePCB::ProjectEditor
)
set_target_properties(librepcb_cli PROPERTIES OUTPUT_NAME librepcb-cli)

# In Qt 5.15, a lot of things were marked as deprecated, without providing
# alternatives available in previous Qt versions. It would require a lot of
# preprocessor conditionals to avoid these deprecation warnings, so let's just
# disable them for now. We are using CI anyway to ensure LibrePCB compiles with
# the targeted Qt versions.
if("${Qt5Core_VERSION_MAJOR}.${Qt5Core_VERSION_MINOR}" VERSION_GREATER 5.14)
    target_compile_options(librepcb_cli PRIVATE -Wno-deprecated-declarations)
endif()

# Install target
install(TARGETS librepcb_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
