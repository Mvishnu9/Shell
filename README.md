# Shell
## A Basic implementation of a Linux shell.

#### Supports the following shell builtins -
 ```
 echo [text]
 pwd
 cd [path]
 exit
 ```
#### Support for ls command with -a -l flags.
```
ls [flags] [path]
```
#### Custom command _pinfo_ 
```
pinfo [PID]
```
Defaults to Process info of current process without user defined PID.

#### Custom command _nightswatch_
```
nightswatch -n [time-interval] [interrupt/dirty]
```
_interrupt_  displays number of keyboard interrupts in gaps of the specified time interval.
_dirty_ displays size of dirty memory in gaps of the specified time interval.
Press 'q' to quit.

#### Support for system commands
Various system commands such as ```emacs, gedit, ps``` etc. can also be executed.

#### Background Processes
Background Processes can be started by using ```&``` at the end of a command.

## Compilation
```
gcc shell.c -lm
```
## Running the shell
```
./a.out
```
