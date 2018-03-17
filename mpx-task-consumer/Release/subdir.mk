################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TaskConsumer.cpp \
../mpx-task-consumer.cpp 

OBJS += \
./TaskConsumer.o \
./mpx-task-consumer.o 

CPP_DEPS += \
./TaskConsumer.d \
./mpx-task-consumer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I"/home/miran/eclipse-workspace/mpx/mpx-edlib" -I"/home/miran/eclipse-workspace/mpx/mpx-task-consumer" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


