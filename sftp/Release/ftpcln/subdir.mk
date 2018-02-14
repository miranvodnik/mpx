################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ftpcln/FtpClient.cpp \
../ftpcln/FtpClientInterface.cpp \
../ftpcln/SftpClient.cpp 

OBJS += \
./ftpcln/FtpClient.o \
./ftpcln/FtpClientInterface.o \
./ftpcln/SftpClient.o 

CPP_DEPS += \
./ftpcln/FtpClient.d \
./ftpcln/FtpClientInterface.d \
./ftpcln/SftpClient.d 


# Each subdirectory must supply rules for building sources it contributes
ftpcln/%.o: ../ftpcln/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/sftp" -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


