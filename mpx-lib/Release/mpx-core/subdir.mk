################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mpx-core/MpxEnvironment.cpp \
../mpx-core/MpxRunnable.cpp \
../mpx-core/MpxRunningContext.cpp \
../mpx-core/MpxTaskMultiplexer.cpp \
../mpx-core/MpxUtilities.cpp \
../mpx-core/MpxXDRProcRegistry.cpp 

OBJS += \
./mpx-core/MpxEnvironment.o \
./mpx-core/MpxRunnable.o \
./mpx-core/MpxRunningContext.o \
./mpx-core/MpxTaskMultiplexer.o \
./mpx-core/MpxUtilities.o \
./mpx-core/MpxXDRProcRegistry.o 

CPP_DEPS += \
./mpx-core/MpxEnvironment.d \
./mpx-core/MpxRunnable.d \
./mpx-core/MpxRunningContext.d \
./mpx-core/MpxTaskMultiplexer.d \
./mpx-core/MpxUtilities.d \
./mpx-core/MpxXDRProcRegistry.d 


# Each subdirectory must supply rules for building sources it contributes
mpx-core/%.o: ../mpx-core/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


