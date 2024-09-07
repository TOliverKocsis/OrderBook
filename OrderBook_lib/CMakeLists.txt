project(OrderBook_lib)

set(HEADER_FILES
        Order.hpp
        OrderBook.hpp
        Trade.hpp
        Level.hpp
)

set(SOURCE_FILES
        OrderBook.cpp
)

add_library(OrderBook_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})