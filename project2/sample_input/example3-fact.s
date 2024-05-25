	.data
	.text
main:
	sw	$4, 0($29)
	slti	$8, $4, 1
	beq	$8, $0, L1
	addi	$2, $0, 1
L1:
	addi	$4, $4, -1
	jal	main
	lw	$4, 0($29)
	lw	$31, 4($29)
	add	$2, $4, $2
