	.data
	.text
main:
        addi   $8, $8, 0x10
        jal     target
        add    $0, $0, $0
        addi   $8, $8, 0x20
        j       end

target:
        addi   $9, $9, 0x30
        addi   $10, $10, 0x40
        jr      $31
end:
        addi   $15, $15, 0x11
