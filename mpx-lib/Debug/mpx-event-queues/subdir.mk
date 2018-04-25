################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mpx-event-queues/MpxEventQueueRepository.cpp \
../mpx-event-queues/MpxLocalMQTask.cpp \
../mpx-event-queues/MpxMQTaskI.cpp \
../mpx-event-queues/MpxPosixMQTask.cpp 

OBJS += \
./mpx-event-queues/MpxEventQueueRepository.o \
./mpx-event-queues/MpxLocalMQTask.o \
./mpx-event-queues/MpxMQTaskI.o \
./mpx-event-queues/MpxPosixMQTask.o 

CPP_DEPS += \
./mpx-event-queues/MpxEventQueueRepository.d \
./mpx-event-queues/MpxLocalMQTask.d \
./mpx-event-queues/MpxMQTaskI.d \
./mpx-event-queues/MpxPosixMQTask.d 


# Each subdirectory must supply rules for building sources it contributes
mpx-event-queues/%.o: ../mpx-event-queues/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I/usr/include/libxml2 -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


