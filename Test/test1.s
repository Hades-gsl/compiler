.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text

read:
	li $v0, 4
	la $a0, _prompt
	syscall
	li $v0, 5
	syscall
	jr $ra

write:
	li $v0, 1
	syscall
	li $v0, 4
	la $a0, _ret
	syscall
	move $v0, $0
	jr $ra

f:
	move $fp, $sp
	addi $sp, $sp, -104
	li $t0, 0
	move $v0, $t0
	move $sp, $fp
	jr $ra

bf:
	move $fp, $sp
	addi $sp, $sp, -108
	li $t0, 3
	sw $t0, -4($fp) # bb
	lw $t0, 8($fp) # ab
	sw $t0, -8($fp) # t5
	lw $t0, 12($fp) # cb
	sw $t0, -12($fp) # t6
	lw $t0, -8($fp) # t5
	lw $t1, -12($fp) # t6
	add $t0, $t0, $t1
	sw $t0, -16($fp) # t4
	lw $t0, -16($fp) # t4
	move $v0, $t0
	move $sp, $fp
	jr $ra

cf:
	move $fp, $sp
	addi $sp, $sp, -92
	lw $t0, 8($fp) # ca
	move $v0, $t0
	move $sp, $fp
	jr $ra

main:
	move $fp, $sp
	addi $sp, $sp, -92
	li $t0, 6
	sw $t0, -4($fp) # a
	li $t0, 7
	sw $t0, -8($fp) # b
	li $t0, 8
	sw $t0, -12($fp) # c
	li $t0, 0
	sw $t0, -16($fp) # i
	li $t0, 0
	sw $t0, -20($fp) # j
	li $t0, 0
	sw $t0, -24($fp) # k
l1:
	lw $t0, -16($fp) # i
	sw $t0, -28($fp) # t20
	lw $t0, -4($fp) # a
	sw $t0, -32($fp) # t21
	lw $t0, -28($fp) # t20
	lw $t1, -32($fp) # t21
	blt $t0, $t1, l2
	j l3
l2:
l4:
	lw $t0, -20($fp) # j
	sw $t0, -36($fp) # t22
	lw $t0, -8($fp) # b
	sw $t0, -40($fp) # t23
	lw $t0, -36($fp) # t22
	lw $t1, -40($fp) # t23
	blt $t0, $t1, l5
	j l6
l5:
l7:
	lw $t0, -24($fp) # k
	sw $t0, -44($fp) # t24
	lw $t0, -12($fp) # c
	sw $t0, -48($fp) # t25
	lw $t0, -44($fp) # t24
	lw $t1, -48($fp) # t25
	blt $t0, $t1, l8
	j l9
l8:
	lw $t0, -24($fp) # k
	addi $sp, $sp, -4
	sw $t0, 0($sp)
	lw $t0, -20($fp) # j
	addi $sp, $sp, -4
	sw $t0, 0($sp)
	lw $t0, -16($fp) # i
	addi $sp, $sp, -4
	sw $t0, 0($sp)
	addi $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	jal f
	lw $ra, 0($sp)
	lw $fp, 4($sp)
	addi $sp, $sp, 8
	sw $v0, -52($fp) # t26
	addi $sp, $sp, 12
	lw $t0, -16($fp) # i
	sw $t0, -56($fp) # t30
	lw $t0, -20($fp) # j
	sw $t0, -60($fp) # t31
	lw $t0, -56($fp) # t30
	lw $t1, -60($fp) # t31
	blt $t0, $t1, l10
	j l11
l10:
	lw $t0, -20($fp) # j
	addi $sp, $sp, -4
	sw $t0, 0($sp)
	lw $t0, -16($fp) # i
	addi $sp, $sp, -4
	sw $t0, 0($sp)
	addi $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	jal bf
	lw $ra, 0($sp)
	lw $fp, 4($sp)
	addi $sp, $sp, 8
	sw $v0, -64($fp) # t32
	addi $sp, $sp, 8
	j l12
l11:
	lw $t0, -16($fp) # i
	addi $sp, $sp, -4
	sw $t0, 0($sp)
	addi $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	jal cf
	lw $ra, 0($sp)
	lw $fp, 4($sp)
	addi $sp, $sp, 8
	sw $v0, -68($fp) # t35
	addi $sp, $sp, 4
l12:
	lw $t0, -24($fp) # k
	move $a0, $t0
	addi $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	jal write
	lw $ra, 0($sp)
	lw $fp, 4($sp)
	addi $sp, $sp, 8
	lw $t0, -24($fp) # k
	sw $t0, -72($fp) # t41
	lw $t0, -72($fp) # t41
	li $t1, 1
	add $t0, $t0, $t1
	sw $t0, -76($fp) # t40
	lw $t0, -76($fp) # t40
	sw $t0, -24($fp) # k
	j l7
l9:
	lw $t0, -20($fp) # j
	sw $t0, -80($fp) # t45
	lw $t0, -80($fp) # t45
	li $t1, 1
	add $t0, $t0, $t1
	sw $t0, -84($fp) # t44
	lw $t0, -84($fp) # t44
	sw $t0, -20($fp) # j
	j l4
l6:
	lw $t0, -16($fp) # i
	sw $t0, -88($fp) # t49
	lw $t0, -88($fp) # t49
	li $t1, 1
	add $t0, $t0, $t1
	sw $t0, -92($fp) # t48
	lw $t0, -92($fp) # t48
	sw $t0, -16($fp) # i
	j l1
l3:
	li $t0, 0
	move $v0, $t0
	move $sp, $fp
	jr $ra
