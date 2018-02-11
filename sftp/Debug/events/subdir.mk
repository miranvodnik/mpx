################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../events/SftpClientReply.cpp \
../events/SftpClientRequest.cpp \
../events/SftpClientStart.cpp \
../events/SftpClientStop.cpp \
../events/SftpInviteReply.cpp \
../events/SftpInviteRequest.cpp \
../events/SftpJobInfo.cpp 

OBJS += \
./events/SftpClientReply.o \
./events/SftpClientRequest.o \
./events/SftpClientStart.o \
./events/SftpClientStop.o \
./events/SftpInviteReply.o \
./events/SftpInviteRequest.o \
./events/SftpJobInfo.o 

CPP_DEPS += \
./events/SftpClientReply.d \
./events/SftpClientRequest.d \
./events/SftpClientStart.d \
./events/SftpClientStop.d \
./events/SftpInviteReply.d \
./events/SftpInviteRequest.d \
./events/SftpJobInfo.d 


# Each subdirectory must supply rules for building sources it contributes
events/%.o: ../events/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/sftp" -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


