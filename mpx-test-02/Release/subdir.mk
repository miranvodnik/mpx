################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Answer.cpp \
../Person.cpp \
../Question.cpp \
../mpx-test-02.cpp 

OBJS += \
./Answer.o \
./Person.o \
./Question.o \
./mpx-test-02.o 

CPP_DEPS += \
./Answer.d \
./Person.d \
./Question.d \
./mpx-test-02.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/miran/eclipse-workspace/mpx/mpx-lib" -I"/home/miran/eclipse-workspace/mpx/mpx-test-02" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


