#
# LGPL Library
#
# This MUST be a dynamic link library to avoid LGPL license infection.
#

include(GenerateExportHeader)

set(kolourpaint_lgpl_SRCS
    generic/kpUrlFormatter.cpp
    generic/kpColorCollection.cpp
    generic/widgets/kpColorCellsBase.cpp
)

add_library(kolourpaint_lgpl SHARED ${kolourpaint_lgpl_SRCS})

set(kolourpaint_lgpl_version 5)

target_link_libraries(kolourpaint_lgpl
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::GuiAddons
    KF${KF_MAJOR_VERSION}::WidgetsAddons
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::JobWidgets
    Qt${QT_MAJOR_VERSION}::Widgets
)

set_target_properties(kolourpaint_lgpl
    PROPERTIES
        VERSION ${kolourpaint_lgpl_version}
        DEFINE_SYMBOL MAKE_KOLOURPAINT4_LGPL_LIB
)

generate_export_header(kolourpaint_lgpl BASE_NAME kolourpaint_lgpl)

install(TARGETS kolourpaint_lgpl ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
