## Event Driven Task Multiplexing Library

The code contained in the mpx repository contains a library mpx-lib which allows the parallel execution of any number of software tasks in one or more working threads. In addition, the sftp library and the sftp-test program are also located in this repository, to test this library.

### Repository is composed of these parts:
- **mpx-lib** implementation of Event Driven Task Multiplexing Library
- **sftp** test library which implements FTP and SFTP clients. This library is an example of mpx-lib usage. It uses **mpx-lib** events to communicate with external environment. It also exploits the lower-level functionality of **mpx-lib** library in order to demonstrate how the principles of multiplexing can be more effectively applied
- **sftp-test** test program which uses **sftp** and **mpx-lib** libraries to demonstrate how can virtually any number of FTP/SFTP clients operate simultaneously using limited number of working threads and how can program communicate with them thus controlling their behaivior
