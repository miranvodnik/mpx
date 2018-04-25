################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ftplog/SftpLog.cpp 

OBJS += \
./ftplog/SftpLog.o 

CPP_DEPS += \
./ftplog/SftpLog.d 


# Each subdirectory must supply rules for building sources it contributes
ftplog/%.o: ../ftplog/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/examples/sftp" -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


