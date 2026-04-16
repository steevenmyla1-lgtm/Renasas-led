set pagination off
target remote :2331
monitor reset halt
load

break main
continue
next
next

printf "PRCRN @ 0x80281A10: 0x%08x\n", *(unsigned int*)0x80281A10
printf "PRCRS @ 0x81281A00: 0x%08x\n", *(unsigned int*)0x81281A00
printf "PMC[17] @ 0x800A0411: 0x%02x\n", *(unsigned char*)0x800A0411
printf "PM[17]  @ 0x800A0222: 0x%04x\n", *(unsigned short*)0x800A0222
printf "P[17]   @ 0x800A0011: 0x%02x\n", *(unsigned char*)0x800A0011

printf "Force writing PM=output, P=1...\n"
set {unsigned short}0x800A0222 = 0x2000
set {unsigned char}0x800A0011 = 0x40

printf "PM[17] after write: 0x%04x\n", *(unsigned short*)0x800A0222
printf "P[17]  after write: 0x%02x\n", *(unsigned char*)0x800A0011

printf "Resuming - observe LED\n"
continue
