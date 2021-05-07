/*We tell the linker that that loader in loader.s is the entry point of the resulting bin file*/
ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
/*Output architecture is going to be i386*/
OUTPUT_ARCH(i386:i386)

/*We now collect sections from different .o files and combine them in new sections*/

SECTIONS
{
    . =0x0100000; /*offset added to the address of SECTIONS(which often is 0x00) and this is the start of .text section*/
    
    .text :
  {
    *(.multiboot)
    *(.text*)
    *(.rodata)
  }

  .data  :
  {
    start_ctors = .;                                    /*Start location of array of pointers to fucntions that initialize global constructors*/
    KEEP(*( .init_array ));                             /*Keep init array even if it is not refernced*/
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;                                      /*end location of array of pointers to fucntions that initialize global constructors*/

    *(.data)
  }

  .bss  :
  {
    *(.bss)
  }

  /DISCARD/ :                                               /*Don't include these sections*/
  {
    *(.fini_array*) 
    *(.comment) 
  }
}
