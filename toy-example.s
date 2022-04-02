  .set noat
	.text
	.align	2
	.globl	__start
	.ent	__start
	.type	__start, @function
__start:
   addi $2, $1, 5
   addi $5, $1, 10
   add $1, $2, $5
   slt $10, $1, $5
   sltu $9, $5, $1
   addu $3, $1, $5
   and $4, $2, $5
   nor $6, $2, $5
   or $7, $2, $5
   sub $2, $2, $1
   subu $8, $7, $1
   sll $12, $1, 2
   srl $13, $12, 2

	.end	__start
	.size	__start, .-__start
