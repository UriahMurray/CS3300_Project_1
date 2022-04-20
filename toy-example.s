  .set noat
	.text
	.align	2
	.globl	__start
	.ent	__start
	.type	__start, @function
__start:
addi $2, $2, 8
addiu $1, $2, 3
and $3, $1, $2
andi $4, $1, 8
addi $6, $6, 1
andi $4, $1, 8
sw $2, 1($2)
beq $2, $1, .silly
addi $7, $7, 9
addi $7, $7, 1
addi $7, $7, 5
addi $8, $8, 11
.silly:
add $9, $2, $3
add $9, $9, $2
add $9, $9, $2
add $9, $9, $2
add $9, $9, $2
	.end	__start
	.size	__start, .-__start
