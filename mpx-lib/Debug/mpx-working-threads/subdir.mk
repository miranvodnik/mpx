################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mpx-working-threads/MpxJob.cpp \
../mpx-working-threads/MpxJobGetAddrInfo.cpp \
../mpx-working-threads/MpxWorkerTask.cpp \
../mpx-working-threads/MpxWorkingQueue.cpp 

OBJS += \
./mpx-working-threads/MpxJob.o \
./mpx-working-threads/MpxJobGetAddrInfo.o \
./mpx-working-threads/MpxWorkerTask.o \
./mpx-working-threads/MpxWorkingQueue.o 

CPP_DEPS += \
./mpx-working-threads/MpxJob.d \
./mpx-working-threads/MpxJobGetAddrInfo.d \
./mpx-working-threads/MpxWorkerTask.d \
./mpx-working-threads/MpxWorkingQueue.d 


# Each subdirectory must supply rules for building sources it contributes
mpx-working-threads/%.o: ../mpx-working-threads/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


