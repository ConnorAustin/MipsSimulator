addi $gp, $zero, 0x400

addi $t0, $zero, 5
sw $t0, 0($gp)

addi $t0, $zero, 3
sw $t0, 4($gp)

addi $t0, $zero, 2
sw $t0, 8($gp)

addi $t0, $zero, 1
sw $t0, 12($gp)

addi $t0, $zero, 6
sw $t0, 16($gp)

sort:
addi $t9, $zero, 5     # constant
addi $t8, $zero, 4     # constant

addi $t0, $zero, 0
outer:
addi $t1, $zero, 0
inner:
sll $s1, $t1, 2
add $s1, $s1, $gp

lw $s2, 0($s1)
lw $s3, 4($s1)

slt $s4, $s3, $s2
beq $s4, $zero, skip

sw $s2, 4($s1)
sw $s3, 0($s1)

skip:
addi $t1, $t1, 1
bne $t1, $t8, inner
addi $t0, $t0, 1
bne $t0, $t9, outer

hang:
j hang