################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mpx-working-threads/MpxWorkerTask.cpp \
../mpx-working-threads/MpxWorkingQueue.cpp 

OBJS += \
./mpx-working-threads/MpxWorkerTask.o \
./mpx-working-threads/MpxWorkingQueue.o 

CPP_DEPS += \
./mpx-working-threads/MpxWorkerTask.d \
./mpx-working-threads/MpxWorkingQueue.d 


# Each subdirectory must supply rules for building sources it contributes
mpx-working-threads/%.o: ../mpx-working-threads/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I/usr/include/libxml2 -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


