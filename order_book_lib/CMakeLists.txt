project(OrderBook_lib)

set(HEADER_FILES
        order.hpp
        order_book.hpp
        trade.hpp
        level.hpp
        order_utilities.hpp
)

set(SOURCE_FILES
        order_book.cpp
)

add_library(OrderBook_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})