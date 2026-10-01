extern int ext(int);
int first(int a) { return ext(a) + 1; }
__asm__(".globl handwritten\nhandwritten:\n\tjr $31\n\tnop\n");
int second(int a) { return ext(a) + 2; }
