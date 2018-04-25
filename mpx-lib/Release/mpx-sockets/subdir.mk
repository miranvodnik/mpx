################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mpx-sockets/MpxLocalClient.cpp \
../mpx-sockets/MpxLocalEndPoint.cpp \
../mpx-sockets/MpxLocalListener.cpp \
../mpx-sockets/MpxPosixMQ.cpp \
../mpx-sockets/MpxTcp4Client.cpp \
../mpx-sockets/MpxTcp4EndPoint.cpp \
../mpx-sockets/MpxTcp4Listener.cpp \
../mpx-sockets/MpxTcp6Client.cpp \
../mpx-sockets/MpxTcp6EndPoint.cpp \
../mpx-sockets/MpxTcp6Listener.cpp 

OBJS += \
./mpx-sockets/MpxLocalClient.o \
./mpx-sockets/MpxLocalEndPoint.o \
./mpx-sockets/MpxLocalListener.o \
./mpx-sockets/MpxPosixMQ.o \
./mpx-sockets/MpxTcp4Client.o \
./mpx-sockets/MpxTcp4EndPoint.o \
./mpx-sockets/MpxTcp4Listener.o \
./mpx-sockets/MpxTcp6Client.o \
./mpx-sockets/MpxTcp6EndPoint.o \
./mpx-sockets/MpxTcp6Listener.o 

CPP_DEPS += \
./mpx-sockets/MpxLocalClient.d \
./mpx-sockets/MpxLocalEndPoint.d \
./mpx-sockets/MpxLocalListener.d \
./mpx-sockets/MpxPosixMQ.d \
./mpx-sockets/MpxTcp4Client.d \
./mpx-sockets/MpxTcp4EndPoint.d \
./mpx-sockets/MpxTcp4Listener.d \
./mpx-sockets/MpxTcp6Client.d \
./mpx-sockets/MpxTcp6EndPoint.d \
./mpx-sockets/MpxTcp6Listener.d 


# Each subdirectory must supply rules for building sources it contributes
mpx-sockets/%.o: ../mpx-sockets/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I/usr/include/libxml2 -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


