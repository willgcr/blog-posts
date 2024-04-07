# Understanding Stack Overflow (with C & Assembly)

The following text was published by me about a decade ago for the Cyberia forum (an old school ethical hacking community)... I think the year was 2013, good times! Anyway, this is somehow a relic which does not deserve to fade into oblivion, and I don't want it to get vanished from the most important human knowledge repository (a.k.a. World Wide Web). So, I am republishing it here on my personal blog. It's from the time when my routine was not limited to PHP, Python and JavaScript; what I loved the most at that time was C and also Assembly (and I still passionately love them). Without further ado, here it goes with a few updates!

![The image is a meme featuring two girls posing for the camera with the text "I heard he can code without Stack Overflow."](https://i.ytimg.com/vi/uIuDvzXeYQM/maxresdefault.jpg)

Deoes anyone who started programming like after 2022 or 2023 knows what stack overflow means? And please do not let [StackOverflow](https://stackoverflow.com/) die! And No, this text is not about this [StackOverflow](https://stackoverflow.com/).

If you don't know what Stack Overflow means, I'll try to explain along the text and hopefully you'll be able to get what it needs to understand. If you are scared of Assembly (do you know what Assembly is?) and/or don't care how your computer processor and operating system works, just follow your path, because things here gonna get low level.

Have a good reading!

## Intro
Throughout the current text I'll try to do a brief explanation on how stack overflow works, why it happens and I'll also show a practical example on how this vulnerability is exploited (a very dummy example, must say). I'm going to use some concepts of [x86 Assembly](https://en.wikipedia.org/wiki/X86_assembly_language) and [C](https://en.wikipedia.org/wiki/C_(programming_language)), and with little to no changes you'll be able to reproduce everything on your computer as well.

**ATTENTION:** I do not encourage the use of this knowledge to circumvent, tamper with or modify in any way third-party programs without consent. The text presented here is of a purely educational nature.

## The environment
I'll be using my personal computer, which is currently running Linux Mint 21.3, with kernel 5.15.0-101-generic x86_64. Originally, this post was written on a 32-bit virtual machine, so I'll do my best to update this accordingly. The topics to be covered, are aimed at the [*x86_64*](https://en.wikipedia.org/wiki/X86-64), or *AMD64*, or *Intel 64* processor architecture (they are all the same). I know, I know, CPU architecture names are weird, but you can check your processor with the following command on your terminal: `lscpu`, it is from a package called `util-linux`, in case you don't have it installed already.

During the explanation, I will be using the following programs:

- **Gnu Compiler Collection** (better known as “gcc”)
- **Gnu Debugger** (better known as “gdb”)
- **Vim** (my code editor of choice for this post, just because it is cool)

## Basic Concepts
### Buffer Overflow
In programming, a buffer is a data storage area (think of it like a piece of memory) used to hold data that is being transferred from one place to another. In high level programming languages, like Python, we don't need to define sizes of buffers when we declare variables, there is no need to set the size of a string when we declare it or even care about how many characters a string is going to receive later on.

On the other hand, in Languages like C and C++ we need to have the buffer size pre-defined or dynamically allocated (we say to the computer "hey, I need N bytes of memory to hold this string").

A buffer overflow is a type of vulnerability that occurs when a program writes more data to a block of memory, or buffer, than it was allocated to hold. This excess data can overflow into adjacent memory, corrupting or overwriting the data stored there.

Example:

```c
// overflow0.c

#include <stdio.h>

int main (void){
	int myArray[6] = {1, 2, 3, 4, 5, 6};
	printf ("%d\n", myAarray[6]);
	return (0);
}

// To compile: gcc -o overflow0 overflow0.c
```

This is a common mistake for beginners in programming, the variable **myArray[6]** was defined as an array of 6 integers, so far so good. Each of these six numbers can be accessed by their indexes **[0]**, **[1]**, **[2]**, **[3]**, **[4]** and **[5]**. 

When we try to print **myArray[6]**, what we are actually printing is the seventh element of the array. As there is no seventh element, we end up referring to “garbage”. It happens because in C we are allowed to access indexes that are beyond the size declared for the array. The position we access is calculated according to the type of data the array stores and its starting address in memory, therefore, the two printfs below are equivalent:

```c
// overflow1.c

#include <stdio.h>

void main (void){
	int array[6] = {1, 2, 3, 4, 5, 6};
	printf ("%d\n", array[6]);
	printf ("%d\n", *(array+6));
	return;
}

// To compile: gcc -o overflow1 overflow1.c
```

If reading values outside the declared array limit is that simple, writing values is also! Let's take a look at an example:

```c
// overflow2.c

#include <stdio.h>

void main (void){
   int myArray[25];
   myArray[0] = 0;
   myArray[24] = 1;
   myArray[25] = 2;
   myArray[30] = 5;
   return;
}

// To compile: gcc -o overflow2 overflow2.c -fno-stack-protector -O0
```

**NOTE:** It happens that when I wrote the first version of this article, the **gcc** was not configured to prevent or detect buffer overflow by default. But now, by writing the current revision, I've seen that my **gcc** build is configured to use a technique called *Stack Smashing Protection* or *Stack Guard* by default, I can talk about it in a different article but for now we need to make sure we're disabling the *Stack Guard* when we compile our code, that's why I'm using the **-fno-stack-protector** option when compiling, I'm also using **-O0** to prevent other optimizations. Important to notice that *Stack Guard* is not a failproof technique, and this leads to a very interesting discussion, but it's not our focus here, let's proceed.

In this example we write to a memory location that is outside the array limit we declared. Now let's disassembly the main function to see how it looks like, to do so we're going to use **gdb** from the terminal (don't be afraid if you understand nothing about this output, just keep reading, I'm going to explain everything soon) `gdb overflow2`:

```nasm
// overflow2

(gdb) set disassembly-flavor intel
(gdb) disas main
Dump of assembler code for function main:
   0x0000000000001129 <+0>:	endbr64 
   0x000000000000112d <+4>:	push   rbp
   0x000000000000112e <+5>:	mov    rbp,rsp
   0x0000000000001131 <+8>:	mov    DWORD PTR [rbp-0x70],0x0 // myArray[0] = 0
   0x0000000000001138 <+15>:	mov    DWORD PTR [rbp-0x10],0x1 // myArray[24] = 1 
   0x000000000000113f <+22>:	mov    DWORD PTR [rbp-0xc],0x2 // myArray[25] = 2
   0x0000000000001146 <+29>:	mov    DWORD PTR [rbp+0x8],0x5 // myArray[30] = 5
   0x000000000000114d <+36>:	nop
   0x000000000000114e <+37>:	pop    rbp
   0x000000000000114f <+38>:	ret    
End of assembler dump.
```

Opening the program in gdb, we can see the **main** function code, in assembly. The gdb uses the AT&T syntax by default, but as I don't like it, I've set the "disassembly flavor" to Intel in the first line. After that I just said "hey gdb, disassembly the main function". For now, what interests us is only what is between the lines <+8> and <+29>, the following:

```nasm
mov   DWORD PTR [rbp-0x70],0x0
mov   DWORD PTR [rbp-0x10],0x1
mov   DWORD PTR [rbp-0xc],0x2
mov   DWORD PTR [rbp+0x8],0x5
```

**RBP** is the **Base Pointer Register**. It plays a crucial role in establishing and accessing the stack frame of a function (I'll talk more about this later). In this context, RBP points near the limits of our array, take a look at this table:

```
+--------------+---------------+---------------+
| Mem. Address | Content       | myArray       |
+--------------+---------------+---------------+
| ...          | ...           | ...           |
| rbp-0x70     | 0x00000000    | (myArray[0])  |
| ...          | ...           | ...           |
| rbp-0x10     | 0x00000001    | (myArray[24]) |
| rbp-0x0C     | 0x00000002    | (myArray[25]) |
| ...          | ...           | ...           |
| rbp+0x08     | 0x00000005    | (myArray[30]) |
| ...          | ...           | ...           |
+--------------+---------------+---------------+
```

We did something interesting here, we have set the program to go there in memory location **[rbp+0x08]** and write the value 5. The offset that points to the beginning of **myArray** is **[rbp-0x70]**, if we calculate the difference between **[rbp+0x08]** and **[rbp-0x70]** we get exactly 120 bytes, each **int** uses **4 bytes** in memory, so we have **[rbp+0x08]** pointing exactly to **myArray[30]**.

*Anyway, if you don't understand this Assembly bullshit, that's okay. For now, you need to understand that in C we are allowed to write and read beyond the limits of a buffer and if we don't worry about this, we may have problems with buffer overflow.*

### The Stack
The stack is a LIFO type of structure (“last in, first out”). Every program stores a stack in memory, to, among other things, control the execution flow of functions and the program itself. The stack is something commonly associated with a pile of plates, in which the last plate to be placed is always the first to be removed, in our case we don't have plates, but rather data!

*The stack is a data structure, in which, as the name suggests, data is “stacked” and “removed”. Easy, right?*

To insert and remove data from the stack, we use the **PUSH** and **POP** instructions respectively. There is also the **RSP (Stack Pointer Register)**, which usually points to the top of the stack.

To have a better representation of the stack: 

```
+------------+-----------+--------------+
| Address    |   Value   | RSP Pointer  |
+------------+-----------+--------------+
|  0x576710  |    123    |  <<--- RSP   |
|  0x576714  |    234    |              |
|  0x576718  |    2      |              |
|  0x57671C  |    54     |              |
|  0x576720  |    abcd   |              |
+------------+-----------+--------------+
```

The **ESP** register points to the last address used by the stack or the first free address in the stack; As I said before, **PUSH** and **POP** are used to insert and remove data from the stack, let's see an example.

Let's assume the stack is like the table above and I execute the `PUSH [addr var]` instruction. After the instruction, the stack will look like this:

```
+------------+-------------+--------------+
| Address    |    Value    | RSP Pointer  |
+------------+-------------+--------------+
|  0x57670C  |  [addr var] |  <<--- RSP   |
|  0x576710  |    123      |              |
|  0x576714  |    234      |              |
|  0x576718  |    2        |              |
|  0x57671C  |    54       |              |
|  0x576720  |    abcd     |              |
+------------+-------------+--------------+
```

Let's suppose two more instructions:

```
pop rax
pop rbx
```

And now, the stack will look like this:

```
+------------+-------------+--------------+
| Address    |    Value    |   Registers  |
+------------+-------------+--------------+
|  0x57670C  |  [addr var] |  --> rax     |
|  0x576710  |    123      |  --> rbx     |
|  0x576714  |    234      |  <<--- RSP   |
|  0x576718  |    2        |              |
|  0x57671C  |    54       |              |
|  0x576720  |    abcd     |              |
+------------+-------------+--------------+
```

What happens is that the data is read from the stack and stored in **RAX** and **RBX**, however, it stays in memory too, and now the top of the stack (RSP) is pointing to the address 0x576714.

Another important register related to the stack is the **RBP**, it is generally used to calculate a relative address within the stack, although it can be used as a general purpose register, its use is commonly associated with the stack. In `overflow2.c`, way back when we disassembled the source code with the help of **gdb**, we could see **RBP** being used to calculate relative addresses to access **myArray**.

In addition to the stack, programs also have a memory area called **HEAP**, I'm not going to talk about it, but it is important that you know of its existence, as there is a similar problem associated with it: `heap overflow`, but the way to exploit it is different and its out of our scope here.

### The Stack and Functions
Let's see now how the stack is used to control the program's execution flow, here things start to get interesting!

The basic purpose of the stack is to make functions more efficient. A function changes the program's execution flow, it pauses for a little bit and jumps into a group of instructions that can be executed independently (the function), it's like stopping on a trip to visit a location that was not in the original route and then return to the initial path after.

When a function ends, it returns to where it was called, reestablishing the program's execution flow. This type of implementation becomes more efficient with the use of the stack. So let's look at the example:

```c
// overflow3.c

#include <stdio.h>

void myFunc (int a, int b, int c, int d, int e, int f, int g, int h){
   int myArray [4];
   myArray[0] = 5;
   return;
}

int main (void){
   myFunc (1, 2, 3, 4, 5, 6, 7, 8);
   printf ("Hello World!\n");
   return (0);
}

// To compile: gcc -o overflow3 overflow3.c -fno-stack-protector -O0
```

The program execution begins in the `main` function, until a call to another function is found. In this case, the first function called is **myFunc**. At this point, the program's execution flow is changed and passes to **myFunc**, when the called function ends, the execution flow returns to the point at which the call was made, here, it returns to main; To make it more interesting, let's see this in assembly:

```nasm
// overflow3

(gdb) set disassembly-flavor intel
(gdb) disas main
Dump of assembler code for function main:
   0x000000000000116f <+0>:	endbr64 
   0x0000000000001173 <+4>:	push   rbp
   0x0000000000001174 <+5>:	mov    rbp,rsp
   0x0000000000001177 <+8>:	push   0x8
   0x0000000000001179 <+10>:	push   0x7
   0x000000000000117b <+12>:	mov    r9d,0x6
   0x0000000000001181 <+18>:	mov    r8d,0x5
   0x0000000000001187 <+24>:	mov    ecx,0x4
   0x000000000000118c <+29>:	mov    edx,0x3
   0x0000000000001191 <+34>:	mov    esi,0x2
   0x0000000000001196 <+39>:	mov    edi,0x1
   0x000000000000119b <+44>:	call   0x1149 <myFunc>
   0x00000000000011a0 <+49>:	add    rsp,0x10
   0x00000000000011a4 <+53>:	lea    rax,[rip+0xe59]        # 0x2004
   0x00000000000011ab <+60>:	mov    rdi,rax
   0x00000000000011ae <+63>:	call   0x1050 <puts@plt>
   0x00000000000011b3 <+68>:	mov    eax,0x0
   0x00000000000011b8 <+73>:	leave  
   0x00000000000011b9 <+74>:	ret    
End of assembler dump.
```

Let's have a look at myFunc now:

```nasm
// overflow3

(gdb) disas myFunc
Dump of assembler code for function myFunc:
   0x0000000000001149 <+0>:	endbr64 
   0x000000000000114d <+4>:	push   rbp
   0x000000000000114e <+5>:	mov    rbp,rsp
   0x0000000000001151 <+8>:	mov    DWORD PTR [rbp-0x14],edi
   0x0000000000001154 <+11>:	mov    DWORD PTR [rbp-0x18],esi
   0x0000000000001157 <+14>:	mov    DWORD PTR [rbp-0x1c],edx
   0x000000000000115a <+17>:	mov    DWORD PTR [rbp-0x20],ecx
   0x000000000000115d <+20>:	mov    DWORD PTR [rbp-0x24],r8d
   0x0000000000001161 <+24>:	mov    DWORD PTR [rbp-0x28],r9d
   0x0000000000001165 <+28>:	mov    DWORD PTR [rbp-0x10],0x5
   0x000000000000116c <+35>:	nop
   0x000000000000116d <+36>:	pop    rbp
   0x000000000000116e <+37>:	ret
End of assembler dump.
```

First of all, the function's prologue is executed, the value of the **RBP** register is pushed to the stack. This happens because **RBP** may be used to reference some values inside myFunc, so its original value must be preserved. When the function ends, the **RBP** value is then restored, as it needs to be used again to reference values used by the previous function (for which the program returns to).

Important to notice here that there are a lot of performance improvements running under the hood. To understand why I passed 8 arguments to **myFunc**, you can take a look at **main's** disassembly. While in 32-bit x86 architecture, function arguments are typically passed using the stack, in x86_64, the first few arguments to a function are typically passed in registers rather than the stack, because processor registers are way faster to access than memory, so it is just a matter of optimization. I had to pass 8 arguments to "force" the compiler "deciding" to use the stack to pass values to the function (`push 0x8` and `push 0x7`).

In summary: The stack is used to store local variables, function parameters, return addresses, and other information necessary for function calls and execution.

It is also worth paying attention to **myFunc** call. In x86_64 architecture, when a function is called (`call`), the return address (the address to which the program should return after the function execution) is typically stored on the stack. This return address is the address of the instruction following the function call. There is a register called **RIP (Instruction Pointer Register)**, it is part of the processor's state, and it is implicitly updated when instructions are executed.

Considering inside **myFunc**, the stack may look something like this:

```
+------------+
|  ...       |
+------------+
| r9d (0x6)  | <- [rbp-0x28]
+------------+
| r8d (0x5)  | <- [rbp-0x24]
+------------+
| ecx (0x4)  | <- [rbp-0x20]
+------------+
| edx (0x3)  | <- [rbp-0x1c]
+------------+
| esi (0x2)  | <- [rbp-0x18]
+------------+
| edi (0x1)  | <- [rbp-0x14]
+------------+
| 0x5        | <- [rbp-0x10]
+------------+
| ...        |
+------------+
| ret        | <- the address to return
+------------+
| ...        |
+------------+
| 0x7        |
+------------+
| 0x8        |
+------------+
```

## The Stack Overflow
The main concepts are already over, if you could understand something of this very confuse thoughts I just wrote, let's see how this stack overflow thing works in practice. This is our next code:

```c
// overflow4.c

#include <stdio.h>

void readString (void){
   char myArray[32];
   scanf ("%s", myArray);
   printf ("%s\n", myArray);
   return;
}

int main (void){
   readString ();
   return (0);
}

// To compile: gcc -o overflow4 overflow4.c -fno-stack-protector -O0
```

Running this program we can try some different inputs to understand how it behaves:

```bash
willgcr@workstation:~/Desktop/stack-overflow$ ./overflow4 
aaaaa
aaaaa
willgcr@workstation:~/Desktop/stack-overflow$ ./overflow4 
aaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaa
willgcr@workstation:~/Desktop/stack-overflow$ ./overflow4 
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
Segmentation fault (core dumped)
```

Note that there is no check regarding the length of the string the user can type. In the third case, when we typed a string with 50 characters, the program gave us a segmentation fault, because we exceeded the size intended for the string and ended up overwriting important data on the stack. Let's disassembly this:

```nasm
(gdb) set disassembly-flavor intel
(gdb) disas main
Dump of assembler code for function main:
   0x000000000000119f <+0>:	endbr64 
   0x00000000000011a3 <+4>:	push   rbp
   0x00000000000011a4 <+5>:	mov    rbp,rsp
   0x00000000000011a7 <+8>:	call   0x1169 <readString>
   0x00000000000011ac <+13>:	mov    eax,0x0
   0x00000000000011b1 <+18>:	pop    rbp
   0x00000000000011b2 <+19>:	ret    
End of assembler dump.
(gdb) disas readString
Dump of assembler code for function readString:
   0x0000000000001169 <+0>:	endbr64 
   0x000000000000116d <+4>:	push   rbp
   0x000000000000116e <+5>:	mov    rbp,rsp
   0x0000000000001171 <+8>:	sub    rsp,0x20
   0x0000000000001175 <+12>:	lea    rax,[rbp-0x20]
   0x0000000000001179 <+16>:	mov    rsi,rax
   0x000000000000117c <+19>:	lea    rax,[rip+0xe81]        # 0x2004
   0x0000000000001183 <+26>:	mov    rdi,rax
   0x0000000000001186 <+29>:	mov    eax,0x0
   0x000000000000118b <+34>:	call   0x1070 <__isoc99_scanf@plt>
   0x0000000000001190 <+39>:	lea    rax,[rbp-0x20]
   0x0000000000001194 <+43>:	mov    rdi,rax
   0x0000000000001197 <+46>:	call   0x1060 <puts@plt>
   0x000000000000119c <+51>:	nop
   0x000000000000119d <+52>:	leave  
   0x000000000000119e <+53>:	ret    
End of assembler dump.
```

At this point, we haven't executed the program inside **gdb** yet, and something interesting is going to happen with the addresses we see in the most left column when we are disassembling the functions, as soon as we set **gdb** to run the program, these addresses changes...

```nasm
(gdb) run
Starting program: /home/willgcr/Desktop/stack-overflow/overflow4 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
aaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaa
[Inferior 1 (process 57762) exited normally]
(gdb) disas main
Dump of assembler code for function main:
   0x000055555555519f <+0>:	endbr64 
   0x00005555555551a3 <+4>:	push   rbp
   0x00005555555551a4 <+5>:	mov    rbp,rsp
   0x00005555555551a7 <+8>:	call   0x555555555169 <readString>
   0x00005555555551ac <+13>:	mov    eax,0x0
   0x00005555555551b1 <+18>:	pop    rbp
   0x00005555555551b2 <+19>:	ret    
End of assembler dump.
(gdb) disas readString
Dump of assembler code for function readString:
   0x0000555555555169 <+0>:	endbr64 
   0x000055555555516d <+4>:	push   rbp
   0x000055555555516e <+5>:	mov    rbp,rsp
   0x0000555555555171 <+8>:	sub    rsp,0x20
   0x0000555555555175 <+12>:	lea    rax,[rbp-0x20]
   0x0000555555555179 <+16>:	mov    rsi,rax
   0x000055555555517c <+19>:	lea    rax,[rip+0xe81]        # 0x555555556004
   0x0000555555555183 <+26>:	mov    rdi,rax
   0x0000555555555186 <+29>:	mov    eax,0x0
   0x000055555555518b <+34>:	call   0x555555555070 <__isoc99_scanf@plt>
   0x0000555555555190 <+39>:	lea    rax,[rbp-0x20]
   0x0000555555555194 <+43>:	mov    rdi,rax
   0x0000555555555197 <+46>:	call   0x555555555060 <puts@plt>
   0x000055555555519c <+51>:	nop
   0x000055555555519d <+52>:	leave  
   0x000055555555519e <+53>:	ret    
End of assembler dump.
```

Important to notice that between different runs of the same program, we can see a thing called **ASLR** in action, it stands for *Address Space Layout Randomization*. This randomization is a security feature that helps prevent certain types of attacks by randomizing the location of code, data, and stack in memory. This makes it more difficult for an attacker to predict the memory layout and exploit vulnerabilities.

In the first set of addresses (before running), the program hasn't been loaded into memory yet, so **gdb** shows the default `0x000000000000...` base address. After running the program, **ASLR** kicks in, and the base address is randomized to `0x0000555555555...`. The actual physical addresses in RAM may be different, of course.

At least this is the most reasonable explanation I found by researching, since it is the first time I see something like this, when I first wrote this article I didn't face this behavior. 

Anyway, only after running the program and having these addresses randomized, I was able to set breakpoints, a very important approach to understand how the program is working and see how the stack is structured. I'm putting two breakpoints, one in the call for **scanf** (inside the **readString** function) and another one before **readString** returns (ret). This is what I get:

```nasm
(gdb) break *0x000055555555518b
Breakpoint 1 at 0x55555555518b
(gdb) break *0x000055555555519e
Breakpoint 2 at 0x55555555519e
(gdb) run
Starting program: /home/willgcr/Desktop/stack-overflow/overflow4 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, 0x000055555555518b in readString ()
(gdb) x/24x $rsp
0x7fffffffdc70:	0x00000002	0x00000000	0x178bfbff	0x00000000
0x7fffffffdc80:	0xffffe149	0x00007fff	0x00000064	0x00000000
0x7fffffffdc90:	0xffffdca0	0x00007fff	0x555551ac	0x00005555
0x7fffffffdca0:	0x00000001	0x00000000	0xf7da5d90	0x00007fff
0x7fffffffdcb0:	0x00000000	0x00000000	0x5555519f	0x00005555
0x7fffffffdcc0:	0xffffdda0	0x00000001	0xffffddb8	0x00007fff
(gdb) continue
Continuing.
AAAAAAAAAABBBBBBBBBBCCCCCCCCCC
AAAAAAAAAABBBBBBBBBBCCCCCCCCCC

Breakpoint 2, 0x000055555555519e in readString ()
(gdb) x/24x $rsp
0x7fffffffdc98:	0x555551ac	0x00005555	0x00000001	0x00000000
0x7fffffffdca8:	0xf7da5d90	0x00007fff	0x00000000	0x00000000
0x7fffffffdcb8:	0x5555519f	0x00005555	0xffffdda0	0x00000001
0x7fffffffdcc8:	0xffffddb8	0x00007fff	0x00000000	0x00000000
0x7fffffffdcd8:	0x194cf12b	0x6f0e5501	0xffffddb8	0x00007fff
0x7fffffffdce8:	0x5555519f	0x00005555	0x55557db8	0x00005555
(gdb) continue
Continuing.
[Inferior 1 (process 58798) exited normally]
```

It's time to be patient and verify what is going on here, step by step. Look that I've set two breakpoints, and when got to those breakpoints I just show how data is arranged inside the **stack** (`x/24x $rsp`), the most important thing to notice here is the address `0x00005555555551ac` in the stack, in the first break it is located in the third and fourth column of the third row (yep, the way it is stored is strange), in the second break it is located right on top of the stack (the two first columns).

Do you remember I explained that when functions are called, the return address is pushed onto the stack so the program knows where to return after the function execution? This is exactly what we're seeing now, If you verify the disassembly of the **main** function you'll notice that the address `0x00005555555551ac` is the instruction right after the **readString** call.

Ok, ok, but why this is important? I'll make things more interesting now, I promise. I'm just using one breakpoint now:

```nasm
(gdb) delete 1 # To delete the breakpoint #1
(gdb) run
Starting program: /home/willgcr/Desktop/stack-overflow/overflow4 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
AAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBB
AAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBB

Breakpoint 2, 0x000055555555519e in readString ()
(gdb) x/24x $rsp
0x7fffffffdc98:	0x42424242	0x00005500	0x00000001	0x00000000
0x7fffffffdca8:	0xf7da5d90	0x00007fff	0x00000000	0x00000000
0x7fffffffdcb8:	0x5555519f	0x00005555	0xffffdda0	0x00000001
0x7fffffffdcc8:	0xffffddb8	0x00007fff	0x00000000	0x00000000
0x7fffffffdcd8:	0x4fea32d3	0x1a86bd3b	0xffffddb8	0x00007fff
0x7fffffffdce8:	0x5555519f	0x00005555	0x55557db8	0x00005555
(gdb) continue
Continuing.

Program received signal SIGSEGV, Segmentation fault.
0x0000550042424242 in ?? ()
```

The breakpoint left is the one at the call to `ret`, inside `readString`, and as you can see now, the stack at this point have the return address replaced by `0x42424242`. It happens that in the ASCII character set, 0x42 corresponds to the uppercase letter **'B'**. Congratulations to me, I just replaced the returning address by overwriting it on the stack using a buffer overflow!

But how the hell is something like this useful? Well, the most boring way of exploiting a stack overflow has already been shown: *a denial of service*, this is exactly what just happened. 

But there is a more interesting approach to this exploitation: controlling how the program works and behaves, and executing arbitrary code. Now let's see if I can still do something better. The next thing I'm going to try is replacing the address of return inside the stack by a known address, not random "ABC" bytes anymore, let's see what happens:

```bash
(gdb) set disassembly-flavor intel
(gdb) run <<< $(printf "TEST_INPUT")
Starting program: /home/willgcr/Desktop/stack-overflow/overflow4 <<< $(printf "TEST_INPUT")
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
TEST_INPUT
[Inferior 1 (process 34678) exited normally]
(gdb) disas main
Dump of assembler code for function main:
   0x000055555555519f <+0>:	endbr64 
   0x00005555555551a3 <+4>:	push   rbp
   0x00005555555551a4 <+5>:	mov    rbp,rsp
   0x00005555555551a7 <+8>:	call   0x555555555169 <readString>
   0x00005555555551ac <+13>:	mov    eax,0x0
   0x00005555555551b1 <+18>:	pop    rbp
   0x00005555555551b2 <+19>:	ret    
End of assembler dump.
(gdb) run <<< $(printf "%040x\xa7\x51\x55\x55\x55\x55")
Starting program: /home/willgcr/Desktop/stack-overflow/overflow4 <<< $(printf "%040x\xa7\x51\x55\x55\x55\x55")
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
0000000000000000000000000000000000000000�QUUUU
0000000000000000000000000000000000000000�QUUUU
[Inferior 1 (process 34699) exited normally]
```

Nice! I could make `readString` run twice, explained below:

1. I've set the `gdb` syntax to Intel: `set disassembly-flavor intel`;
2. Ran one first time, so ASLR comes to action and randomizes memory addresses. It also allows us to see the `readString` being executed once and printing back the "TEST_INPUT" string, as it would do on a normal execution;
3. Disassembled main again to see the randomized memory addresses I got (the most important here is to get the address of the `readString` function);
4. I knew previously that by passing 40 bytes as input I would align the return address right on the next stack position to be written (I did it before passing random ABs), so I basically need to pass 40 bytes plus my custom return address;
5. In this case I'm using the address of `readString` to replace the right return address on the stack, this is my input buffer (40 bytes + payload): `0000000000000000000000000000000000000000\xa7\x51\x55\x55\x55\x55`, this strange thing in the end is just the address of the `readString` function: `0x00005555555551a7`;
6. I run the program again, now passing my custom input. To make it easier to read I'm using `printf` so I don't need to count 40 zeros, and *voilà*, the `readString` gets executed twice, so we can see the buffer data printed not one, but two times!
7. Ok, but the question stills the same: "How the hell is this useful?". Let's take a look at a different scenario now!


## Cracking a license

Ok, now let's get into a more interesting scenario, which simulates how a buffer overflow can be risky, let's suppose we got a program like this one:

```c
// overflow5.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int checkLicense (void);
void licensingSucceeds (void);
void licensingFails (void);

int main (void) {
	if (!checkLicense ()) {
		licensingFails ();
		return (1);
	} else {
		licensingSucceeds ();
		return (0);
	}
}

void licensingSucceeds (void) {
	printf ("Software Licensed!\n");
	return;
}

void licensingFails (void) {
	printf ("Invalid Key!\n");
	return;
}

int checkLicense (void) {
	char serial[32];
	int total = 0;	
	printf ("License Key: ");
	scanf ("%s", serial);
	for (int i = 0; i < strlen(serial); i++) {
		total += serial[i];
	}
	printf ("%d\n", total);
	if (total % 77 == 11)
		return (1);
	else
		return (0);
}

// To compile: gcc -o overflow5 overflow5.c -fno-stack-protector -O0
```

To successfully license this program, the user should pass as key a string for which the sum of the ASCII characters divided by 77 gives 11 as the remainder. You could try, a very simple solution would be the character X which has the ASCII code of 88.

Let's now disassembly this program:

```bash
(gdb) set disassembly-flavor intel
(gdb) run
Starting program: /home/willgcr/Desktop/stack-overflow/overflow5 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
License Key: X
Software Licensed!
[Inferior 1 (process 39773) exited normally]
(gdb) disas main
Dump of assembler code for function main:
   0x00005555555551a9 <+0>:	endbr64 
   0x00005555555551ad <+4>:	push   rbp
   0x00005555555551ae <+5>:	mov    rbp,rsp
   0x00005555555551b1 <+8>:	call   0x555555555206 <checkLicense>
   0x00005555555551b6 <+13>:	test   eax,eax
   0x00005555555551b8 <+15>:	jne    0x5555555551c6 <main+29>
   0x00005555555551ba <+17>:	call   0x5555555551ec <licensingFails>
   0x00005555555551bf <+22>:	mov    eax,0x1
   0x00005555555551c4 <+27>:	jmp    0x5555555551d0 <main+39>
   0x00005555555551c6 <+29>:	call   0x5555555551d2 <licensingSucceeds>
   0x00005555555551cb <+34>:	mov    eax,0x0
   0x00005555555551d0 <+39>:	pop    rbp
   0x00005555555551d1 <+40>:	ret    
End of assembler dump.
```

Actually, there is no secret here, we already have a very interesting address, which is the addres of `licensingSucceeds`: `0x00005555555551c6`. Now we just need to get a little deeper into `checkLicense`, I'm setting two breakpoints, one in the `scanf` and other right before the function return:

```bash
(gdb) disas checkLicense
Dump of assembler code for function checkLicense:
   0x0000555555555206 <+0>:	endbr64 
   0x000055555555520a <+4>:	push   rbp
   0x000055555555520b <+5>:	mov    rbp,rsp
   0x000055555555520e <+8>:	push   rbx
   0x000055555555520f <+9>:	sub    rsp,0x38
   0x0000555555555213 <+13>:	mov    DWORD PTR [rbp-0x14],0x0
   0x000055555555521a <+20>:	lea    rax,[rip+0xe03]        # 0x555555556024
   0x0000555555555221 <+27>:	mov    rdi,rax
   0x0000555555555224 <+30>:	mov    eax,0x0
   0x0000555555555229 <+35>:	call   0x5555555550a0 <printf@plt>
   0x000055555555522e <+40>:	lea    rax,[rbp-0x40]
   0x0000555555555232 <+44>:	mov    rsi,rax
   0x0000555555555235 <+47>:	lea    rax,[rip+0xdf6]        # 0x555555556032
   0x000055555555523c <+54>:	mov    rdi,rax
   0x000055555555523f <+57>:	mov    eax,0x0
   0x0000555555555244 <+62>:	call   0x5555555550b0 <__isoc99_scanf@plt>
   0x0000555555555249 <+67>:	mov    DWORD PTR [rbp-0x18],0x0
   0x0000555555555250 <+74>:	jmp    0x555555555266 <checkLicense+96>
   0x0000555555555252 <+76>:	mov    eax,DWORD PTR [rbp-0x18]
   0x0000555555555255 <+79>:	cdqe   
   0x0000555555555257 <+81>:	movzx  eax,BYTE PTR [rbp+rax*1-0x40]
   0x000055555555525c <+86>:	movsx  eax,al
   0x000055555555525f <+89>:	add    DWORD PTR [rbp-0x14],eax
   0x0000555555555262 <+92>:	add    DWORD PTR [rbp-0x18],0x1
   0x0000555555555266 <+96>:	mov    eax,DWORD PTR [rbp-0x18]
   0x0000555555555269 <+99>:	movsxd rbx,eax
   0x000055555555526c <+102>:	lea    rax,[rbp-0x40]
   0x0000555555555270 <+106>:	mov    rdi,rax
   0x0000555555555273 <+109>:	call   0x555555555090 <strlen@plt>
   0x0000555555555278 <+114>:	cmp    rbx,rax
   0x000055555555527b <+117>:	jb     0x555555555252 <checkLicense+76>
   0x000055555555527d <+119>:	mov    eax,DWORD PTR [rbp-0x14]
   0x0000555555555280 <+122>:	movsxd rdx,eax
   0x0000555555555283 <+125>:	imul   rdx,rdx,0x3531dec1
   0x000055555555528a <+132>:	shr    rdx,0x20
   0x000055555555528e <+136>:	sar    edx,0x4
   0x0000555555555291 <+139>:	mov    ecx,eax
   0x0000555555555293 <+141>:	sar    ecx,0x1f
   0x0000555555555296 <+144>:	sub    edx,ecx
   0x0000555555555298 <+146>:	imul   ecx,edx,0x4d
   0x000055555555529b <+149>:	sub    eax,ecx
   0x000055555555529d <+151>:	mov    edx,eax
   0x000055555555529f <+153>:	cmp    edx,0xb
   0x00005555555552a2 <+156>:	jne    0x5555555552ab <checkLicense+165>
   0x00005555555552a4 <+158>:	mov    eax,0x1
   0x00005555555552a9 <+163>:	jmp    0x5555555552b0 <checkLicense+170>
   0x00005555555552ab <+165>:	mov    eax,0x0
   0x00005555555552b0 <+170>:	mov    rbx,QWORD PTR [rbp-0x8]
   0x00005555555552b4 <+174>:	leave  
=> 0x00005555555552b5 <+175>:	ret    
End of assembler dump.
(gdb) break *0x0000555555555244
Breakpoint 1 at 0x555555555244
(gdb) break *0x00005555555552b5
Breakpoint 2 at 0x5555555552b5
(gdb) run <<< $(printf "%035x")
Starting program: /home/willgcr/Desktop/stack-overflow/overflow5 <<< $(printf "%035x")
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, 0x0000555555555244 in checkLicense ()
(gdb) x/24x $rsp
0x7fffffffdcc0:	0x00000d30	0x00000000	0xffffe199	0x00007fff
0x7fffffffdcd0:	0xf7fc1000	0x00007fff	0x01000000	0x00000101
0x7fffffffdce0:	0x00000002	0x00000000	0x178bfbff	0x00000000
0x7fffffffdcf0:	0xffffe1a9	0x00007fff	0x00000000	0x00000000
0x7fffffffdd00:	0xffffdd10	0x00007fff	0x555551b6	0x00005555
0x7fffffffdd10:	0x00000001	0x00000000	0xf7da3d90	0x00007fff
(gdb) continue
Continuing.

Breakpoint 2, 0x00005555555552b5 in checkLicense ()
(gdb) x/24x $rsp
0x7fffffffdd08:	0x555551b6	0x00005555	0x00000001	0x00000000
0x7fffffffdd18:	0xf7da3d90	0x00007fff	0x00000000	0x00000000
0x7fffffffdd28:	0x555551a9	0x00005555	0xffffde10	0x00000001
0x7fffffffdd38:	0xffffde28	0x00007fff	0x00000000	0x00000000
0x7fffffffdd48:	0x6380f9b0	0x165bd285	0xffffde28	0x00007fff
0x7fffffffdd58:	0x555551a9	0x00005555	0x55557da8	0x00005555
```

And here we go, we can see the return address (`0x00005555555551b6`) in the stack, I just need to try aligning it to the point where I can insert my own return address now. To do so I'm going to check for different input sizes:

```bash
(gdb) run <<< $(printf "%035xAAAAAAAAAABBBBBBBBBB")
Starting program: /home/willgcr/Desktop/stack-overflow/overflow5 <<< $(printf "%035xAAAAAAAAAABBBBBBBBBB")
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, 0x0000555555555244 in checkLicense ()
(gdb) x/24x $rsp
0x7fffffffdcc0:	0x00000d30	0x00000000	0xffffe199	0x00007fff
0x7fffffffdcd0:	0xf7fc1000	0x00007fff	0x01000000	0x00000101
0x7fffffffdce0:	0x00000002	0x00000000	0x178bfbff	0x00000000
0x7fffffffdcf0:	0xffffe1a9	0x00007fff	0x00000000	0x00000000
0x7fffffffdd00:	0xffffdd10	0x00007fff	0x555551b6	0x00005555
0x7fffffffdd10:	0x00000001	0x00000000	0xf7da3d90	0x00007fff
(gdb) continue
Continuing.

Breakpoint 2, 0x00005555555552b5 in checkLicense ()
(gdb) x/24x $rsp
0x7fffffffdd08:	0x555551b6	0x00005555	0x00000001	0x00000000
0x7fffffffdd18:	0xf7da3d90	0x00007fff	0x00000000	0x00000000
0x7fffffffdd28:	0x555551a9	0x00005555	0xffffde10	0x00000001
0x7fffffffdd38:	0xffffde28	0x00007fff	0x00000000	0x00000000
0x7fffffffdd48:	0xe1f6bf8d	0xb8494858	0xffffde28	0x00007fff
0x7fffffffdd58:	0x555551a9	0x00005555	0x55557da8	0x00005555
(gdb) continue
Continuing.
License Key: Invalid Key!
[Inferior 1 (process 40174) exited with code 01]
(gdb) run <<< $(printf "%055xAAAAAAAAAABBBBBBBBBB")
Starting program: /home/willgcr/Desktop/stack-overflow/overflow5 <<< $(printf "%055xAAAAAAAAAABBBBBBBBBB")
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, 0x0000555555555244 in checkLicense ()
(gdb) x/24x $rsp
0x7fffffffdcc0:	0x00000d30	0x00000000	0xffffe199	0x00007fff
0x7fffffffdcd0:	0xf7fc1000	0x00007fff	0x01000000	0x00000101
0x7fffffffdce0:	0x00000002	0x00000000	0x178bfbff	0x00000000
0x7fffffffdcf0:	0xffffe1a9	0x00007fff	0x00000000	0x00000000
0x7fffffffdd00:	0xffffdd10	0x00007fff	0x555551b6	0x00005555
0x7fffffffdd10:	0x00000001	0x00000000	0xf7da3d90	0x00007fff
(gdb) continue
Continuing.

Breakpoint 2, 0x00005555555552b5 in checkLicense ()
(gdb) x/24x $rsp
0x7fffffffdd08:	0x00424242	0x00005555	0x00000001	0x00000000
0x7fffffffdd18:	0xf7da3d90	0x00007fff	0x00000000	0x00000000
0x7fffffffdd28:	0x555551a9	0x00005555	0xffffde10	0x00000001
0x7fffffffdd38:	0xffffde28	0x00007fff	0x00000000	0x00000000
0x7fffffffdd48:	0x7b671ce9	0x33f9e352	0xffffde28	0x00007fff
0x7fffffffdd58:	0x555551a9	0x00005555	0x55557da8	0x00005555
```

And here we go! This is the stack arrangement in the second breakpoint, right before the `ret`, I can see these beautiful 0x42 (the ASCII for B) there! Now I just need to adjust the size of my buffer a little bit so I can replace the return address using something useful (that `licensingSucceeds` address).

```bash
(gdb) run <<< $(printf "%076xAAA")
Starting program: /home/willgcr/Desktop/stack-overflow/overflow5 <<< $(printf "%076xAAA")
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, 0x0000555555555244 in checkLicense ()
(gdb) continue
Continuing.

Breakpoint 2, 0x00005555555552b5 in checkLicense ()
(gdb) x/24x $rsp
0x7fffffffdd08:	0x30303030	0x00414141	0x00000001	0x00000000
0x7fffffffdd18:	0xf7da3d90	0x00007fff	0x00000000	0x00000000
0x7fffffffdd28:	0x555551a9	0x00005555	0xffffde10	0x00000001
0x7fffffffdd38:	0xffffde28	0x00007fff	0x00000000	0x00000000
0x7fffffffdd48:	0xaf18769e	0x808cab70	0xffffde28	0x00007fff
0x7fffffffdd58:	0x555551a9	0x00005555	0x55557da8	0x00005555
```


I found that the perfect size to my buffer is 72 bytes, so it aligns the return address, anything greater that this replaces the return address stored into the stack! So let's hack it:

```bash
(gdb) run <<< $(printf "%072x\xc6\x51\x55\x55\x55\x55")
Starting program: /home/willgcr/Desktop/stack-overflow/overflow5 <<< $(printf "%072x\xc6\x51\x55\x55\x55\x55")
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
License Key: Software Licensed!
[Inferior 1 (process 40970) exited normally]
```

Finally, we managed to validate our little (non-secure) test program!

Of course, using `gdb` and having the source code of the target program the process becomes relatively simple. But when it comes to commercial software and closed-source code, there are other tools that can be used to reverse engineer and discover everything that `gdb` helped us with, and as I said, there are also means to bypass the ASLR protection.

In real world scenarios, sometimes is possible to insert a payload using the input buffer itself, and redirect the program to execute the payload, this makes feasible to run arbitrary code in the target machine, and if you have the right privileges the sky is the limit.

That's all! If you have any thoughts on this, please fell free to reach me at [hello@willgcr.me](mailto:hello@willgcr.me)!

See ya,

Willian Rocha