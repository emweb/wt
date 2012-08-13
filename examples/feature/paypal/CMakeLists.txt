WT_ADD_EXAMPLE(paypal.wt 
        PayPalExample.C
)

TARGET_LINK_LIBRARIES(paypal.wt)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
