################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mpx-tasks/mpx-messages_xdr.c 

CPP_SRCS += \
../mpx-tasks/MpxExternalTask.cpp \
../mpx-tasks/MpxLocalClientProxyTask.cpp \
../mpx-tasks/MpxLocalEndPointProxyTask.cpp \
../mpx-tasks/MpxPosixMQProxyTask.cpp \
../mpx-tasks/MpxTaskBase.cpp \
../mpx-tasks/MpxTcp4ClientProxyTask.cpp \
../mpx-tasks/MpxTcp4EndPointProxyTask.cpp \
../mpx-tasks/MpxTcp6ClientProxyTask.cpp \
../mpx-tasks/MpxTcp6EndPointProxyTask.cpp 

OBJS += \
./mpx-tasks/MpxExternalTask.o \
./mpx-tasks/MpxLocalClientProxyTask.o \
./mpx-tasks/MpxLocalEndPointProxyTask.o \
./mpx-tasks/MpxPosixMQProxyTask.o \
./mpx-tasks/MpxTaskBase.o \
./mpx-tasks/MpxTcp4ClientProxyTask.o \
./mpx-tasks/MpxTcp4EndPointProxyTask.o \
./mpx-tasks/MpxTcp6ClientProxyTask.o \
./mpx-tasks/MpxTcp6EndPointProxyTask.o \
./mpx-tasks/mpx-messages_xdr.o 

C_DEPS += \
./mpx-tasks/mpx-messages_xdr.d 

CPP_DEPS += \
./mpx-tasks/MpxExternalTask.d \
./mpx-tasks/MpxLocalClientProxyTask.d \
./mpx-tasks/MpxLocalEndPointProxyTask.d \
./mpx-tasks/MpxPosixMQProxyTask.d \
./mpx-tasks/MpxTaskBase.d \
./mpx-tasks/MpxTcp4ClientProxyTask.d \
./mpx-tasks/MpxTcp4EndPointProxyTask.d \
./mpx-tasks/MpxTcp6ClientProxyTask.d \
./mpx-tasks/MpxTcp6EndPointProxyTask.d 


# Each subdirectory must supply rules for building sources it contributes
mpx-tasks/%.o: ../mpx-tasks/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

mpx-tasks/%.o: ../mpx-tasks/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


