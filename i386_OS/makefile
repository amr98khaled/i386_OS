						#We have to tell the compiler to not to assume that there is a glibc nor exception handling for handling interrupts about memory management 
						#There is no glibc nor exceptions because we are creating our own OS
						#So we have to pass g++ parameters about the above situation
						#-m32 , -m64 [Generate code for a 32-bit or 64-bit environment. The 32-bit environment sets int, long and pointer to 32 bits and generates code that runs on any i386 system. The 64-bit environment sets int to 32 bits and long and pointer to 64 bits and generates code for AMD 's x86-64 architecture.]
						#-melf [Generate an executable in the ELF format, rather than the default mmo format used by the mmix simulator.]
						#-Idir : Add the directory dir to the head of the list of directories to be searched for header files.
						#-Iinclude : tell the compiler to look for the header files in the include directory
						
GCCPARAMS = -m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = obj/loader.o \
          obj/gdt.o \
          obj/memorymanagement.o \
          obj/drivers/driver.o \
          obj/hardwarecommunication/port.o \
          obj/hardwarecommunication/interruptstubs.o \
          obj/hardwarecommunication/interrupts.o \
           obj/syscalls.o \
          obj/multitasking.o \
          obj/drivers/amd_am79c973.o \
          obj/hardwarecommunication/pci.o \
          obj/drivers/keyboard.o \
          obj/drivers/mouse.o \
          obj/drivers/vga.o \
          obj/drivers/ata.o \
          obj/gui/widget.o \
          obj/gui/window.o \
          obj/gui/desktop.o \
          obj/net/etherframe.o \
          obj/net/arp.o \
          obj/net/ipv4.o \
          obj/kernel.o

						#target.o depends on source.cpp
						#g++[compiler] -o[output or target file] $@[the name of target file from %.o]  $<[source file from %.cpp or %.s]
						#-c [Compile or assemble the source files, but do not link. The linking stage simply is not done. The ultimate output is in the form of an object file for each source file.]
						#-o file [Place output File in file. This applies regardless to whatever sort of output is being produced, whether it be an executable file, an object file, an assembler file or preprocessed C code.]
						#-fno-builtin [Don't recognize built-in functions that do not begin with __builtin_ as prefix.]
						#-fno-rtti [Disable generation of information about every class with virtual functions for use by the C ++ runtime type identification features (dynamic_cast and typeid). If you don't use those parts of the language, you can save some space by using this flag. Note that exception handling uses the same information, but it will generate it as needed. The dynamic_cast operator can still be used for casts that do not require runtime type information, i.e. casts to "void *" or to unambiguous base classes.]
						#-nostdlib (as we don't have glibc ) [Do not use the standard system startup files or libraries when linking. No startup files and only the libraries you specify will be passed to the linker. The compiler may generate calls to "memcmp", "memset", "memcpy" and "memmove". These entries are usually resolved by entries in libc. These entry points should be supplied through some other mechanism when this option is specified.]
						#-fno-leading-underscore (if we use leading underscore we have to call _kernelMain not kernelMain)[forcibly change the way C symbols are represented in the object file. One use is to help link with legacy assembly code.]
						 
						# ‘$(@D)’ : [target directory] The directory part of the file name of the target, with the trailing slash removed. If the value of ‘$@’ is dir/foo.o then ‘$(@D)’ is dir. This value is . if ‘$@’ does not contain a slash.
						# -p: A flag which enables the command to create parent directories as necessary. If the directories exist, no error is specified.
						
obj/%.o: src/%.cpp
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<
	
						#%.s[assembley source file] as[assembler] 	
obj/%.o: src/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

						#depends on linker.ld and objects , ld[link]	
						#-T script [Use script as the linker script. This option is supported by most systems using the GNU linker. On some targets, such as bare-board targets without an operating system, the -T option may be required when linking to avoid references to undefined symbols.]
						
mykernel.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)

						#cp[copy] the source file to directory	
install: mykernel.bin
	sudo cp $< /boot/mykernel.bin
	
mykernel.iso: mykernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp mykernel.bin iso/boot/mykernel.bin
	echo 'set timeout=0'                     > iso/boot/grub/grub.cfg		#timeout for kernels menu appearance during bootloading 
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg		#index of default kernel to be loaded if user doesn't choose a kernel from menu 
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "Amr OS" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/mykernel.bin'    >> iso/boot/grub/grub.cfg		#multiboot [Load a multiboot kernel image from file.]
	echo '  boot'                            >> iso/boot/grub/grub.cfg		#boot [Boot the OS or chain-loader which has been loaded.]
	echo '}'                                 >> iso/boot/grub/grub.cfg		
	grub-mkrescue --output=mykernel.iso iso						#grub-mkrescue -o x.iso file [Save output x.iso in file. ] #We create the cd image here 
	rm -rf iso									#Force deletion of everything in iso folder including iso folder.we don't need the file after we created mykernel.iso

run: mykernel.iso
	(killall VirtualBox && sleep 1) || true						#The killall command lets you kill processes by name. # ||true is written so that no error occur if we try to run again the virtual box while it is being currently runningstartvm
	VirtualBox --startvm 'Amr OS' &	
	
.PHONY: clean
clean: 
	rm -rf obj mykernel.bin mykernel.iso 
#-rf obj : Force deletion of everything in obj directory.
