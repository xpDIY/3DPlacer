##### Project #####

PROJECT			?= app
# The path for generated files
BUILD_DIR		= Build

# MCU types: 
#   PY32F002Ax5
#   PY32F002Bx5
#   PY32F003x6, PY32F003x8, 
#   PY32F030x6, PY32F030x8, 
#   PY32F072xB
MCU_TYPE		= PY32F002Ax5

##### Options #####

# Use LL library instead of HAL, y:yes, n:no
USE_LL_LIB ?= n
# Enable printf float %f support, y:yes, n:no
ENABLE_PRINTF_FLOAT	?= n
# Build with FreeRTOS, y:yes, n:no
USE_FREERTOS	?= n
# Build with CMSIS DSP functions, y:yes, n:no
USE_DSP			?= n
# Build with Waveshare e-paper lib, y:yes, n:no
USE_EPAPER		?= n
# Programmer, jlink or pyocd
FLASH_PROGRM	?= pyocd

##### Toolchains #######

#ARM_TOOCHAIN	?= /opt/gcc-arm/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/bin
#ARM_TOOCHAIN	?= /opt/gcc-arm/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin
ARM_TOOCHAIN	?= /opt/gcc-arm/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin

# path to JLinkExe
JLINKEXE		?= /opt/SEGGER/JLink/JLinkExe
# path to PyOCD
PYOCD_EXE		?= pyocd

##### Paths ############

# C source folders
CDIRS	:= src src/commands ../common/src
# C source files (if there are any single ones)
CFILES := 

# ASM source folders
ADIRS	:= src/commands
# ASM single files
AFILES	:= 

# Include paths
INCLUDES	:= ../py32f0/Libraries/CMSIS/Core/Include \
			../py32f0/Libraries/CMSIS/Device/PY32F0xx/Include \
			src \
			src/commands \
			../common/src

##### Library Paths ############

# Library flags
LIB_FLAGS		= $(MCU_TYPE)
# JLink device (Uppercases)
JLINK_DEVICE	?= $(shell echo $(MCU_TYPE) | tr '[:lower:]' '[:upper:]')
# PyOCD device (Lowercases)
PYOCD_DEVICE	?= $(shell echo $(MCU_TYPE) | tr '[:upper:]' '[:lower:]')
# Link descript file: 
LDSCRIPT		= ../py32f0/Libraries/LDScripts/$(PYOCD_DEVICE).ld


ifneq (,$(findstring PY32F002B,$(MCU_TYPE)))

# PY32F002B >>>
CFILES		+= ../py32f0/Libraries/CMSIS/Device/PY32F0xx/Source/system_py32f002b.c

ifeq ($(USE_LL_LIB),y)
CDIRS		+= ../py32f0/Libraries/PY32F002B_LL_Driver/Src \
		../py32f0/Libraries/PY32F002B_LL_BSP/Src
INCLUDES	+= ../py32f0/Libraries/PY32F002B_LL_Driver/Inc \
		../py32f0/Libraries/PY32F002B_LL_BSP/Inc
LIB_FLAGS   += USE_FULL_LL_DRIVER
else
CDIRS		+= ../py32f0/Libraries/PY32F002B_HAL_Driver/Src \
		../py32f0/Libraries/PY32F002B_HAL_BSP/Src
INCLUDES	+= ../py32f0/Libraries/PY32F002B_HAL_Driver/Inc \
		../py32f0/Libraries/PY32F002B_HAL_BSP/Inc
endif
# Startup file
AFILES	:= ../py32f0/Libraries/CMSIS/Device/PY32F0xx/Source/gcc/startup_py32f002b.s
# PY32F002B <<<

else ifneq (,$(findstring PY32F07,$(MCU_TYPE)))

#  PY32F07x >>>
CFILES		+= ../py32f0/Libraries/CMSIS/Device/PY32F0xx/Source/system_py32f07x.c

CDIRS		+= ../py32f0/Libraries/PY32F07x_HAL_Driver/Src \
		../py32f0/Libraries/PY32F07x_HAL_BSP/Src
INCLUDES	+= ../py32f0/Libraries/PY32F07x_HAL_Driver/Inc \
		../py32f0/Libraries/PY32F07x_HAL_BSP/Inc
LIB_FLAGS   += USE_HAL_DRIVER
# Startup file
AFILES	:= ../py32f0/Libraries/CMSIS/Device/PY32F0xx/Source/gcc/startup_py32f072.s
#  PY32F07 <<<

else

# PY32F002A,003,030 >>>
CFILES		+= ../py32f0/Libraries/CMSIS/Device/PY32F0xx/Source/system_py32f0xx.c

ifeq ($(USE_LL_LIB),y)
CDIRS		+= ../py32f0/Libraries/PY32F0xx_LL_Driver/Src \
		../py32f0/Libraries/PY32F0xx_LL_BSP/Src
INCLUDES	+= ../py32f0/Libraries/PY32F0xx_LL_Driver/Inc \
		../py32f0/Libraries/PY32F0xx_LL_BSP/Inc
LIB_FLAGS   += USE_FULL_LL_DRIVER
else
CDIRS		+= ../py32f0/Libraries/PY32F0xx_HAL_Driver/Src \
		 ../py32f0/Libraries/PY32F0xx_HAL_BSP/Src
INCLUDES	+= ../py32f0/Libraries/PY32F0xx_HAL_Driver/Inc \
		 ../py32f0/Libraries/PY32F0xx_HAL_BSP/Inc
endif
# Startup file
ifneq (,$(findstring PY32F002A,$(LIB_FLAGS)))
AFILES	:= ../py32f0/Libraries/CMSIS/Device/PY32F0xx/Source/gcc/startup_py32f002a.s
endif
ifneq (,$(findstring PY32F003,$(LIB_FLAGS)))
AFILES	:= ../py32f0/Libraries/CMSIS/Device/PY32F0xx/Source/gcc/startup_py32f003.s
endif
ifneq (,$(findstring PY32F030,$(LIB_FLAGS)))
AFILES	:= ../py32f0/Libraries/CMSIS/Device/PY32F0xx/Source/gcc/startup_py32f030.s
endif
# PY32F002A,003,030 <<<

endif

######## Additional Libs ########

ifeq ($(USE_FREERTOS),y)
CDIRS		+= ../py32f0/Libraries/FreeRTOS \
			../py32f0/Libraries/FreeRTOS/portable/GCC/ARM_CM0

CFILES		+= ../py32f0/Libraries/FreeRTOS/portable/MemMang/heap_4.c

INCLUDES	+= ../py32f0/Libraries/FreeRTOS/include \
			../py32f0/Libraries/FreeRTOS/portable/GCC/ARM_CM0
endif

ifeq ($(USE_DSP),y)
CFILES 		+= ../py32f0/Libraries/CMSIS/DSP/Source/BasicMathFunctions/BasicMathFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/BayesFunctions/BayesFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/CommonTables/CommonTables.c \
		../py32f0/Libraries/CMSIS/DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/ControllerFunctions/ControllerFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/DistanceFunctions/DistanceFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/FastMathFunctions/FastMathFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/FilteringFunctions/FilteringFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/InterpolationFunctions/InterpolationFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/MatrixFunctions/MatrixFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/QuaternionMathFunctions/QuaternionMathFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/StatisticsFunctions/StatisticsFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/SupportFunctions/SupportFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/SVMFunctions/SVMFunctions.c \
		../py32f0/Libraries/CMSIS/DSP/Source/TransformFunctions/TransformFunctions.c
INCLUDES	+= ../py32f0/Libraries/CMSIS/DSP/Include \
		../py32f0/Libraries/CMSIS/DSP/PrivateInclude
endif

ifeq ($(USE_EPAPER),y)
CDIRS		+= ../py32f0/Libraries/EPaper/Lib \
			../py32f0/Libraries/EPaper/Examples \
			../py32f0/Libraries/EPaper/Fonts \
			../py32f0/Libraries/EPaper/GUI

INCLUDES	+= ../py32f0/Libraries/EPaper/Lib \
			../py32f0/Libraries/EPaper/Examples \
			../py32f0/Libraries/EPaper/Fonts \
			../py32f0/Libraries/EPaper/GUI
endif

include ./rules.mk
