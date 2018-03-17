################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mpx-events_xdr.c 

CPP_SRCS += \
../MpxConsumerEventA.cpp \
../MpxConsumerEventB.cpp \
../MpxConsumerEventXDR.cpp 

OBJS += \
./MpxConsumerEventA.o \
./MpxConsumerEventB.o \
./MpxConsumerEventXDR.o \
./mpx-events_xdr.o 

C_DEPS += \
./mpx-events_xdr.d 

CPP_DEPS += \
./MpxConsumerEventA.d \
./MpxConsumerEventB.d \
./MpxConsumerEventXDR.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I"/home/miran/eclipse-workspace/mpx/mpx-edlib" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I"/home/miran/eclipse-workspace/mpx/mpx-edlib" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


