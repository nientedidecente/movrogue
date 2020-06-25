# movrogue
Rogue, but using x86 mov only.

Be sure to have yacc (usually given via bison) and a 32 bit libc library installed. E.g.:

```
apt-get install bison
apt-get install libc6-dev-i386
```

If you are on a 64bit OS, you need to run any executable in 32bit mode. QEMU is an option:

```
apt install qemu-user-static
qemu-i386-static ./a.out
```