################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mpx-events/mpx-events_xdr.c 

CPP_SRCS += \
../mpx-events/MpxEventBase.cpp \
../mpx-events/MpxEventXDRItf.cpp \
../mpx-events/MpxExternalTaskEvent.cpp \
../mpx-events/MpxJobFinishedEvent.cpp \
../mpx-events/MpxLocalClientEvent.cpp \
../mpx-events/MpxLocalEndPointEvent.cpp \
../mpx-events/MpxLocalListenerEvent.cpp \
../mpx-events/MpxLocalTaskQueryEvent.cpp \
../mpx-events/MpxPosixMQEvent.cpp \
../mpx-events/MpxPosixMQTaskQueryEvent.cpp \
../mpx-events/MpxProxyTaskEvent.cpp \
../mpx-events/MpxProxyTaskRelocationEvent.cpp \
../mpx-events/MpxStartEvent.cpp \
../mpx-events/MpxStopEvent.cpp \
../mpx-events/MpxTaskQueryEvent.cpp \
../mpx-events/MpxTaskResponseEvent.cpp \
../mpx-events/MpxTcp4ClientEvent.cpp \
../mpx-events/MpxTcp4EndPointEvent.cpp \
../mpx-events/MpxTcp4ListenerEvent.cpp \
../mpx-events/MpxTcp4TaskQueryEvent.cpp \
../mpx-events/MpxTcp6ClientEvent.cpp \
../mpx-events/MpxTcp6EndPointEvent.cpp \
../mpx-events/MpxTcp6ListenerEvent.cpp \
../mpx-events/MpxTcp6TaskQueryEvent.cpp \
../mpx-events/MpxTimerEvent.cpp \
../mpx-events/MpxUdp4TaskQueryEvent.cpp \
../mpx-events/MpxUdp6TaskQueryEvent.cpp 

OBJS += \
./mpx-events/MpxEventBase.o \
./mpx-events/MpxEventXDRItf.o \
./mpx-events/MpxExternalTaskEvent.o \
./mpx-events/MpxJobFinishedEvent.o \
./mpx-events/MpxLocalClientEvent.o \
./mpx-events/MpxLocalEndPointEvent.o \
./mpx-events/MpxLocalListenerEvent.o \
./mpx-events/MpxLocalTaskQueryEvent.o \
./mpx-events/MpxPosixMQEvent.o \
./mpx-events/MpxPosixMQTaskQueryEvent.o \
./mpx-events/MpxProxyTaskEvent.o \
./mpx-events/MpxProxyTaskRelocationEvent.o \
./mpx-events/MpxStartEvent.o \
./mpx-events/MpxStopEvent.o \
./mpx-events/MpxTaskQueryEvent.o \
./mpx-events/MpxTaskResponseEvent.o \
./mpx-events/MpxTcp4ClientEvent.o \
./mpx-events/MpxTcp4EndPointEvent.o \
./mpx-events/MpxTcp4ListenerEvent.o \
./mpx-events/MpxTcp4TaskQueryEvent.o \
./mpx-events/MpxTcp6ClientEvent.o \
./mpx-events/MpxTcp6EndPointEvent.o \
./mpx-events/MpxTcp6ListenerEvent.o \
./mpx-events/MpxTcp6TaskQueryEvent.o \
./mpx-events/MpxTimerEvent.o \
./mpx-events/MpxUdp4TaskQueryEvent.o \
./mpx-events/MpxUdp6TaskQueryEvent.o \
./mpx-events/mpx-events_xdr.o 

C_DEPS += \
./mpx-events/mpx-events_xdr.d 

CPP_DEPS += \
./mpx-events/MpxEventBase.d \
./mpx-events/MpxEventXDRItf.d \
./mpx-events/MpxExternalTaskEvent.d \
./mpx-events/MpxJobFinishedEvent.d \
./mpx-events/MpxLocalClientEvent.d \
./mpx-events/MpxLocalEndPointEvent.d \
./mpx-events/MpxLocalListenerEvent.d \
./mpx-events/MpxLocalTaskQueryEvent.d \
./mpx-events/MpxPosixMQEvent.d \
./mpx-events/MpxPosixMQTaskQueryEvent.d \
./mpx-events/MpxProxyTaskEvent.d \
./mpx-events/MpxProxyTaskRelocationEvent.d \
./mpx-events/MpxStartEvent.d \
./mpx-events/MpxStopEvent.d \
./mpx-events/MpxTaskQueryEvent.d \
./mpx-events/MpxTaskResponseEvent.d \
./mpx-events/MpxTcp4ClientEvent.d \
./mpx-events/MpxTcp4EndPointEvent.d \
./mpx-events/MpxTcp4ListenerEvent.d \
./mpx-events/MpxTcp4TaskQueryEvent.d \
./mpx-events/MpxTcp6ClientEvent.d \
./mpx-events/MpxTcp6EndPointEvent.d \
./mpx-events/MpxTcp6ListenerEvent.d \
./mpx-events/MpxTcp6TaskQueryEvent.d \
./mpx-events/MpxTimerEvent.d \
./mpx-events/MpxUdp4TaskQueryEvent.d \
./mpx-events/MpxUdp6TaskQueryEvent.d 


# Each subdirectory must supply rules for building sources it contributes
mpx-events/%.o: ../mpx-events/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I/usr/include/libxml2 -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

mpx-events/%.o: ../mpx-events/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I/usr/include/libxml2 -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


