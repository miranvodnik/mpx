################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Task003.cpp \
../mpx-test-03.cpp 

OBJS += \
./Task003.o \
./mpx-test-03.o 

CPP_DEPS += \
./Task003.d \
./mpx-test-03.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-test-03" -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


