################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TaskProvider.cpp \
../mpx-task-provider.cpp 

OBJS += \
./TaskProvider.o \
./mpx-task-provider.o 

CPP_DEPS += \
./TaskProvider.d \
./mpx-task-provider.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I"/home/miran/eclipse-workspace/mpx/examples/mpx-edlib" -I"/home/miran/eclipse-workspace/mpx/examples/mpx-task-provider-simple" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


