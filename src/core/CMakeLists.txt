# CORE LIB


set(CORE_SRCS 
      blink.c
      entry.c
   )

idf_component_register(
   SRCS ${CORE_SRCS}
   INCLUDE_DIRS 
               "." 
               "../../include"
   REQUIRES 
      freertos 
      esp_driver_gpio
)
