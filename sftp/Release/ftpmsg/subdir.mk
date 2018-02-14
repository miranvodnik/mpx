################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ftpmsg/ftpmsg_xdr.c 

CPP_SRCS += \
../ftpmsg/FtpRequestInterface.cpp 

OBJS += \
./ftpmsg/FtpRequestInterface.o \
./ftpmsg/ftpmsg_xdr.o 

C_DEPS += \
./ftpmsg/ftpmsg_xdr.d 

CPP_DEPS += \
./ftpmsg/FtpRequestInterface.d 


# Each subdirectory must supply rules for building sources it contributes
ftpmsg/%.o: ../ftpmsg/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/sftp" -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

ftpmsg/%.o: ../ftpmsg/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/miran/eclipse-workspace/mpx/sftp" -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


