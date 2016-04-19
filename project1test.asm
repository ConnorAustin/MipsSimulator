START: add $s0, $s1, $s2
sub $s3, $s4, $s5
and $s6, $s7, $t0
or $t1, $t2, $t3
nor $t4, $t5, $t6 # START:
slt $t0, $t8, $t9
sll $s0, $s6, 31

LABEL:


srl $s0, $s6, 5
jr $s3

LOOP:   addi $s6, $s3, -1
andi $s6, $s3, 25535
ori $s6, $s3, 32
beq $s3, $s6, LOOP
bne $s3, $s6, EXIT 
lw $s6, 4($s3)
sw $s6, -4($s3) 

EXIT: j START
jal EXIT
