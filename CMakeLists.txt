cmake_minimum_required(VERSION 3.16)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(bare_metal VERSION 1.0 LANGUAGES C ASM)

set(CMAKE_CROSSCOMPILING TRUE)

# Vendor headers copied locally from the Renesas package
# - vendor/CMSIS/Core/Include
# - vendor/Device/RENESAS/Include (including the cr/ subfolder)
set(VENDOR_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(CMSIS_CORE_INC  "${VENDOR_DIR}/CMSIS/Core/Include")
set(CMSIS_DEVICE_INC "${VENDOR_DIR}/Device/RENESAS/Include")

if(NOT EXISTS "${CMSIS_CORE_INC}")
    message(FATAL_ERROR
        "Missing CMSIS core headers at ${CMSIS_CORE_INC}. "
        "Copy rz/arm/CMSIS_6/CMSIS/Core/Include into vendor/CMSIS/Core/Include.")
endif()

if(NOT EXISTS "${CMSIS_DEVICE_INC}")
    message(FATAL_ERROR
        "Missing Renesas device headers at ${CMSIS_DEVICE_INC}. "
        "Copy rz/fsp/src/bsp/cmsis/Device/RENESAS/Include into vendor/Device/RENESAS/Include.")
endif()

set(CPU_FLAGS
    -mcpu=cortex-r52
    -mthumb
    -mthumb-interwork
    -fsigned-char
    -ffunction-sections
    -fdata-sections
)

add_executable(${PROJECT_NAME}.elf
    startup/startup.S
    bsp/mpu.c
    app/main.c
)

target_compile_options(${PROJECT_NAME}.elf PRIVATE
    $<$<COMPILE_LANGUAGE:C>:
        ${CPU_FLAGS}
        -std=c99
        -Wall -Wextra
        -Wmissing-declarations -Wconversion
        -fmessage-length=0
    >
    $<$<COMPILE_LANGUAGE:ASM>:
        ${CPU_FLAGS}
        -x assembler-with-cpp
    >
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE
    bsp
    app
    ${CMSIS_DEVICE_INC}
    ${CMSIS_DEVICE_INC}/cr
    ${CMSIS_CORE_INC}
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${CPU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/startup/linker.ld
    -Wl,--gc-sections
    -Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
    --specs=nano.specs
    -nostartfiles
)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O srec ${PROJECT_NAME}.elf ${PROJECT_NAME}.srec
    COMMENT "Generating S-record"
)
