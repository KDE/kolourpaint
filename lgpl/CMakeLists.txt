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
    KF5::I18n
    KF5::GuiAddons
    KF5::WidgetsAddons
    KF5::KIOCore
    KF5::JobWidgets
    Qt5::Widgets
)

set_target_properties(kolourpaint_lgpl
    PROPERTIES
        VERSION ${kolourpaint_lgpl_version}
        DEFINE_SYMBOL MAKE_KOLOURPAINT4_LGPL_LIB
)

generate_export_header(kolourpaint_lgpl BASE_NAME kolourpaint_lgpl)

install(TARGETS kolourpaint_lgpl ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
