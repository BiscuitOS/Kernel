BiscuitOS
--------------------------------------------

A tiny operate system running on i386. It's a funny toy for me.

### Basic Infomation

  * Current Version

    BiscuitOS v0.0.1

  * Platform

    Intel-i386/32bit

### Usage

  * Comple BiscuitOS

    ```
    make clean
    make 
    make start
    ```

  * Debug BiscuitOS

    On Host:
    ```
    make debug
    ```
    On Target:
    ```
    gdb tools/build/Image
    (gdb)target remote :1234
    (gdb)b *0x7C00
    (gdb)c
    (gdb)si
    ```
