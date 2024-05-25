	.data
	.text
main:
        addi   $8, $8, 0
        addi   $9, $9, 0

loop:
        add    $9, $9, $8
        addi   $8, $8, 1
        slti   $7, $8, 15
        bne     $7, $0, loop

        addi   $10, $10, 0
        addi   $11, $11, 0
        addi   $12, $12, 0
        addi   $13, $13, 0
