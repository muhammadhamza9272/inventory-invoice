add_executable(inventory_invoice_test
    test_main.cpp
    ../src/model/invoice.cpp
    ../src/model/invoice.h
    ../src/model/product.h
    ../src/data/database.cpp
    ../src/data/database.h
    ../src/data/product_repository.cpp
    ../src/data/product_repository.h
    ../src/data/customer_repository.cpp
    ../src/data/customer_repository.h
    ../src/data/invoice_repository.cpp
    ../src/data/invoice_repository.h
)
target_include_directories(inventory_invoice_test PRIVATE ../src)
target_link_libraries(inventory_invoice_test PRIVATE Qt6::Core Qt6::Test Qt6::Sql)
include(CTest)
if(BUILD_TESTING)
    add_test(NAME inventory_invoice_test COMMAND inventory_invoice_test)
endif()
