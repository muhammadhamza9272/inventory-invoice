cmake_minimum_required(VERSION 3.16)

project(inventory_invoice
    VERSION 0.1.0
    DESCRIPTION "Inventory and Invoice desktop application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Svg Test)

qt_standard_project_setup()

set(INVENTORY_INVOICE_SOURCES
    src/app/main.cpp
    src/app/demo_seed.cpp
    src/app/demo_seed.h
    src/model/invoice.cpp
    src/model/invoice.h
    src/model/product.h
    src/data/database.cpp
    src/data/database.h
    src/data/product_repository.cpp
    src/data/product_repository.h
    src/data/customer_repository.cpp
    src/data/customer_repository.h
    src/data/invoice_repository.cpp
    src/data/invoice_repository.h
    src/ui/main_window.cpp
    src/ui/main_window.h
    src/ui/product_table_model.cpp
    src/ui/product_table_model.h
    src/ui/product_dialog.cpp
    src/ui/product_dialog.h
    src/ui/line_item_table_model.cpp
    src/ui/line_item_table_model.h
    src/ui/customer_dialog.cpp
    src/ui/customer_dialog.h
    src/ui/customer_table_model.cpp
    src/ui/customer_table_model.h
    src/ui/invoice_editor_dialog.cpp
    src/ui/invoice_editor_dialog.h
    src/ui/invoice_list_model.cpp
    src/ui/invoice_list_model.h
    src/ui/invoice_pdf_exporter.cpp
    src/ui/invoice_pdf_exporter.h
    src/ui/invoices_tab.cpp
    src/ui/invoices_tab.h
    src/ui/customers_tab.cpp
    src/ui/customers_tab.h
    src/ui/sales_by_customer_model.cpp
    src/ui/sales_by_customer_model.h
    src/ui/reports_tab.cpp
    src/ui/reports_tab.h
)

if(APPLE)
    qt_add_executable(inventory_invoice MACOSX_BUNDLE ${INVENTORY_INVOICE_SOURCES})
    set_target_properties(inventory_invoice PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "Inventory & Invoice"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.muhammadhamza9272.inventory-invoice"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
else()
    qt_add_executable(inventory_invoice ${INVENTORY_INVOICE_SOURCES})
endif()

install(TARGETS inventory_invoice
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

target_include_directories(inventory_invoice PRIVATE src)

target_link_libraries(inventory_invoice PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Svg
)

enable_testing()
add_subdirectory(test)
