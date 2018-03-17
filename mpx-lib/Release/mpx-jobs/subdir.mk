################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mpx-jobs/MpxExtTaskAddrInfo.cpp \
../mpx-jobs/MpxJob.cpp \
../mpx-jobs/MpxJobGetAddrInfo.cpp \
../mpx-jobs/MpxOpenLibrary.cpp 

OBJS += \
./mpx-jobs/MpxExtTaskAddrInfo.o \
./mpx-jobs/MpxJob.o \
./mpx-jobs/MpxJobGetAddrInfo.o \
./mpx-jobs/MpxOpenLibrary.o 

CPP_DEPS += \
./mpx-jobs/MpxExtTaskAddrInfo.d \
./mpx-jobs/MpxJob.d \
./mpx-jobs/MpxJobGetAddrInfo.d \
./mpx-jobs/MpxOpenLibrary.d 


# Each subdirectory must supply rules for building sources it contributes
mpx-jobs/%.o: ../mpx-jobs/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


