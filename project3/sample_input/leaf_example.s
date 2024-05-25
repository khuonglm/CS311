	.data
data1:	.word	1
data2:	.word	2
data3:	.word	3
data4:	.word	4
	.text
main:
	la	$4, data1
	lw	$5, 4($4)
	lw	$6, 8($4)
	lw	$7, 12($4)
	lw	$4, 0($4)

	add	$8, $4, $5
	add	$9, $6, $7
	sub	$16, $9, $8
	add	$2, $16, $0
