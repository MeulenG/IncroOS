	incbin	"External/Boot-Tools/osboot/rawfs.bin"
	incbin	"External/Boot-Tools/osboot/kernel.sys"
	times	630*2*512-($-$$)	db	0