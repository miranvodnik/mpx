################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../EventA.cpp \
../EventB.cpp \
../TaskA.cpp \
../TaskB.cpp \
../mpx-test-1.cpp 

OBJS += \
./EventA.o \
./EventB.o \
./TaskA.o \
./TaskB.o \
./mpx-test-1.o 

CPP_DEPS += \
./EventA.d \
./EventB.d \
./TaskA.d \
./TaskB.d \
./mpx-test-1.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I"/home/miran/eclipse-workspace/mpx/mpx-test-1" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


