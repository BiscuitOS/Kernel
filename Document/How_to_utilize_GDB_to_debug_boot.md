How to utilize GDB to debug boot
-------------------------------------------------

## Tools

  GDB, qemu

## Debug

  On host
  ```
  cd boot
  make debug
  ```
  
  On Target
  ```
  gdb boot/boot
  (gdb) file boot/bootsect.s
  (gdb) target remote :1234
  (gdb) b *0x7C00
  (gdb) c
  (gdb) si
  ```

## Usage for GDB

  When you have connected target on debuging, you can use more useful 
  option to debug program, so we will descript how to use option to debug 
  your program.

  * Running program

    ```
    c
	```

  * Set breakpoint

    Set breakpoint with absolute address.
	```
	b *0x7C00
	```
	Set breakpoint with function.
	```
    b main
	```
    Check breakpoint
	```
	info b
	```

  * Show value
    
	We can use option of GDB to get value of "value", 
	"register", "memory" and etc.

	Print value of all register, we can use "info":
	```
	info reg
	```
	When we want to check value of memory, we can use `x/<n/f/u> <addr>`
	
	`n` is length of display for memory that will prints `n` values after
	`addr`,

	`f` is format for displaying, it's optional, we can get format output that it set. The value of `f` is:
	```
	x: display in 16-hexadecimal
	d: display in 10-hexadecimal
	o: display in 8-hexadecimal
	t: display in 2-hexadecimal
	c: display in character
	```
	`u` is number of byte as a value, the default value is 4 bytes.
	The value of `u` is:
	```
	b: single byte
	h: double bytes
	w: quat bytes
	g: eight bytes

	```



