# fileMonitor

This program takes a relative or absolute path and monitors it for changes. 
The program ends if no changes occur for 10 consecutive seconds

The epoll and inotify linux system calls are used.

To compile type make. A Makefile is included that assumes you have gcc installed
Then type ./fileMonitor [name of some file or directory]
